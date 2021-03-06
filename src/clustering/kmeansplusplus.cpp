//
// Created by guibertf on 9/22/21.
//

#include "kmeansplusplus.h"
#include <random>
#include <utility>
#include <chrono>
#include <iostream>


std::vector<Eigen::VectorXd>
kmeans::generateStartCentroids(const Eigen::VectorXd &firstCentroid, const std::vector<Point *> &inputPoints,
                               const unsigned int &k, Distance measure) {
    // Compute initialization points according to the kMeans++ algorithm
    std::vector<Eigen::VectorXd> startCentroids;
    int nPoints = inputPoints.size();

    // Initialize to have something
    for(int j=0; j < k; ++j){
        startCentroids.push_back(inputPoints.at(0)->getData());
    }

    double minDist[nPoints];
    for(int i=0; i < nPoints; ++i){
        minDist[i] = 0;
    }

    // First, select a point randomly and uniformly (generate a uniform sample and that's it)
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> unif_distribution(0, 1.0);

    startCentroids.at(0) = std::move(firstCentroid);

    double sum, minDistance, probability;
    for(int j=1; j<k; ++j){
        sum = 0;
        for(int i=0; i < nPoints; ++i){
            // Find nearest cluster among all clusters from 0 to j for this point, set result in minDistance
            minDistance = __DBL_MAX__;
            double dist(0);
            for(int c=0; c<j; c++){
                std::unique_ptr<Eigen::VectorXd> centroid_ptr = std::make_unique<Eigen::VectorXd>(startCentroids.at(c));
                dist = Point::computeDistance(centroid_ptr, inputPoints.at(i)->getDataRef(), measure);
                if(dist < minDistance){
                    minDistance = dist;
                }
            }
            minDist[i] = minDistance;
            sum += minDistance;
        }

        // Generate random sample
        probability = unif_distribution(gen);
        sum *= probability;
        for(int i=0; i < nPoints; ++i){
            sum -= minDist[i];
            if(sum>0) continue;
            startCentroids.at(j) = inputPoints.at(i)->getData();
            break;
        }
    }

    return startCentroids;
}

Threeple * kmeans::kMeansPlusPlus(const std::vector<Point *> &inputPoints, const unsigned int &k,
                                  const unsigned int &epochs,
                                  Distance measure) {
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<int> distribution(0, inputPoints.size() - 1);
    int id = distribution(gen);
    // This part can be made faster, if we specify the startCentroids vector pre allocated already such that it doesn't have to be at every call
    std::vector<Eigen::VectorXd> startCentroids = generateStartCentroids(inputPoints.at(id)->getData(), inputPoints, k,
                                                                         measure);
    // Apply kMeans with these initializations
    return kMeans(inputPoints, &startCentroids, k, epochs, measure);
}

std::vector<Eigen::VectorXd>
kmeans::getBestClusters(int nTries, const std::vector<Point *> &inputPoints, const unsigned int &k,
                        const unsigned int &epochs,
                        Distance measure) {
    if(inputPoints.size() < k){
        throw std::invalid_argument("Asking for " + std::to_string(k) + " centroids with only " +
        std::to_string(inputPoints.size()) + " representative points available. Please add more points or consider increasing number of representatives.");
    }
    std::vector<Eigen::VectorXd> bestCentroids;
    for(int i = 0; i < k; ++i){
        bestCentroids.emplace_back(Eigen::VectorXd::Zero(inputPoints.at(0)->getData().size()));
    }
    double bestCost(__DBL_MAX__);

    for(int r=0; r < nTries; ++r){
        Threeple *results = kmeans::kMeansPlusPlus(inputPoints, k, epochs, measure);
        if(results->totalCost < bestCost){
            bestCost = results->totalCost;
            bestCentroids.clear();
            bestCentroids.insert(bestCentroids.begin(), results->points.begin(), results->points.end());
        }

        delete results; // We need to deallocate this memory!
    }
    return bestCentroids;
}

int kmeans::findNearestClusterIndex(const std::vector<Eigen::VectorXd> &centroids, const Eigen::VectorXd &point,
                                    Distance measure) {
    int assignment = 0;
    int k = centroids.size();
    double centroidToCentroidDistance[k][k];
    double dist(0);
    for(int i=0; i < k; ++i){
        for(int j=i; j < k; ++j){
            dist = Point::computeDistance(centroids.at(i), centroids.at(j), measure);
            centroidToCentroidDistance[i][j] = dist;
            centroidToCentroidDistance[j][i] = dist;
        }
        centroidToCentroidDistance[i][i] = 0.0;
    }
    double currDistance = Point::computeDistance(centroids.at(assignment), point, measure);
    for(int i=1; i < centroids.size(); ++i){
        if(i == assignment || centroidToCentroidDistance[assignment][i] >= 2.0*currDistance) continue;
        dist = Point::computeDistance(centroids.at(i), point, measure);
        if(dist < currDistance){
            currDistance = dist;
            assignment = i;
        }
    }

    return assignment;
}

