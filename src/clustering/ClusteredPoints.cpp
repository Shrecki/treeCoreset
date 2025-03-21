//
// Created by guibertf on 9/20/21.
//

#include "ClusteredPoints.h"
#include "coreset_algorithms.h"
#include "kmeansplusplus.h"


#include <iostream>
#include <algorithm>


#if PYTHON_BIND == 1
/*#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python.hpp>*/
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#endif



ClusteredPoints::ClusteredPoints(unsigned int nBuckets, unsigned int bucketCapacity, Distance distance)
        : nBuckets(nBuckets),
          bucketCapacity(bucketCapacity), nsplits((unsigned int)2*(bucketCapacity-1) + 1), dimension(-1), otherBucketsFull(false),
          distance(distance){
    std::cout << "N_buckets : " << std::to_string(nBuckets) << " Bucket capacity: " << std::to_string(bucketCapacity) << std::endl;
    buckets.reserve(nBuckets);
    for(int i=0; i<nBuckets;++i){
        auto *tmpVec = new std::vector<Point*>();
        tmpVec->reserve(bucketCapacity);
        buckets.push_back(tmpVec);
        bucketCapacities.push_back(0);
    }


    // We will allocate exactly 2*(m-1)+1 nodes
    for(int i=0; i < nsplits; ++i){
        nodes.push_back(new Node(10));
    }
}


ClusteredPoints::ClusteredPoints(unsigned int nBuckets, unsigned int bucketCapacity)
        : ClusteredPoints(nBuckets, bucketCapacity, Distance::Euclidean) {

}


void ClusteredPoints::insertVectors(Eigen::MatrixXd &vectors, unsigned int n_points){
    // Assumption: we receive someting that is row major.
    // To simplify these assumptions, we require the number of points to be passed along. This way we can always transpose
    // the incoming matrix to manipulate precisely what we want (ie operate by rows or by cols)
    bool operate_by_rows = vectors.rows() == n_points;

    std::cout << "Matrix is " << vectors.rows() << " by " << vectors.cols() << std::endl;

    if(!operate_by_rows && vectors.cols() != n_points){
        throw std::logic_error("The provided matrix has no dimension agreeing with the provided number of points.");
    }

    if(!operate_by_rows){
        vectors.transposeInPlace();
    }
    // Allocate a new point to be created. We make the vector pointer a unique_ptr and are on our way
    for(int i=0; i < n_points; ++i){
        Point *p;
        auto* vec = new Eigen::VectorXd(vectors.row(i));
        p = new Point(std::unique_ptr<Eigen::VectorXd>(vec));
        //std::cout << p->getData() << std::endl;
        insertPoint(p);
        std::cout << "Inserted a point !" << std::endl; //<< p->getData() << std::endl;
    }

    // Restore original matrix afterwards
    if(!operate_by_rows){
        vectors.transposeInPlace();
    }
}

