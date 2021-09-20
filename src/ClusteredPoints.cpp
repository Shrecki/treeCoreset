//
// Created by guibertf on 9/20/21.
//

#include "ClusteredPoints.h"
#include "coreset_algorithms.h"


void ClusteredPoints::insertPoint(Point *newPoint) {
    unsigned int curr_cap = bucketCapacities[0];
    if(curr_cap < bucketCapacity){
        buckets[0]->at(curr_cap) = newPoint;
        bucketCapacities[0]++;
    } else{
        // Create empty set
        std::set<Point*> q;
        // Move all points from bucket[0] into said set ( plus p, quite obviously)
        for(int i=0; i < curr_cap; ++i){
            q.insert(buckets[0]->at(i));
        }
        q.insert(newPoint);

        unsigned int bucket_i = 1;
        curr_cap = bucketCapacities[bucket_i];
        std::vector<Point*> tmp_vec;
        tmp_vec.reserve(bucketCapacity*4);
        while(curr_cap !=0 && bucket_i < nBuckets){
            for(int i=0;i<curr_cap;i++){
                q.insert(buckets[bucket_i]->at(i));
            }

            // Convert q to vector
            tmp_vec.insert(tmp_vec.begin(), q.begin(), q.end());
            // Reduce step
            q=coreset::treeCoresetReduce(&tmp_vec, bucketCapacity);
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
                bool isin = q.count(buckets.at(nBuckets-1)->at(i));
                if(!isin){
                    buckets[nBuckets-1]->at(i)->cleanupData();
                    delete buckets[nBuckets-1]->at(i);
                }
                buckets[nBuckets-1]->at(i) = nullptr;
            }
            bucket_i = bucket_i-1;
        }
        // Whether in 1 or 2, we must now put all points from Q into Bi
        for(Point*p: q){
            curr_cap = 0;
            buckets[bucket_i]->at(curr_cap) = p;
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
        }
    }

}

ClusteredPoints::ClusteredPoints(unsigned int nBuckets, unsigned int bucketCapacity) {
    buckets.reserve(nBuckets);

    for(int i=0; i<nBuckets;++i){
        auto *tmpVec = new std::vector<Point*>();
        tmpVec->reserve(bucketCapacity);
        buckets.push_back(tmpVec);
        bucketCapacities.push_back(0);
    }
}



