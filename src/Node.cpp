//
// Created by guibertf on 9/15/21.
//

#include "Node.h"
#include <memory>
#include <random>
#include <iostream>

Node::Node(unsigned int setInitialCapacity):
leftChild(nullptr), rightChild(nullptr), parent(nullptr), representative(nullptr), size(0), cost(-1.0),
shouldDeleteChildren(false), shouldDeleteRng(true){
    rng = new RandomGenerator(0, 1);
    pointSet.reserve(setInitialCapacity);
}

unsigned int Node::getSize() const{
    return size;
}


double Node::getCost() const{
    return cost;
}


void Node::setCost(double newCost) {
    if(newCost < 0.0){
        throw std::invalid_argument("Cost cannot be negative");
    }
    cost = newCost;
}

void Node::setRepresentative(Point *newPoint, Distance distance) {
    if(!isLeaf()){
        throw std::logic_error("Setting a representative on a non leaf node does not make sense.");
    }
    if(newPoint == nullptr){
        throw std::invalid_argument("Cannot set null pointer as representative");
    }
    if(representative == newPoint){
        throw std::logic_error("This point is already representative of this node");
    }
    representative = newPoint;
    cost = 0;
    addPoint(representative, distance);
}

void Node::setParent(Node *newParent) {
    if(newParent == nullptr){
        throw std::invalid_argument("Parent cannot be a null pointer.");
    }
    parent = newParent;
}

bool Node::isLeaf() const{
    return (leftChild == nullptr && rightChild == nullptr);
}

Node *Node::getRandomChild() {
    // This is the iterative version
    Node* currTarget = this;
    double proba = 0.0;
    while(!currTarget->isLeaf()){
        // Because the tree is binary and because the cost of a node is the sum of costs of its children,
        // it follows that (cost(left)+cost(right))/cost(this) = 1.0.
        // We choose a child v of node u with probability cost(v)/cost(u).
        // Therefore, simply worry about one node's cost (we choose without loss of generality the left child's cost).
        // Pick a random number between 0 and 1. If it is below probability of left child, pick left child and continue
        // Otherwise pick the right child and continue.
        // The process stops once we reach a leaf node, which is then returned.
        double leftChildProba = currTarget->leftChild->getCost()/currTarget->getCost();
        proba = currTarget->rng->getRandom(); // Generate a random number between 0 and 1
        if(proba <= leftChildProba){
            currTarget = currTarget->leftChild;
        } else {
            currTarget = currTarget->rightChild;
        }
    }
    return currTarget;
}

void Node::propagateUpCostsParentOneSided() {
    // To do that, we will walk up each node in the parent tree, iteratively. We will modify the costs accordingly.
    // An important hypothesis is that the cost of current node is set.
    if(cost == -1.0){
        throw std::logic_error("Cost of node should be computed before updating parent's costs. Insert at least a representative or point!");
    }

    Node* currTarget = this;
    Node* leftC = nullptr;
    Node* rightC = nullptr;
    // Walk all the way to the root
    double otherChildCost;
    while(currTarget->parent != nullptr){
        // To update cost, instead of calling computeCost() which will walk all the way down the tree, the parent node
        // will need to look at which child is "calling" for a change.
        leftC = currTarget->parent->leftChild;
        rightC = currTarget->parent->rightChild;
        if(leftC == currTarget){
            otherChildCost = rightC->getCost();
            if(otherChildCost == -1){
                throw std::logic_error("Cost of right child is not initialized. Should call this method once costs are set.");
            }
            currTarget->parent->setCost(currTarget->cost + otherChildCost);
        } else {
            otherChildCost = leftC->getCost();
            if(otherChildCost == -1){
                throw std::logic_error("Cost of left child is not initialized. Should call this method once costs are set.");
            }
            currTarget->parent->setCost(currTarget->cost + otherChildCost);
        }
        // Move up the tree
        currTarget = currTarget->parent;
    }

}


void Node::updateCostBasedOnChildren() {
    if(leftChild == nullptr){
        throw std::logic_error("Cannot have a null pointer child for left children cost computation");
    }
    if(rightChild == nullptr){
        throw std::logic_error("Cannot have a null pointer child for right children cost computation");
    }

    if(leftChild->getCost() == -1){
        throw std::logic_error("Left child cost not initialized!");
    }

    if(rightChild->getCost() == -1){
        throw std::logic_error("Right child cost not initialized!");
    }

    setCost(leftChild->getCost()+rightChild->getCost());
}

bool Node::isInInterval(double x, double min, double max, bool inclusive){
    if(max < min){
        throw std::invalid_argument("Max should be greater or equal than min");
    }
    double res= (x-min)*(max-x);
    return inclusive? res >=0 : res > 0;
}