// Maybe for now just implement the easy version. This way, it can be tested properly.
// We can always return to a more advanced version later down the line
Threeple * kmeans::kMeans(const std::vector<Point *> &inputPoints, const std::vector<Eigen::VectorXd> *startCentroids,
                          const unsigned int &k, const unsigned int &epochs, Distance measure) {

    //Distance measure = Distance::Euclidean;
    double THRESHOLD = 10e-6;
    if(inputPoints.empty()){
        throw std::invalid_argument("Cannot run this algorithm without points");
    }
    int nPoints = inputPoints.size();
    long dimension = inputPoints.at(0)->getData().size();

    std::vector<Eigen::VectorXd> centroids;
    std::vector<Eigen::VectorXd> previousCentroids;
    unsigned int assignments[nPoints];

    if(inputPoints.size() <=k ){
        for(int i=0; i < inputPoints.size();++i){
            Eigen::VectorXd currEigen = inputPoints.at(i)->getData();
            centroids.push_back(currEigen);
            assignments[i]=i;
        }

    } else {
        if (startCentroids != nullptr) {
            if (startCentroids->size() != k) {
                throw std::invalid_argument("Start centroids should contain exactly k elements");
            }
            centroids.insert(centroids.begin(), startCentroids->begin(), startCentroids->end());
            previousCentroids.insert(previousCentroids.begin(), startCentroids->begin(), startCentroids->end());
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
        double clusterToClusterDist[k][k];
        bool outDated[nPoints]; // Signals if assignment is outdated (Starts to false, since we initialize points to their closest cluster initially)
        double s[k];
/*
        for(auto v: centroids){
            std::cout << v.norm() << std::endl;
        }*/

        std::vector<Eigen::VectorXd> centroidMeans;
        //centroids.reserve(k);
        centroidMeans.reserve(k);
        int pointCounts[k];

        for (int j = 0; j < k; ++j) {
            centroidMeans.emplace_back(Eigen::VectorXd::Zero(dimension));
            pointCounts[j]=0;
        }

        for (int i = 0; i < nPoints; ++i) {
            for (int j = 0; j < k; ++j) {
                lowerBounds[i][j] = 0;
            }
        }


        int nearest;
        double dist(0);
        for (int i = 0; i < nPoints; ++i) {
            nearest = findNearestClusterIndex(centroids, inputPoints.at(i)->getData(), measure);
            assignments[i] = nearest;

            centroidMeans.at(nearest) += inputPoints.at(i)->getData();
            pointCounts[nearest]++;

            dist = Point::computeDistance(centroids.at(nearest), inputPoints.at(i)->getData(), measure);
            upperBounds[i] = dist;
            lowerBounds[i][nearest] = dist;
            outDated[i] = false;
        }

        bool converged(false);
        double deltaMov[k];
        unsigned int c_x(0);
        double u_x(0), newDist(0);

        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;

        Eigen::VectorXd currPoint;

        for (int iter = 0; iter < epochs; iter++) {

            // Compute cluster to cluster distance
            // Compute s(c) as smallest half distance of cluster c to any other cluster
            //auto t1 = high_resolution_clock::now();

            for (int i = 0; i < k; i++) {
                double minS(__DBL_MAX__), currDist(0.0);
                for (int j = i; j < k; ++j) {
                    currDist = Point::computeDistance(centroids.at(i), centroids.at(j), measure);
                    clusterToClusterDist[i][j] = currDist;
                    // Matrix is symmetric
                    clusterToClusterDist[j][i] = currDist;
                    if (currDist < minS) {
                        minS = currDist;
                    }
                }
                clusterToClusterDist[i][i] = 0.0;
                s[i] = 0.5 * minS;
            }
            //auto t2 = high_resolution_clock::now();

            c_x = 0;
            dist = 0;
            newDist =0;
            for (int i = 0; i < nPoints; ++i) {
                c_x = assignments[i];
                u_x = upperBounds[i];
                if (u_x <= s[c_x]) continue;
                for (int j = 0; j < k; ++j) {
                    if (j == c_x || u_x <= lowerBounds[i][j] || u_x <= 0.5 * clusterToClusterDist[c_x][j]) continue;
                    if (outDated[i]) {
                        dist = Point::computeDistance(inputPoints.at(i)->getData(), centroids.at(c_x), measure);
                        lowerBounds[i][c_x] = dist;
                    } else {
                        dist = upperBounds[i];
                    }

                    if (dist > lowerBounds[i][j] || dist > 0.5 * clusterToClusterDist[c_x][j]) {
                        currPoint = inputPoints.at(i)->getData();
                        newDist = Point::computeDistance(currPoint, centroids.at(j), measure);
                        lowerBounds[i][j] = newDist;
                        if (newDist < dist) {
                            // We're changing assignments, so remove point influence from current assignment in mean
                            unsigned int assId = assignments[i];
                            pointCounts[assId]-=1;
                            centroidMeans.at(assId) -= currPoint;

                            // Assign to new centroid
                            assignments[i] = j;

                            // Update this centroid mean
                            pointCounts[j]+=1;
                            centroidMeans.at(j) += currPoint;
                        }
                    }
                }
            }

            //auto t3 = high_resolution_clock::now();



            // Reset means
/*
            for (int j = 0; j < k; j++) {
                centroidMeans.at(j) = Eigen::VectorXd::Zero(dimension);
                pointCounts[j] = 0;
            }

            for (int i = 0; i < nPoints; ++i) {
                centroidMeans.at(assignments[i]) += inputPoints.at(i)->getData();
                pointCounts[assignments[i]]++;
            }*/
            /*for (int j = 0; j < k; j++) {
                if (pointCounts[j] != 0) {
                    centroidMeans.at(j) /= pointCounts[j];
                    std::cout << centroidMeans.at(j) << std::endl;
                }
            }*/

            for (int i = 0; i < nPoints; ++i) {
                for (int j = 0; j < k; ++j) {
                    lowerBounds[i][j] = std::max(
                            lowerBounds[i][j] - Point::computeDistance(centroids.at(j), centroidMeans.at(j), measure),
                            0.0);
                }
            }

            //auto t4 = high_resolution_clock::now();


            double delta(0);
            Eigen::VectorXd currM;
            for (int i = 0; i < nPoints; ++i) {
                c_x = assignments[i];
                currM = centroidMeans.at(c_x);
                currM /= pointCounts[c_x];
                delta = Point::computeDistance(currM, centroids.at(c_x), measure);
                upperBounds[i] += delta;
                deltaMov[c_x] = delta;
                outDated[i] = true;
            }
            // Update centroids with their means!
            for (int j = 0; j < k; ++j) {
                centroids.at(j) = centroidMeans.at(j);
                if(pointCounts[j] != 0){
                    centroids.at(j) /= pointCounts[j];
                }
                //std::cout << centroids.at(j).norm() << std::endl;
            }


            double tS(0);
            for (int j = 0; j < k; ++j) {
                tS += deltaMov[j];
            }

            tS /= k;
            auto t5 = high_resolution_clock::now();
            /*duration<double, std::milli> t2t1 = t2 - t1;
            duration<double, std::milli> t3t2 = t3 - t2;
            duration<double, std::milli> t4t3 = t4 - t3;
            duration<double, std::milli> t5t4 = t5 - t4;
            double totalTime = t2t1.count() + t3t2.count() + t4t3.count() + t5t4.count();
            std::cout << "T1: " << t2t1.count()/totalTime<<" (" << t2t1.count() <<" ms) T2: " << t3t2.count()/totalTime
            << "(" << t3t2.count() <<" ms) T3: " <<
            t4t3.count()/totalTime << " (" << t4t3.count() <<" ms) T4: "<<  t5t4.count()/totalTime<<" (" << t5t4.count() <<" ms)" <<  std::endl;*/

            if (tS < THRESHOLD) {
                break;
            }
        }
    }

    // Now, compute total cost (that is, sum of distances in each cluster)
    double totalDist(0.0);
    std::vector<unsigned int> finalAssignments;
    finalAssignments.reserve(nPoints);
    for(int i=0; i < nPoints; ++i){
        std::unique_ptr<Eigen::VectorXd> centroid_i = std::make_unique<Eigen::VectorXd>(centroids.at(assignments[i]));
        totalDist += Point::computeDistance(centroid_i, inputPoints.at(i)->getDataRef(), measure);
        finalAssignments.push_back(assignments[i]);
    }

    return new Threeple(centroids, finalAssignments, totalDist);


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
}