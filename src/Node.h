//
// Created by guibertf on 9/15/21.
//

#ifndef UNTITLED_NODE_H
#define UNTITLED_NODE_H

#include "Point.h"
#include <vector>
#include "RandomGenerator.h"
#include <memory>

/**
 * Class describing a node. It should be noted that a node, in itself, is sufficient to describe a binary tree
 */
class Node {
private:
    std::vector<Point*> pointSet;
    Point* representative;
    Node* leftChild;
    Node* rightChild;
    Node* parent;
    RandomGenerator *rng;

    unsigned int size;
    double cost;

    bool shouldDeleteChildren;
    bool shouldDeleteRng;
public:
    Node();
    explicit Node(unsigned int setInitialCapacity);
    ~Node();

    Node(const Node&) = delete;

    [[nodiscard]] bool isLeaf() const;

    // Getter methods
    [[nodiscard]] unsigned int getSize() const;
    [[nodiscard]] double getCost() const;
    [[nodiscard]] const std::vector<Point*>* getPointSet() const{ return &this->pointSet; }
    [[nodiscard]] Point* getRepresentative() const;
    [[nodiscard]] Node* getLeftChild() const { return leftChild; }
    [[nodiscard]] Node* getRightChild() const { return rightChild; }
    [[nodiscard]] Node* getParent() const { return parent; }

    // Various setters, to update inner states of the Node
    void setCost(double newCost);
    void setAsChild(Node* targetNode, bool isLeftChild);
    void setRepresentative(Point* newPoint, Distance distance);
    void setParent(Node* newParent);
    void setRng(RandomGenerator* newRng);

    // Adding a point to a Node's point set
    void addPoint(Point* newPoint, Distance distance);

    // Methods related to specific algorithms of the tree coreset paper
    // (random child selection, cluster rep selection, cost propagation, node splitting)
    Node* getRandomChild();
    void propagateUpCostsParentOneSided();
    void updateCostBasedOnChildren();
    Point* selectNewClusterRep(Distance distance);
    Point* splitNode(Distance distance, Node* n1, Node *n2);

    void resetNode();

    static bool isInInterval(double x, double min, double max, bool inclusive);

};


#endif //UNTITLED_NODE_H
