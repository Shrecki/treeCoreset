//
// Created by guibertf on 9/17/21.
//

#ifndef UNTITLED_CORESET_ALGORITHMS_H
#define UNTITLED_CORESET_ALGORITHMS_H

#endif //UNTITLED_CORESET_ALGORITHMS_H

#include <vector>
#include <set>
#include "Point.h"

namespace coreset{
    std::set<Point*> treeCoresetReduce(std::vector<Point*>* points, unsigned int m);
    Eigen::VectorXd* getMapFromArray(double* array, int arraySz);

    Point* convertArrayToPoint(double* array, int arraySz);
}

