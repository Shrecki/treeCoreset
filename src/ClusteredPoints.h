//
// Created by guibertf on 9/20/21.
//

#ifndef UNTITLED_CLUSTEREDPOINTS_H
#define UNTITLED_CLUSTEREDPOINTS_H

#include <zmq.hpp>
#include <vector>
#include "Point.h"
#include <set>
#include "Node.h"
#include "coreset_algorithms.h"
#include "kmeansplusplus.h"

enum Requests {
    POST_REQ, GET_REQ, LOAD_REQ, SAVE_REQ, STOP_REQ, POST_OK, GET_OK, LOAD_OK, SAVE_OK, STOP_OK, ERROR
};

/**
 * Compressed representation of a set of points, based on buckets of binary trees split on point proximities.
 * This data structure handles the logic of the buckets described in the StreamKM++ algorithm.
 * Under the hood, it relies on a tree coreset, most notably to insert points in the bucket.
 * @see Ackermann, Marcel R., et al. "Streamkm++ a clustering algorithm for data streams." Journal of Experimental Algorithmics (JEA) 17 (2012): 2-1.
 */
class ClusteredPoints {
    unsigned int nBuckets;
    unsigned int bucketCapacity;
    unsigned int nsplits;
    int dimension;
public:
    std::vector<std::vector<Point*>*> buckets;
    std::vector<unsigned int> bucketCapacities;
    std::vector<Node*> nodes;

    explicit ClusteredPoints(unsigned int nBuckets, unsigned int bucketCapacity);
    ~ClusteredPoints();

    /**
     * @brief insertPoint algorithm, as described in the StreamKM++ method.
     * @cite Ackermann, Marcel R., et al. "Streamkm++ a clustering algorithm for data streams." Journal of Experimental Algorithmics (JEA) 17 (2012): 2-1.
     * @see treeCoresetReduceOptim
     * @param newPoint the point to insert
     */
    void insertPoint(Point *newPoint);

    /**
     * @brief Given the buckets, perform the tree coreset reduction on the union of points of all buckets and return the m resulting points, as described in the StreamKM++ algorithm.
     * @see Ackermann, Marcel R., et al. "Streamkm++ a clustering algorithm for data streams." Journal of Experimental Algorithmics (JEA) 17 (2012): 2-1.
     * @return m representative points
     */
    std::vector<Point*> getRepresentatives();

    void setAllToNullPtr();

    /**
     * @brief Returns union of bucket pointers as a vector
     * @param startBucket
     * @param endBucket
     * @return
     */
    std::vector<Point*> getUnionOfBuckets(int startBucket, int endBucket);

    void getClustersAsFlattenedArray(std::vector<double> &data, int k, int epochs){
        // Run coreset on union of buckets
        std::vector<Point *> currPoints = getUnionOfBuckets(0, buckets.size());
        std::set<Point *> representativeSet = coreset::treeCoresetReduceOptim(&currPoints, bucketCapacity, nodes);
        std::vector<Point *> representatives(representativeSet.begin(), representativeSet.end());

        // Get best clustering out of 5 attempts
        std::vector<Eigen::VectorXd> clusters = kmeans::getBestClusters(5, representatives, k, epochs);

        // Convert to a 1D array (ie: flatten all vectors together)
        kmeans::convertFromVectorOfEigenXdToArray(data, clusters);
    }


};

#endif //UNTITLED_CLUSTEREDPOINTS_H+
