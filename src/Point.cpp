//
// Created by guibertf on 9/15/21.
//

#include "Point.h"
#include <cmath>

Point::Point(Eigen::VectorXd *newData): data(newData) {

}

Eigen::VectorXd *Point::getData() const{
    return this->data;
}

static double computeCosineDistance(Eigen::VectorXd *v1, Eigen::VectorXd *v2) {
    return (v1->dot(*v2)/(v1->norm()*v2->norm()));
}

static double computeCorrelationDistance(Eigen::VectorXd *v1, Eigen::VectorXd *v2){
    double meanP1 = v1->mean();
    double meanP2 = v2->mean();
    auto n = v1->size();
    Eigen::VectorXd centeredV1 = *v1 - meanP1 * Eigen::VectorXd::Ones(n);
    Eigen::VectorXd centeredV2 = *v2 - meanP2 * Eigen::VectorXd::Ones(n);
    // Correlation is nothing but cosine distance on values centered w.r.t mean
    return computeCosineDistance(&centeredV1, &centeredV2);
}

static double computeEuclideanDistance(Eigen::VectorXd *v1, Eigen::VectorXd *v2){
    return (*v1-*v2).norm();
}

double Point::computeDistance(Point &otherPoint, Distance distance) const{
    double d = 0;
    Eigen::VectorXd *p1 = this->getData();
    Eigen::VectorXd *p2 = otherPoint.getData();
    switch (distance) {
        case Distance::Euclidean: {
            d = computeEuclideanDistance(p1, p2);
            break;
        }
        case Distance::Cosine:
            d = computeCosineDistance(p1, p2);
            break;
        case Distance::Correlation:
            d = computeCorrelationDistance(p1, p2);
            break;
    }
    return d;
}
