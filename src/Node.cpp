//
// Created by guibertf on 9/15/21.
//

#include "Node.h"

Node::Node(unsigned int setInitialCapacity):
leftChild(nullptr), rightChild(nullptr), parent(nullptr), representative(nullptr), size(0), cost(0.0){
    this->pointSet.reserve(setInitialCapacity);
}

unsigned int Node::getSize() const{
    return this->pointSet.size();
}

double Node::getCost() const{
    if(this->isLeaf()){
        /* The cost is the sum of squared distances of points to the reference */
        double d = 0;
        for(Point* p: this->pointSet){
            double c_d = p->computeDistance(*this->representative, Distance::Correlation);
            d += c_d*c_d;
        }
        return d;
    }else{
        return this->leftChild->getCost() + this->rightChild->getCost();
    }
}

void Node::setPointSet(std::vector<Point *> &newPoints) {
    this->pointSet.clear();
    for(Point*p : newPoints){
        this->pointSet.push_back(p);
    }

}

void Node::setLeftChild(Node *newChild) {
    this->leftChild = newChild;
}

void Node::setRightChild(Node *newChild) {
    this->rightChild = newChild;
}

void Node::setRepresentative(Point *newPoint) {
    this->representative = newPoint;
}

void Node::setParent(Node *newParent) {
    this->parent = newParent;
}

bool Node::isLeaf() const{
    return (this->leftChild == nullptr && this->rightChild == nullptr);
}
