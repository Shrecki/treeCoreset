//
// Created by guibertf on 9/17/21.
//

#include <iostream>
#include "coreset_algorithms.h"
#include "Node.h"
#include <chrono>

std::set<Point *> coreset::treeCoresetReduce(std::vector<Point *> *points, unsigned int m, Distance measure) {
    std::vector<Node*> nodes;
    return treeCoresetReduceOptim(points, m, nodes, measure);
}


std::set<Point *>
coreset::treeCoresetReduceOptim(std::vector<Point *> *points, unsigned int m, std::vector<Node *> &nodes,
                                Distance distance) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    unsigned int n=points->size();
    std::set<Point*> s;
    std::vector<Point*> tmpV;
    tmpV.reserve(m);

    bool useOnlyRoot(false);
    if(nodes.size() < 2*(m-1)+1){
        // We will only use the first node
        useOnlyRoot = true;
    }
    if(points->size() <= m){
        s.insert(points->begin(), points->end());
        return s;
    } else {
        //std::cout << "Starting main coreset alg" << std::endl;
        auto t1 = high_resolution_clock::now();
        // 1. Select q1 at random from P (uniformly random this time)
        std::random_device rd;  //Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
        std::uniform_int_distribution<> distrib(0, int(n - 1));
        unsigned int index = distrib(gen);
        Point *q1(points->at(index));

        auto t2 = high_resolution_clock::now();
        // 2. Create the root node with representative q1 and points as point set
        Node *root;
        if (useOnlyRoot) {
            root = new Node(n);
        } else {
            root = nodes.at(0);
        }
        unsigned int allocated_nodes = 1;
        root->setRepresentative(q1, distance);

        for (int i = 0; i < n; ++i) {
            if (i != index) {
                root->addPoint(points->at(i), distance);
            }
        }
        auto t3 = high_resolution_clock::now();

        // 3. Append q1 to S, the set of representatives (which we will return at end of function)
        //s.insert(q1);
        tmpV.push_back(q1);
        // 4. Main loop
        //auto tBef = high_resolution_clock::now();
        //long tChildCount = 0;
        //long tSplitCount = 0;
        for (int i = 1; i < m; ++i) {
            //tBef = high_resolution_clock::now();
            // select new child node randomly
            Node *electedChild = root->getRandomChild(); // Replace with new implem to see how runtime is affected in practice ? No, because it is actually not dominating runtime at all
            // elect new qi from root using splitNode
            //auto tMid = high_resolution_clock::now();

            Point *newRep;

            // This step is incredibly expensive. Look to optimize it
            if (useOnlyRoot) {
                newRep = electedChild->splitNode(distance, nullptr, nullptr);
            } else {
                newRep = electedChild->splitNode(distance, nodes.at(allocated_nodes),
                                                 nodes.at(allocated_nodes + 1));
            }
            //uto tAft = high_resolution_clock::now();

            // Add newRep into S only if not already present (hence the set)
            tmpV.push_back(newRep);
            //s.insert(newRep);
            allocated_nodes += 2;

            //tChildCount += (tMid - tBef).count();
            //tSplitCount += (tAft - tMid).count();
        }
        auto t4 = high_resolution_clock::now();

        if (useOnlyRoot) {
            delete root;
        } else {
            for (Node *no: nodes) {
                no->resetNode();
            }
        }
        s.insert(tmpV.begin(), tmpV.end());
        auto t5 = high_resolution_clock::now();
        duration<double, std::milli> t2t1 = t2 - t1;
        duration<double, std::milli> t3t2 = t3 - t2;
        duration<double, std::milli> t4t3 = t4 - t3;
        duration<double, std::milli> t5t4 = t5 - t4;

        //std::cout << "T1: " << t2t1.count() << " T2: " << t3t2.count() << " T3: " << t4t3.count() << " T4: " << t5t4.count() << " Size: " <<  points->size() << std::endl;
        //std::cout << "Tchoose: " << tChildCount << " Tsplit: " << tSplitCount << std::endl;
    }

    return s;
}


