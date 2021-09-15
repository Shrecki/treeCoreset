//
// Created by guibertf on 9/15/21.
//

#ifndef UNTITLED_POINT_H
#define UNTITLED_POINT_H

#include <Eigen/Dense>

enum class Distance { Euclidean, Cosine, Correlation };

class Point {
private:
    Eigen::VectorXd * data;
public:
    Eigen::VectorXd * getData() const;
    double computeDistance(Point &otherPoint, Distance distance) const;
    Point(Eigen::VectorXd *newData);
};


#endif //UNTITLED_POINT_H
