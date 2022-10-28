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
    bool otherBucketsFull;
    Distance distance;
public:
    std::vector<std::vector<Point*>*> buckets;
    std::vector<unsigned int> bucketCapacities;
    std::vector<Node*> nodes;

    explicit ClusteredPoints(unsigned int nBuckets, unsigned int bucketCapacity, Distance distance);
    explicit ClusteredPoints(unsigned int nBuckets, unsigned int bucketCapacity);

    ~ClusteredPoints();

    /**
     * @brief insertVectors algorithm, as described in the StreamKM++ method.
     * @cite Ackermann, Marcel R., et al. "Streamkm++ a clustering algorithm for data streams." Journal of Experimental Algorithmics (JEA) 17 (2012): 2-1.
     * @see treeCoresetReduceOptim
     * @param newPoint the point to insert
     */
    void insertPoint(Point *newPoint);


    void insertVectors(Eigen::MatrixXd &vectors, unsigned int n_points);

    /**
     * @brief Given the buckets, perform the tree coreset reduction on the union of points of all buckets and return the m resulting points, as described in the StreamKM++ algorithm.
     * @see Ackermann, Marcel R., et al. "Streamkm++ a clustering algorithm for data streams." Journal of Experimental Algorithmics (JEA) 17 (2012): 2-1.
     * @return m representative points
     */
    std::vector<Point*> getRepresentatives();

    Eigen::MatrixXd getRepresentativesAsMatrix();

    void setAllToNullPtr();

    /**
     * @brief Returns union of bucket pointers as a vector
     * @param startBucket
     * @param endBucket
     * @return
     */
    std::vector<Point*> getUnionOfBuckets(int startBucket, int endBucket);

    /**
     * @brief Get union of points of all buckets. Perform coreset reduce algorithm on said union to obtain m
     * representative points, and run kmeans++ five times to get best cluster assignments.
     * Finally write out the clusters in the data array as a single contiguous array.
     * @param data Vector of doubles, storing contiguously the centroids of the best assignment.
     * @param k
     * @param epochs
     */
    void getClustersAsFlattenedArray(std::vector<double> &data, unsigned int k, int epochs);

    void reduceBuckets();

    void performUnionCoresetAndGetRepresentativesAsFlattenedArray(std::vector<double> &data);

    int getDimension(){
        return dimension;
    }

};

#endif //UNTITLED_CLUSTEREDPOINTS_H+
