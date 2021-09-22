//
// Created by guibertf on 9/22/21.
//

#include "kmeansplusplus.h"

Threeple* kmeans::kMeans(std::vector<Point*> &inputPoints, std::vector<Point*> *startCentroids, unsigned int k, unsigned int epochs){
    double THRESHOLD = 10e-6;
    if(inputPoints.empty()){
        throw std::invalid_argument("Cannot run this algorithm without points");
    }
    unsigned long nPoints = inputPoints.size();
    long dimension = inputPoints.at(0)->getData()->size();

    std::vector<Eigen::VectorXd> centroids;
    std::vector<Eigen::VectorXd> previousCentroids;
    if(startCentroids != nullptr){
        if(startCentroids->size() != k){
            throw std::invalid_argument("Start centroids should contain exactly k elements");
        }
        for(auto c: *startCentroids){
            Eigen::VectorXd tmpVector;
            tmpVector = *c->getData(); // this does a deep copy of centroid's data
            centroids.push_back(tmpVector);
            previousCentroids.push_back(tmpVector);
        }
    } else {
        // @todo: Generate randomly new centroids from input points
    }


    /**
     * Elkan's algorithm is used here instead of the naive k-means
     * The reason for this is that we expect in our use-case data of very high dimension. As a consequence, computing
     * distance between points becomes more expensive, since it is proportional to data dimensionality.
     * Elkan's algorithm allows to minimize these comparisons, using triangle inequality.
     * BE CAREFUL TO USE A DISTANCE METRIC THAT SATISFIES THE TRIANGLE INEQUALITY!
     */

    // We will keep track of the assignments
    unsigned int assignments[nPoints];
    double upperBounds[nPoints];

    for(int i=0; i < nPoints; ++i){
        assignments[i] = 0;
        upperBounds[i] = __DBL_MAX__;
    }

    double sjs[k];
    double deltaMov[k];
    unsigned int pointsPerCentroid[k];

    for(int i=0; i < k; ++i){
        sjs[i] = 0.0;
        deltaMov[i] = 0.0;
        pointsPerCentroid[i] = 0;
    }

    Eigen::MatrixXd lowerBounds = Eigen::MatrixXd::Zero(nPoints,k);
    Eigen::MatrixXd centerToCenterDist = Eigen::MatrixXd::Zero(k,k);

    bool r(false), shouldStop(false);
    double z, minHalfDist, newDist;

    Eigen::VectorXd tmpCentroid;

    for(int e=0; e<epochs; ++e){
        for(int j=0; j<k;++j){
            minHalfDist = __DBL_MAX__;
            newDist = 0.0;
            for(int i=0; i<k;++i){
                newDist =  Point::computeDistance(centroids.at(j), centroids.at(i), Distance::Euclidean);
                centerToCenterDist(j,i) = newDist;
                if(newDist*0.5 < minHalfDist){
                    minHalfDist = newDist*0.5;
                }
            }
            sjs[j] = minHalfDist;
        }
        for(int i=0; i<nPoints; ++i){
            unsigned int curr_assignment = assignments[i];
            if(upperBounds[i] <= sjs[curr_assignment]){
                continue;
            }
            r = true;
            for(int j=0; j < k; ++j){
                z= std::max(lowerBounds(i,j), centerToCenterDist(curr_assignment, j)*0.5);
                if(j==curr_assignment || upperBounds[i] <= z){
                    continue;
                }
                if(r){
                    upperBounds[i] = Point::computeDistance(*inputPoints.at(i)->getData(), centroids.at(curr_assignment), Distance::Euclidean);
                    r = false;
                    if(upperBounds[i] <= z){
                        continue;
                    }
                }
                lowerBounds(i,j) = Point::computeDistance(*inputPoints.at(i)->getData(), centroids.at(j), Distance::Euclidean);
                if(lowerBounds(i,j) < upperBounds[i]){
                    assignments[i] = j;
                }
            }
        }

        // Keep track of previous centroids
        int nAssignments[k];
        Eigen::VectorXd tmpCentroids[k];
        for(int j=0;j<k;j++){
            previousCentroids.at(j) = centroids.at(j);
            nAssignments[j] = 0;
            tmpCentroids[j] =Eigen::VectorXd::Zero(dimension);
        }

        // Compute new centroids
        unsigned int currAssign;
        for(int i=0; i < nPoints; ++i){
            currAssign = assignments[i];
            tmpCentroids[currAssign] += *inputPoints.at(i)->getData();
            nAssignments[currAssign] += 1;
        }

        // Update centroids and how much they moved
        for(int i=0; i < k; ++i){
            if(nAssignments[i] != 0){
                centroids.at(i) = tmpCentroids[i]/nAssignments[i];
            } else {
                centroids.at(i) = tmpCentroids[i];
            }
            deltaMov[i] = Point::computeDistance(centroids.at(i), previousCentroids.at(i), Distance::Euclidean);
        }

        // Update the upper and lower distance bounds
        for(int i=0; i < nPoints; i++){
            upperBounds[i] += deltaMov[assignments[i]];
            for(int j=0; j<k; ++j){
                lowerBounds(i,j) -= deltaMov[j];
            }
        }

        // We can check something here: if overall distance by which the cluster centers moved is smaller than some threshold
        // we can terminate early, because we have reached an optimum.
        double distSum(0.0);
        for(int j = 0; j <k; j++){
            distSum += deltaMov[j];
        }
        if(distSum < THRESHOLD){
            break;
        }
    }

    // Now, return the centroids, assignments and total sum of distances for this specific solution
    double totalDist(0.0);
    std::vector<unsigned int> finalAssignments;
    finalAssignments.reserve(nPoints);
    for(int i=0; i < nPoints; ++i){
        totalDist += Point::computeDistance(centroids.at(assignments[i]), *inputPoints.at(i)->getData(), Distance::Euclidean);
        finalAssignments.push_back(assignments[i]);
    }
    return new Threeple(centroids, finalAssignments, totalDist);
}