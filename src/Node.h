//
// Created by guibertf on 9/15/21.
//

#ifndef UNTITLED_NODE_H
#define UNTITLED_NODE_H

#include "Point.h"
#include <vector>
#include "RandomGenerator.h"

class Node {
private:
    std::vector<Point*> pointSet;
    Point* representative;
    Node* leftChild;
    Node* rightChild;
    Node* parent;
    RandomGenerator * rng;

    unsigned int size;
    double cost;
public:
    Node();
    explicit Node(unsigned int setInitialCapacity);

    bool isLeaf() const;

    // Getter methods
    unsigned int getSize() const;
    double getCost() const;
    const std::vector<Point*>* getPointSet() const{ return &this->pointSet; }
    Point* getRepresentative() const;
    Node* getLeftChild() const { return leftChild; }
    Node* getRightChild() const { return rightChild; }
    Node* getParent() const { return parent; }

    // Various setters, to update inner states of the Node
    void setCost(double newCost);
    void setAsChild(Node* targetNode, bool isLeftChild);
    void setPointSet(std::vector<Point*> &newPoints);
    void setRepresentative(Point* newPoint, Distance distance);
    void setParent(Node* newParent);
    void setRng(RandomGenerator* newRng){ rng = newRng;}

    // Adding a point to a Node's point set
    void addPoint(Point* newPoint, Distance distance);

    // Methods related to specific algorithms of the tree coreset paper
    // (random child selection, cluster rep selection, cost propagation, node splitting)
    Node* getRandomChild();
    void propagateUpCostsParentOneSided();
    void updateCostBasedOnChildren();
    Point* selectNewClusterRep(Distance distance);
    Point* splitNode(Distance distance);

    static bool isInInterval(double x, double min, double max, bool inclusive);

    double getRandomSample();
};


#endif //UNTITLED_NODE_H