Point *Node::selectNewClusterRep(Distance distance) {
    /**
     * We will select a point, among all possible points, following a very special distribution: each point has
     * probability proportional to how far away from representative Ql it is
     * (it is in fact dist^2(p, Ql)/cost(this), where p is a given point).
     */
    // Can only be called on leaf nodes
    if(!isLeaf()){
        throw std::logic_error("Selecting a new cluster representative only makes sense on a leaf node!");
    }
    if(pointSet.size()<=1){
        throw std::logic_error("Call selectNewCluster only once at least one non representative point is available.");
    }
    // First, get a random number
    double proba = rng->getRandom();

    // Now, we will check in which interval it falls.
    double startP = 0.0;
    double endP = 0.0;
    for(Point* p: pointSet){
        double dist(p->computeDistance(*representative,distance));
        endP += (dist*dist)/cost;
        if(isInInterval(proba, startP, endP, true)){
            // We have selected a point! Return it immediately
            return p;
        }
        // Otherwise, our next interval of probability starts at the end of current probability interval
        startP = endP;
    }
    // If we did not return any point, it means something went wrong.
    throw std::range_error("Should have selected at least a point, but probability never fell within normalized cost intervals");
}

Point* Node::splitNode(Distance distance, Node* n1, Node* n2) {
    if(!isLeaf()){
        throw std::logic_error("Splitting node only makes sense on leaf nodes!");
    }

    /*
     * Splitting involves the following steps:
     * 1- Create child nodes, leftChild and rightChild. Associate them to current node
     * 2- Select a new node representative as far away as current representative as possible
     * 3- For left node, set the representative elected in 2- as its representative. Right node will use current representative
     * 4- Split current array of points between left and right nodes, depending on proximity from each representative
     * 5- Update the costs
     * 6- Return the new representative
     */
    Point * newRep = selectNewClusterRep(distance);

    unsigned int n_elems = pointSet.size();
    Node * left;
    Node * right;

    if(n1 == nullptr || n2 == nullptr){
        // We must remember to delete these two nodes from this Node, because they were created from this Node
        shouldDeleteChildren = true;
        left = new Node(n_elems);
        right = new Node(n_elems);
    } else {
        left = n1;
        right = n2;
    }
    setAsChild(left, true);
    setAsChild(right, false);
    left->setRepresentative(newRep, distance);
    right->setRepresentative(representative, distance);

    // Split points between the two array
    double leftD = 0;
    double rightD = 0;
    for(Point* p: pointSet){
        leftD = p->computeDistance(*left->getRepresentative(), distance);
        rightD = p->computeDistance(*right->getRepresentative(), distance);
        if(leftD < rightD){
            if(p != newRep){
                left->addPoint(p, distance);
            }
        } else {
            if(p != representative){
                right->addPoint(p, distance);
            }
        }
    }

    // Clearing up current node, which is no longer leaf and hence has no more pointSet nor representative point
    pointSet.clear();
    representative = nullptr;


    // Update this node's cost based on its new children
    updateCostBasedOnChildren();

    // Propagate the new cost up the tree
    propagateUpCostsParentOneSided();

    return newRep;
}

void Node::setAsChild(Node *targetNode, bool isLeftChild) {
    if(targetNode == nullptr){
        throw std::invalid_argument("Target node cannot be a null pointer");
    }
    if(isLeftChild){
        leftChild = targetNode;
    } else {
        rightChild = targetNode;
    }
    targetNode->parent = this;
}

void Node::addPoint(Point *newPoint, Distance distance) {
    if(!isLeaf()){
        throw std::logic_error("Adding a point only makes sense on a leaf node!");
    }
    if(newPoint == nullptr){
        throw std::invalid_argument("New point cannot be a null pointer!");
    }

    if(representative == nullptr){
        throw std::logic_error("Set a representative before adding a new point to this node!");
    }

    pointSet.push_back(newPoint);
    if(cost < 0){
        throw std::runtime_error("Cost should not be negative if representative was set");
    }

    // Update cost
    if(newPoint != representative){
        double dist(newPoint->computeDistance(*representative, distance));
        cost += dist*dist;
    }

    // Update size
    size += 1;
 }

Point *Node::getRepresentative() const {
    return representative;
}

Node::Node():Node(10) {

}

Node::~Node() {
    parent = nullptr;
    if(shouldDeleteChildren){
        delete leftChild;
        delete rightChild;
    }
    if(shouldDeleteRng){
        delete rng;
    }
}

void Node::setRng(RandomGenerator *newRng) {
    if(newRng != rng){
        if(shouldDeleteRng){
            delete rng;
            shouldDeleteRng = false;
        }
        rng = newRng;
    }
}