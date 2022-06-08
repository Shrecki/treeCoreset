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

    /**
     * TreeCoreset algorithm of the StreamKM algorithm.
     * This is a naive implementation, which given points will allocate new nodes on the heap, potentially causing loss of performance as all nodes will have to be deallocated
     * once we leave the tree, leading to potentially a huge number of allocation and deallocation as the number of points tends to grow.
     * @see treeCoresetReduceOptim
     * @cite Ackermann, Marcel R., et al. "Streamkm++ a clustering algorithm for data streams." Journal of Experimental Algorithmics (JEA) 17 (2012): 2-1.
     * @param points
     * @param m
     * @return
     */
    std::set<Point*> treeCoresetReduce(std::vector<Point*>* points, unsigned int m);

    /**
     * Optimized version of the TreeCoreset algorithm from the perspective of memory management, this method expects to be passed already initialized nodes.
     * It will then simply modify the nodes during the course of the algorithm to obtain a compressed representation of the points.
     * Note that points that are no longer used by the representation will be freed from memory.
     * If the passed array of nodes does not at least 2^(m-1) + 1 elements, this method will fall back to the non optimized version and allocate
     * nodes as is needed (which will be exactly 2^(m-1) + 1 nodes).
     * @see treeCoresetReduce
     * @param points
     * @param m
     * @param nodes
     * @return
     */
    std::set<Point *>
    treeCoresetReduceOptim(std::vector<Point *> *points, unsigned int m, std::vector<Node *> &nodes, Distance distance);


}

