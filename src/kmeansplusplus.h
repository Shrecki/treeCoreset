//
// Created by guibertf on 9/22/21.
//

#ifndef UNTITLED_KMEANSPLUSPLUS_H
#define UNTITLED_KMEANSPLUSPLUS_H

#include "Point.h"
#include <vector>
#include <cmath>
class Threeple{
public:
    std::vector<Eigen::VectorXd> points;
    std::vector<unsigned int> assignments;
    double totalCost;
    Threeple(std::vector<Eigen::VectorXd> &points, std::vector<unsigned int> &assignments, double totalCost):
    totalCost(totalCost){
        this->points = std::move(points);
        this->assignments = std::move(assignments);
    }
};

namespace kmeans {

    /**
     * @brief Generates start centroids. The first centroid is chosen randomly, the others as a function of their distance then.
     * @param inputPoints
     * @param k
     * @return
     */
    std::vector<Eigen::VectorXd> generateStartCentroids(Eigen::VectorXd firstCentroid, std::vector<Point *> inputPoints, unsigned int k);

        /**
         * k-means algorithm, with the potential to specify initial startCentroids (otherwise it is generated by taking k points randomly)
         * The algorithm stops after specified epochs or early if no new point assignment occurred in an iteration.
         * @param inputPoints
         * @param startCentroids
         * @param k
         * @param epochs number of epochs before algorithm finishes
         * @return Threeple containing centroids, point assignments and total cost of this solution
         */
    Threeple *kMeans(std::vector<Point *> &inputPoints, std::vector<Eigen::VectorXd> *startCentroids, unsigned int k, unsigned int epochs);


    /**
     * The k-means++ algorithm, as described in k-means++: The Advantages of Careful Seeding
     * First, initialize the centroids with a specific seeding
     * Then simply run the usual k-means
     * @param inputPoints
     * @param k
     * @param epochs
     * @return
     */
    Threeple *kMeansPlusPlus(std::vector<Point *> inputPoints, unsigned int k, unsigned int epochs);
}

#endif //UNTITLED_KMEANSPLUSPLUS_H
