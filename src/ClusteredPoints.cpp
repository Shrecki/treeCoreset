//
// Created by guibertf on 9/20/21.
//

#include "ClusteredPoints.h"
#include "coreset_algorithms.h"


void ClusteredPoints::insertPoint(Point *newPoint) {
    unsigned int curr_cap(0);
    if(buckets[0]->size()== buckets[0]->capacity()){
        throw std::logic_error("Bucket 0 cannot be full before point insertion.");
    }
    buckets[0]->push_back(newPoint);
    bucketCapacities[0]++;
    if(buckets[0]->size()== buckets[0]->capacity()){
        // Create set q
        std::set<Point*> q;
        // Move all points from bucket[0] into set q
        q.insert(buckets[0]->begin(), buckets[0]->end());

        unsigned int bucket_i = 1;
        curr_cap = bucketCapacities[bucket_i];
        std::vector<Point*> tmp_vec;
        tmp_vec.reserve(bucketCapacity*4);
        while(curr_cap !=0 && bucket_i < nBuckets){
            // Copy all points from bucket_i in q (notice that, since these are sets, this is like taking the union)
            q.insert(buckets[bucket_i]->begin(), buckets[bucket_i]->end());

            // Convert q to vector
            tmp_vec.insert(tmp_vec.begin(), q.begin(), q.end());
            // Reduce step
            q=coreset::treeCoresetReduceOptim(&tmp_vec, bucketCapacity, nodes);
            bucket_i++;
            curr_cap = bucketCapacities[bucket_i];
        }

        // There are 2 possibilities:
        //  1. We reached an empty Bi. It means we can copy all points from Qi without a worry in the world
        //  2. Somehow, we reached the last Bi, which was not empty, and we now have an aggregate of its points in Qi.
        // First case is easy, because nothing needs to be freed.
        // Second case is harder, because for each pointer to insert we must decide whether to free a pointer or not.
        if(bucket_i == nBuckets){ // This is case 2
            for(int i=0; i < curr_cap; ++i){
                bool isin = q.find(buckets[nBuckets-1]->at(i)) != q.end();
                if(!isin){
                    buckets[nBuckets-1]->at(i)->cleanupData();
                    delete buckets[nBuckets-1]->at(i);
                }
                buckets[nBuckets-1]->clear();
            }
            bucket_i = bucket_i-1;
            curr_cap = 0;
        }
        // Whether in 1 or 2, we must now put all points from Q into Bi
        for(Point*p: q){
            buckets[bucket_i]->push_back(p);
            ++curr_cap;
        }
        bucketCapacities[bucket_i] = curr_cap;

        // Now invalidate all pointers not present in Q
        for(int i=0; i<bucket_i; i++){
            for(int j=0;j<bucketCapacities[i];++j){
                if(q.find(buckets[i]->at(j)) == q.end()){
                    buckets[i]->at(j)->cleanupData();
                    delete buckets[i]->at(j);
                }
                buckets[i]->at(j)=nullptr;
            }
            bucketCapacities[i] = 0;
            buckets[i]->clear();
        }
    }
}

ClusteredPoints::ClusteredPoints(unsigned int nBuckets, unsigned int bucketCapacity): nBuckets(nBuckets),
bucketCapacity(bucketCapacity), nsplits((unsigned int)pow(2, bucketCapacity-1) + 1) {
    buckets.reserve(nBuckets);

    for(int i=0; i<nBuckets;++i){
        auto *tmpVec = new std::vector<Point*>();
        tmpVec->reserve(bucketCapacity);
        buckets.push_back(tmpVec);
        bucketCapacities.push_back(0);
    }

    // We will allocate exactly 2^(m-1)+1 nodes
    for(int i=0; i < nsplits; ++i){
        nodes.push_back(new Node(10));
    }
}

ClusteredPoints::~ClusteredPoints() {
    for(int i=0; i < buckets.size(); ++i){
        for(int j=0; j < buckets[i]->size(); ++j){
            Point *p =buckets[i]->at(j);
            p->cleanupData();
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



