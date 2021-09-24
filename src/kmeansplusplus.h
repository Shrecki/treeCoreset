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
     * @brief: Find nearest cluster index in provided centroid vector, leveraging triangular ineq. to minimize distance computations.
     *  The minimization comes from lemma 1 of https://www.aaai.org/Papers/ICML/2003/ICML03-022.pdf, which states that given three points, a,b,c
     *  if distance(b,c) >= 2*distance(a,b) then distance(a,c) >= distance(a,b)
     *  In other words, under specific conditions, we can avoid computing distance(a,c).
     *  Obviously, it means remembering distance(a,b), which is nothing more than the distance between the point and its currently assigned centroid.
     *  The distance between b and c is then the distance between current centroid and candidate centroid, ie:
     *  if distance(current centroid, candidate centroid) >= 2*distance(point, current centroid) then we know the candidate is farther away than
     *  the current centroid, so no need to compute any distance.
     *  If this condition is not satisfied, then we compute the distance normally.
     * @param centroids
     * @param point
     * @return
     */
    int findNearestClusterIndex(const std::vector<Eigen::VectorXd> &centroids, const Eigen::VectorXd &point);

    /**
     * @brief Generates start centroids. The first centroid is chosen randomly, the others as a function of their distance then.
     * This code is an adaptation of https://github.com/ieyjzhou/KmeansPlusPlus , which was for 2D points while this implementation
     * handles points of any dimension.
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
