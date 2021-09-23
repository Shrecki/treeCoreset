//
// Created by guibertf on 9/22/21.
//

#include "kmeansplusplus.h"


// Maybe for now just implement the easy version. This way, it can be tested properly.
// We can always return to a more advanced version later down the line
Threeple* kmeans::kMeans(std::vector<Point*> &inputPoints, std::vector<Point*> *startCentroids, unsigned int k, unsigned int epochs){
    double THRESHOLD = 10e-6;
    if(inputPoints.empty()){
        throw std::invalid_argument("Cannot run this algorithm without points");
    }
    int nPoints = inputPoints.size();
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

    /**
     * Initialize lower bound to 0 for each point x and center c
    * Assign each point x to closest center c (todo: improve to avoid redundant computations)
    * Each time the distance b/w x and c is computed, set lower bound to this value
    * Assign upper bound as distance of x to closest center c
    **/
    double lowerBounds[nPoints][k];
    double upperBounds[nPoints];
    unsigned int assignments[nPoints];
    double clusterToClusterDist[k][k];
    bool outDated[nPoints]; // Signals if assignment is outdated (Starts to false, since we initialize points to their closest cluster initially)
    double s[k];

    std::vector<Eigen::VectorXd> centroidMeans;
    centroids.reserve(k);
    for(int j=0; j <k ; ++j){
        centroidMeans.emplace_back(Eigen::VectorXd::Zero(dimension));
    }

    for(int i=0; i < nPoints;++i){
        double minDist(__DBL_MAX__), dist(0);
        for(int j=0; j < k; ++j){
            dist = Point::computeDistance(*inputPoints.at(i)->getData(), centroids.at(j), Distance::Euclidean);
            lowerBounds[i][j]=dist;
            if(dist < minDist){
                upperBounds[i] = dist;
                minDist = dist;
                assignments[i] = j;
            }
        }
        outDated[i] = false;
    }

    bool converged(false);

    while(!converged){
        // Compute cluster to cluster distance
        // Compute s(c) as smallest half distance of cluster c to any other cluster
        for(int i=0; i<k; i++){
            double minS(__DBL_MAX__), currDist(0.0);
            for(int j=i; j<k; ++j){
                currDist = Point::computeDistance(centroids.at(i), centroids.at(j), Distance::Euclidean);
                clusterToClusterDist[i][j] = currDist;
                // Matrix is symmetric
                clusterToClusterDist[j][i] = currDist;
                if(currDist < minS){
                    minS = currDist;
                }
            }
            clusterToClusterDist[i][i] = 0.0;
            s[k] = 0.5*minS;
        }
        unsigned int c_x(0);
        double u_x(0), dist(0), newDist(0);
        for(int i=0; i < nPoints; ++i){
            c_x = assignments[i];
            u_x = upperBounds[i];
            if(u_x <= s[c_x]) continue;
            for(int j=0; j<k; ++j){
                if(j == c_x || u_x <= lowerBounds[i][k] || u_x <= 0.5*clusterToClusterDist[c_x][k]) continue;
                if(outDated[i]){
                    dist = Point::computeDistance(*inputPoints.at(i)->getData(), centroids.at(c_x), Distance::Euclidean);
                    lowerBounds[i][c_x] = dist;
                } else {
                    dist = upperBounds[i];
                }

                if(dist > lowerBounds[i][k] || dist > 0.5*clusterToClusterDist[c_x][k]){
                    newDist = Point::computeDistance(*inputPoints.at(i)->getData(), centroids.at(k), Distance::Euclidean);
                    lowerBounds[i][k] = newDist;
                    if(newDist < dist){
                        assignments[i] = k;
                    }
                }
            }
        }

        // Compute for each centroid the mean of its points
        int pointCounts[k];
        for(int j=0; j<k; j++){
            centroidMeans.at(j) = Eigen::VectorXd::Zero(dimension);
            pointCounts[j] = 0;
        }
        for(int i=1; i < nPoints; ++i){
            centroidMeans.at(assignments[i]) += *inputPoints.at(i)->getData();
            pointCounts[assignments[i]]++;
        }
        for(int j=0; j<k; j++){
            if(pointCounts[j] != 0){
                centroidMeans.at(j)/= pointCounts[j];
            }
        }

        for(int i =0; i <nPoints; ++i){
            for(int j=0; j <k; ++j){
                lowerBounds[i][j] = std::max(lowerBounds[i][j]-Point::computeDistance(centroids.at(j), centroidMeans.at(j), Distance::Euclidean), 0.0);
            }
        }

        for(int i=0; i < nPoints; ++i){
            c_x = assignments[i];
            upperBounds[i] += Point::computeDistance(centroidMeans.at(c_x), centroids.at(c_x), Distance::Euclidean);
            outDated[i] = true;
        }
        // Update centroids with their means!
        for(int j=0; j <k; ++j){
            centroids.at(j) = centroidMeans.at(j);
        }

    }



    /** Until convergence:
    * 1- Compute cluster to cluster distance
    * 2- Compute s(c) as smallest half distance to any other cluster
    * For all x:
    *    if u(x) <= s(c(x)): continue with next x
    *    For all k:
    *        if k=c(x) or u(x) <= l(x,c) or u(x) <= 1/2d(c(x), k): continue with next k
    *        if r(x):
    *            compute d(x, c(x)) (so assign this value to l(x, c(x)))
    *            r(x) = false
    *        else:
    *            d(x, c(x)) = u(x)
    *        if d(x, c(x)) > l(x,k) or d(x, c(x)) > 1/2 d(c(x),k):
    *            compute d(x,c) (so assign this value to l(x, c))
    *            if d(x,k) <= d(x, c(x)):
    *                assign c(x) = k
    * For all c:
    *     m(c) = mean of points assigned to c // Maybe we can later optimize this part
    * For all x:
    *    For all k:
    *        l(x,k) = max(l(x,k)-d(k,m(k)), 0)
    * For all x:
    *    u(x) = u(x) + d(m(c(x), c(x))
    *    r(x) = true
    * For all c:
    *    Replace center c by m(c)
    * We will keep track of the assignments
    **/




    /**unsigned int assignments[nPoints];
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

    double centerToCenterDist[k][k];
    for(int i=0; i < k;++i){
        for(int j=0; j < k; ++j){
            centerToCenterDist[i][j]=0;
        }
    }

    bool r(false), shouldStop(false);
    double z, minHalfDist, newDist;

    Eigen::VectorXd tmpCentroid;

    int e=0;
    for(; e<epochs; ++e){
        for(int j=0; j<k;++j){
            minHalfDist = __DBL_MAX__;
            newDist = 0.0;

            // Compute all centroid to centroid distances
            for(int i=0; i<k;++i){
                newDist =  Point::computeDistance(centroids.at(j), centroids.at(i), Distance::Euclidean);
                centerToCenterDist[j][i] = newDist;
                if(i !=j && newDist*0.5 < minHalfDist){
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
                z= std::max(lowerBounds[i][j], centerToCenterDist[curr_assignment][j]*0.5);
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
                lowerBounds[i][j] = Point::computeDistance(*inputPoints.at(i)->getData(), centroids.at(j), Distance::Euclidean);
                if(lowerBounds[i][j] < upperBounds[i]){
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
                lowerBounds[i][j] -= deltaMov[j];
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
 **/
    // Now, return the centroids, assignments and total sum of distances for this specific solution
    double totalDist(0.0);
    std::vector<unsigned int> finalAssignments;
    finalAssignments.reserve(nPoints);
    for(int i=0; i < nPoints; ++i){
        totalDist += Point::computeDistance(centroids.at(assignments[i]), *inputPoints.at(i)->getData(), Distance::Euclidean);
        finalAssignments.push_back(assignments[i]);
    }*
    return new Threeple(centroids, finalAssignments, totalDist);
}