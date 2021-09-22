//
// Created by guibertf on 9/15/21.
//

#ifndef UNTITLED_POINT_H
#define UNTITLED_POINT_H

#include <Eigen/Dense>
#include <vector>
#include <set>

enum class Distance { Euclidean, Cosine, Correlation };

class Point {
private:
    Eigen::VectorXd * data;
    bool wasConverted;
public:
    Eigen::VectorXd * getData() const;
    double computeDistance(Point &otherPoint, Distance distance) const;
    static double computeDistance(const Eigen::VectorXd &p1, const Eigen::VectorXd &p2, Distance distance);
    Point(Eigen::VectorXd *newData);
    explicit Point(Eigen::VectorXd *newData, bool wasConverted);
    void cleanupData();
    ~Point();
    static Point* convertArrayToPoint(double* array, int arraySz);
    static Eigen::VectorXd* getMapFromArray(double* array, int arraySz);
};



#endif //UNTITLED_POINT_H
