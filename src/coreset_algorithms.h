//
// Created by guibertf on 9/17/21.
//

#ifndef UNTITLED_CORESET_ALGORITHMS_H
#define UNTITLED_CORESET_ALGORITHMS_H

#endif //UNTITLED_CORESET_ALGORITHMS_H

#include <vector>
#include <set>
#include "Point.h"
#include "Node.h"

namespace coreset{
    std::set<Point*> treeCoresetReduce(std::vector<Point*>* points, unsigned int m);
    std::set<Point*> treeCoresetReduceOptim(std::vector<Point*>* points, unsigned int m, std::vector<Node*> &nodes);


}