void ClusteredPoints::insertPoint(Point *newPoint) {
    if(newPoint == nullptr){
        throw std::logic_error("Cannot add a nullptr as point");
    }
    if(newPoint->getDataRef()->size() == 0){
        throw std::logic_error("Cannot add a point of 0 dimension");
    }
    if(dimension == -1){
        dimension = newPoint->getDataRef()->size();
    }
    if(newPoint->getDataRef()->size()!=dimension){
        throw std::logic_error("Cannot add a point with this dimension ("+ std::to_string(newPoint->getDataRef()->size()) +
        ") when the first point had a different dimension (" + std::to_string(dimension)+")");
    }
    //auto data = newPoint->getDataRef();
    for(int i=0; i < dimension; ++i){
        assert(!std::isnan((*newPoint->getDataRef())(i)));
    }
    unsigned int curr_cap(0);
    if(buckets[0]->size()== buckets[0]->capacity()){
        throw std::logic_error("Bucket 0 cannot be full before point insertion.");
    }
    buckets[0]->push_back(newPoint);
    bucketCapacities[0]++;
    if(buckets[0]->size()== buckets[0]->capacity()){
        //std::cout << "Moving buckets ";
        otherBucketsFull = true;
        // Create set q
        std::set<Point*> q;
        // Move all points from bucket[0] into set q
        q.insert(buckets[0]->begin(), buckets[0]->end());
        // By construction, not point in here should be a nullptr, but we should check
        assert(q.find(nullptr) == q.end());
        assert(q.size() == buckets[0]->size());
        // Start to look at next bucket to see if it is empty or not
        unsigned int bucket_i = 1;
        assert(bucket_i < bucketCapacities.size());
        curr_cap = bucketCapacities[bucket_i];
        std::vector<Point*> tmp_vec;
        tmp_vec.reserve(bucketCapacity*4);
        while(curr_cap != 0 && bucket_i < nBuckets){
            assert(curr_cap == bucketCapacity); // A non-empty bucket at position other than 0 should have EXACTLY m points
            // Copy all valid points from bucket_i in q (notice that, since these are sets, this is like taking the union)
            std::copy_if(buckets[bucket_i]->begin(), buckets[bucket_i]->end(),
                         std::inserter(q, q.end()), [](auto val){return val != nullptr;});

            // Convert q to vector
            tmp_vec.insert(tmp_vec.begin(), q.begin(), q.end());
            // Reduce step
            //std::cout << "Size: " << tmp_vec.size() << " for bucket " << bucket_i << std::endl;

            q= coreset::treeCoresetReduceOptim(&tmp_vec, bucketCapacity, nodes, distance);
            ++bucket_i;
            if(bucket_i == nBuckets){
                break; // Otherwise this would result in a segfault in our loop
            }
            curr_cap = bucketCapacities[bucket_i];

            // RESET HERE tmp_vec !!!
            tmp_vec.clear();
        }

        // There are 2 possibilities:
        //  1. We reached an empty Bi. It means we can copy all points from Qi without a worry in the world
        //  2. Somehow, we reached the last Bi and it was not empty, and we now have an aggregate of its points in Qi.
        // First case is easy, because nothing needs to be freed.
        // Second case is harder, because for each pointer to insert we must decide whether to free a pointer or not.
        if(bucket_i == nBuckets){ // This is case 2
            curr_cap = buckets[nBuckets-1]->size();
            // Each point in the last bucket may or may not have been selected as a representative by the reduce op
            for(int i=0; i < curr_cap; ++i){
                bool isin = q.find(buckets[nBuckets-1]->at(i)) != q.end();
                // If the point is not a representative, it can be freed from memory
                if(!isin){
                    // Cleanup point data
                    buckets[nBuckets-1]->at(i)->cleanupData();
                    // Delete it
                    //delete buckets[nBuckets-1]->at(i);
                }
                // Even if the memory referred by the pointer is not deleted, it should be set to nullptr to ensure
                // later usages do not point to it by accident
                buckets[nBuckets-1]->at(i) = nullptr;
            }
            // Empty the bucket
            buckets[nBuckets-1]->clear();

            bucket_i = nBuckets-1;
            curr_cap = 0;
        }
        // Whether in 1 or 2, we must now put all points from Q into Bi (copy back Q into Bi)
        for(Point*p: q){
            buckets[bucket_i]->push_back(p);
            ++curr_cap;
        }
        bucketCapacities[bucket_i] = curr_cap;

        // Now invalidate all pointers of previous buckets (which are now empty)
        // Because we have aggregated all points from bucket 0 all the way to bucket_i-1, we should invalidate all pointers
        // in these buckets.
        // Note that because we are dealing with pointers, we should only free pointers NOT in Q, otherwise we will unwillingly
        // free memory within Q as well.
        for(int i=0; i<bucket_i; i++){
            for(int j=0;j<buckets[i]->size();++j){
                // If point not in q, it can be freed from memory
                if(q.find(buckets[i]->at(j)) == q.end()){
                    buckets[i]->at(j)->cleanupData();
                    //delete buckets[i]->at(j);
                }
                buckets[i]->at(j)=nullptr;
            }
            bucketCapacities[i] = 0;
            buckets[i]->clear();
        }
    }
}


ClusteredPoints::~ClusteredPoints() {
    for(int i=0; i < buckets.size(); ++i){
        for(int j=0; j < buckets[i]->size(); ++j){
            Point *p =buckets[i]->at(j);
            if(p != nullptr){
                p->cleanupData();
            }
            buckets[i]->at(j)=nullptr;
        }
        buckets[i]->clear();
        delete buckets[i];
        buckets[i] = nullptr;
    }

    for(int i=0; i < nsplits; ++i){
        delete nodes.at(i);
        nodes.at(i) = nullptr;
    }
    nodes.clear();
    buckets.clear();
    bucketCapacities.clear();
}


Eigen::MatrixXd ClusteredPoints::getRepresentativesAsMatrix() {
    std::vector<Point *> reps = getRepresentatives();
    unsigned int n_points = reps.size();
    Eigen::MatrixXd mat(n_points, dimension);
    for(int i=0; i < n_points; ++i){
        mat.row(i) = reps.at(i)->getData();
    }
    return mat;
}

