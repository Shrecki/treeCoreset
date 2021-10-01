//
// Created by guibertf on 9/17/21.
//

#include <iostream>
#include "coreset_algorithms.h"
#include "Node.h"

std::set<Point*> coreset::treeCoresetReduce(std::vector<Point *> *points, unsigned int m) {
    std::vector<Node*> nodes;
    return treeCoresetReduceOptim(points, m, nodes);
}


std::set<Point*> coreset::treeCoresetReduceOptim(std::vector<Point *> *points, unsigned int m, std::vector<Node*> &nodes) {
    unsigned int n=points->size();
    std::set<Point*> s;

    bool useOnlyRoot(false);
    if(nodes.size() < 2*(m-1)+1){
        // We will only use the first node
        useOnlyRoot = true;
    }
    if(points->size() <= m){
        s.insert(points->begin(), points->end());
        return s;
    } else {
        // 1. Select q1 at random from P (uniformly random this time)
        std::random_device rd;  //Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
        std::uniform_int_distribution<> distrib(0, int(n - 1));
        unsigned int index = distrib(gen);
        Point *q1(points->at(index));

        // 2. Create the root node with representative q1 and points as point set
        Node *root;
        if (useOnlyRoot) {
            root = new Node(n);
        } else {
            root = nodes.at(0);
        }
        unsigned int allocated_nodes = 1;
        root->setRepresentative(q1, Distance::Euclidean);

        for (int i = 0; i < n; ++i) {
            if (i != index) {
                root->addPoint(points->at(i), Distance::Euclidean);
            }
        }

        // 3. Append q1 to S, the set of representatives (which we will return at end of function)
        s.insert(q1);

        // 4. Main loop
        for (int i = 1; i < m; ++i) {
            // select new child node randomly
            Node *electedChild = root->getRandomChild();
            // elect new qi from root using splitNode
            Point *newRep;
            if (useOnlyRoot) {
                newRep = electedChild->splitNode(Distance::Euclidean, nullptr, nullptr);
            } else {
                newRep = electedChild->splitNode(Distance::Euclidean, nodes.at(allocated_nodes),
                                                 nodes.at(allocated_nodes + 1));
            }
            // Add newRep into S only if not already present (hence the set)
            s.insert(newRep);
            allocated_nodes += 2;
        }

        if (useOnlyRoot) {
            delete root;
        } else {
            for (Node *no: nodes) {
                no->resetNode();
            }
        }
    }

    return s;
}


