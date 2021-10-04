//
// Created by guibertf on 9/15/21.
//

#ifndef UNTITLED_POINT_H
#define UNTITLED_POINT_H

#include <Eigen/Dense>
#include <vector>
#include <set>
#include <memory>

enum class Distance { Euclidean, Cosine, Correlation };


/**
 * Convenience class describe a point and allowing distance computations on it.
 */
class Point {
private:
    std::unique_ptr<Eigen::VectorXd> data;
    bool wasConverted;
    double dist;
public:
    [[nodiscard]] Eigen::VectorXd getData() const;
    /**
     * Computes distance measure between point and an other point, based on provided distance
     * @param otherPoint
     * @param distance
     * @return
     */
    [[nodiscard]] double computeDistance(const Point &otherPoint, Distance distance) const;
    /**
     * Distance computation between two vectors, based on the provided distance metric
     * @param p1
     * @param p2
     * @param distance
     * @return Distance between the two points
     */
    static double computeDistance(const Eigen::VectorXd &p1, const Eigen::VectorXd &p2, Distance distance);
    explicit Point(std::unique_ptr<Eigen::VectorXd> newData);
    explicit Point(std::unique_ptr<Eigen::VectorXd> newData, bool wasConverted);
    void cleanupData();

    ~Point();
    /**
     * @brief Given an input array of double and the number of elements to consider in the array, convert said array
     * into a Point class, ie convert to an eigen vectorXd the array and initialize a new point that uses this vector.
     * @param array
     * @param arraySz
     * @return Resulting point vector
     */
    static Point* convertArrayToPoint(const double* array, int arraySz);
    /**
     * @brief Convert an input array to an eigen vectorXd and return unique pointer to said vector Xd.
     * @param array
     * @param arraySz
     * @return Unique pointer to a vector of size arraySz, containing the elements of array
     */
    static std::unique_ptr<Eigen::VectorXd> getMapFromArray(const double* array, int arraySz);

    [[nodiscard]] static double computeEuclideanDistance(const Eigen::VectorXd &v1, const Eigen::VectorXd &v2){
        return (v1-v2).norm();
    }

    void setDistance(double newDist){ dist = newDist;}

    inline double getDistance() { return dist; }
};



#endif //UNTITLED_POINT_H
