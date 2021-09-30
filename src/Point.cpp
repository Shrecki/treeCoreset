//
// Created by guibertf on 9/15/21.
//
#include <iostream>
#include "Point.h"

Point::Point(std::unique_ptr<Eigen::VectorXd> newData, bool wasConverted): wasConverted(wasConverted) {
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

static double computeEuclideanDistance(const Eigen::VectorXd &v1, const Eigen::VectorXd &v2){
    assert(v1.size() == v2.size());
    for(int i=0; i < v1.size(); ++i){
        assert(!std::isnan(v1(i)));
        assert(!std::isnan(v2(i)));
    }
    Eigen::VectorXd v = Eigen::VectorXd::Zero(v1.size());
    double n(0.0);
    double c;
    for(int i=0; i < v1.size(); ++i){
        //if(i == 901288){
        //    std::cout << v1(i) << std::endl;
        //    std::cout << v2(i) << std::endl;
        //}
        c = v1(i) - v2(i);
        if(std::isnan(c)){
            if(std::isnan(v1(i))){
                std::cout << "v1 nan at index " << i << std::endl;
            }
            if(std::isnan(v2(i))){
                std::cout << "v2 nan at index " << i << std::endl;
            }
        }
        v(i) = c;
        n+= c*c;
    }
    return sqrt(n);
}

double Point::computeDistance(const Eigen::VectorXd &p1, const Eigen::VectorXd &p2, Distance distance){
    double d = 0;
    for(int i=0; i < p1.size(); ++i){
        assert(!std::isnan(p1(i)));
        assert(!std::isnan(p2(i)));
    }
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

double Point::computeDistance(Point &otherPoint, Distance distance) const{
    Eigen::VectorXd p1 = getData();
    Eigen::VectorXd p2 = otherPoint.getData();
    //std::cout << (p1)(901184) << std::endl;
    //std::cout << (p1)(901194) << std::endl;
    //std::cout << (p1)(901287) << std::endl;
    //std::cout << (p1)(901288) << std::endl;
    return Point::computeDistance(p1, p2, distance);
}

Point::~Point() {
}

void Point::cleanupData() {
    if(wasConverted){
        // It came from a map, which was then cast as data
        //delete (Eigen::Map<Eigen::VectorXd>*)data;
        delete this;
    }
}

Point* Point::convertArrayToPoint(double* array, int arraySz){
    if(arraySz ==0) throw std::logic_error("Cannot convert an array with size 0");
    return new Point(Point::getMapFromArray(array, arraySz), true);
}

std::unique_ptr<Eigen::VectorXd> Point::getMapFromArray(double* array, int arraySz){
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

Point::Point(std::unique_ptr<Eigen::VectorXd> newData): wasConverted(false) {
    // Change it here to make a deep copy of newData instead of simply using this pointer?
    data = std::move(newData);

}
