//
// Created by guibertf on 9/20/21.
//

#ifndef UNTITLED_CLUSTEREDPOINTS_H
#define UNTITLED_CLUSTEREDPOINTS_H

#include <zmq.hpp>
#include <vector>
#include "Point.h"
#include <set>
#include "Node.h"

enum Requests {
    POST_REQ, GET_REQ, LOAD_REQ, SAVE_REQ, STOP_REQ, POST_OK, GET_OK, LOAD_OK, SAVE_OK, STOP_OK, ERROR
};

class ClusteredPoints {
public:
    unsigned int nBuckets;
    unsigned int bucketCapacity;
    unsigned int nsplits;
    std::vector<std::vector<Point*>*> buckets;
    std::vector<unsigned int> bucketCapacities;
    std::vector<Node*> nodes;

    explicit ClusteredPoints(unsigned int nBuckets, unsigned int bucketCapacity);
    ~ClusteredPoints();
    void insertPoint(Point *newPoint);
};

#endif //UNTITLED_CLUSTEREDPOINTS_H+
