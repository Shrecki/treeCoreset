//
// Created by guibertf on 9/15/21.
//

#ifndef UNTITLED_NODE_H
#define UNTITLED_NODE_H

#include "Point.h"
#include <vector>

class Node {
private:
    std::vector<Point*> pointSet;
    Point* representative;
    Node* leftChild;
    Node* rightChild;
    Node* parent;

    unsigned int size;
    double cost;
public:
    bool isLeaf() const;
    unsigned int getSize() const;
    double getCost() const;

    void setPointSet(std::vector<Point*> &newPoints);
    void setLeftChild(Node* newChild);

    void setRightChild(Node* newChild);

    void setRepresentative(Point* newPoint);
    void setParent(Node* newParent);

    explicit Node(unsigned int setInitialCapacity);

    const std::vector<Point*>* getPointSet() const{ return &this->pointSet; }

};


#endif //UNTITLED_NODE_H