std::vector<Point *> ClusteredPoints::getRepresentatives() {
    // This method must:
    // 1- Compute the union of all buckets
    // 2- Perform coresetreduce on it
    // We don't care about kmeans++, some other part of the program can handle it
    std::set<Point*> q;
    std::vector<Point*> u;

    // Union of all buckets
    for(auto v: buckets){
        q.insert(v->begin(), v->end());
    }

    // Convert to vector
    u.insert(u.begin(), q.begin(), q.end());

    // Reduce step
    q= coreset::treeCoresetReduceOptim(&u, bucketCapacity, nodes, distance);

    // Convert back to vector and return
    std::vector<Point*> results;
    results.insert(results.begin(), q.begin(), q.end());

    return results;
}

std::vector<Point *> ClusteredPoints::getUnionOfBuckets(int startBucket, int endBucket) {
    std::set<Point *> u;
    for(int i=startBucket; i < endBucket; ++i){
        std::copy_if(buckets.at(i)->begin(), buckets.at(i)->end(), std::inserter(u, u.end()), [](auto val){return val != nullptr;});
        //u.insert(buckets.at(i)->begin(),buckets.at(i)->end());
    }
    std::vector<Point *> vec(u.begin(), u.end());

    // A sanity check to assert that:
    // - No null pointer should be copied in the union
    // - No nan should be present within any point
    // - Pointers point to valid memory location
    for(auto &p: vec){
        assert(p != nullptr);
        //Eigen::VectorXd v = p->getDataRef()->size();
        for(int i = 0; i < p->getDataRef()->size(); ++i){
            assert(!std::isnan((*p->getDataRef())(i)));
        }

    }
    return vec;
}

void ClusteredPoints::setAllToNullPtr() {
    for(auto & bucket : buckets){
        for(auto & i : *bucket){
            i = nullptr;
        }
    }
}

void ClusteredPoints::reduceBuckets(){
    // std::cout << "Starting reduce on buckets " << std::endl;
    if(otherBucketsFull){
        std::vector<Point *> currPoints = getUnionOfBuckets(0, buckets.size());
        std::set<Point *> representativeSet = coreset::treeCoresetReduceOptim(&currPoints, bucketCapacity, nodes,
                                                                              distance);

        // Update all buckets: bucket 0 will get all representative sets, whereas other buckets will be emptied
        // Note here: we should check for all points if they are in the representative set, free them otherwise
        for(auto &b: buckets){
            for(int i=0; i < b->size(); ++i){
                // If not in representatives, clean it up
                if(representativeSet.find(b->at(i)) == representativeSet.end()){
                    b->at(i)->cleanupData();
                }
                b->at(i) = nullptr;
            }
            b->clear();
        }
        for(auto &p: representativeSet){
            buckets.at(0)->push_back(p);
        }
        otherBucketsFull = false;
    }
}

void ClusteredPoints::performUnionCoresetAndGetRepresentativesAsFlattenedArray(std::vector<double> &data){
    reduceBuckets();
    std::vector<Eigen::VectorXd> representatives;
    for(auto &b: *buckets.at(0)){
        representatives.push_back(b->getData());
    }
    kmeans::convertFromVectorOfEigenXdToArray(data, representatives);
}

void ClusteredPoints::getClustersAsFlattenedArray(std::vector<double> &data, unsigned int k, int epochs) {
    if(k == 0){
        throw std::invalid_argument("Number of centroids cannot be zero.");
    }
    // Run coreset on union of buckets
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    reduceBuckets();

    //std::vector<Point *> currPoints = getUnionOfBuckets(0, buckets.size());
    //std::set<Point *> representativeSet = coreset::treeCoresetReduceOptim(&currPoints, bucketCapacity, nodes);
    std::vector<Point *> representatives(buckets.at(0)->begin(), buckets.at(0)->end());

    // Get best clustering out of 5 attempts
    auto t1 = high_resolution_clock::now();
    std::vector<Eigen::VectorXd> clusters = kmeans::getBestClusters(5, representatives, k, epochs, distance);
    auto t2 = high_resolution_clock::now();


    duration<double, std::milli> t2t1 = t2 - t1;
    std::cout << "Took " << t2t1.count() << " ms" << std::endl;
    // Convert to a 1D array (ie: flatten all vectors together)
    kmeans::convertFromVectorOfEigenXdToArray(data, clusters);
}



#if PYTHON_BIND == 1
/*
 * In this specific case, we want to include the boost binding!
 */
//using namespace boost::python;
namespace py = pybind11;

PYBIND11_MODULE(client_coreset, m) {
    py::class_<ClusteredPoints>(m, "ClusteredPoints").def(py::init<unsigned int, unsigned int>())
             .def("insertVectors", &ClusteredPoints::insertVectors)
             .def("getRepresentatives", &ClusteredPoints::getRepresentativesAsMatrix);
}
#endif


