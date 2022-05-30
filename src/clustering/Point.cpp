//
// Created by guibertf on 9/15/21.
//
#include <iostream>
#include "Point.h"

Point::Point(std::unique_ptr<Eigen::VectorXd> newData, bool wasConverted): wasConverted(wasConverted), dist(-1.0) {
    data = std::move(newData);
    for(int i=0; i < data->size(); ++i){
        assert(!std::isnan((*data)(i)));
    }

}

Eigen::VectorXd Point::getData() const{
    return *data;
}

static double computeCosineDistance(const Eigen::VectorXd &v1, const Eigen::VectorXd &v2) {
    return (v1.dot(v2)/(v1.norm()*v2.norm()));
}

static double computeCorrelationDistance(const Eigen::VectorXd &v1, const Eigen::VectorXd &v2){
    double meanP1 = v1.mean();
    double meanP2 = v2.mean();
    auto n = v1.size();
    Eigen::VectorXd centeredV1 = v1 - meanP1 * Eigen::VectorXd::Ones(n);
    Eigen::VectorXd centeredV2 = v2 - meanP2 * Eigen::VectorXd::Ones(n);
    // Correlation is nothing but cosine distance on values centered w.r.t mean
    return computeCosineDistance(centeredV1, centeredV2);
}


double Point::computeDistance(const Eigen::VectorXd &p1, const Eigen::VectorXd &p2, Distance distance){
    double d = 0;

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

double Point::computeDistance(const Point &otherPoint, Distance distance) const{
    Eigen::VectorXd p1 = getData();
    Eigen::VectorXd p2 = otherPoint.getData();
    return Point::computeDistance(p1, p2, distance);
}

Point::~Point() = default;

void Point::cleanupData() {
    if(wasConverted){
        // It came from a map, which was then cast as data
        //delete (Eigen::Map<Eigen::VectorXd>*)data;
        delete this;
    }
}

Point* Point::convertArrayToPoint(const double* array, const int arraySz){
    if(arraySz ==0) throw std::logic_error("Cannot convert an array with size 0");
    return new Point(Point::getMapFromArray(array, arraySz), true);
}

std::unique_ptr<Eigen::VectorXd> Point::getMapFromArray(const double* array, const int arraySz){
    std::unique_ptr<Eigen::VectorXd> b = std::make_unique<Eigen::VectorXd>(Eigen::VectorXd::Zero(arraySz));
    for(int i =0; i < (*b).size(); ++i){
        (*b)(i) = array[i];
    }
    for(int i=0; i < b->size(); ++i){
        assert(!std::isnan((*b)(i)));
    }
    int nNans = b->array().isNaN().sum();
    if(nNans > 0){
        std::cout << "Included " << nNans << " nans" << std::endl;
        throw std::invalid_argument("Array contains some NaN values! ");
    }
    return b;
}

Point::Point(std::unique_ptr<Eigen::VectorXd> newData): wasConverted(false), dist(-1.0) {
    // Change it here to make a deep copy of newData instead of simply using this pointer?
    data = std::move(newData);

}
