//
// Created by guibertf on 9/21/21.
//
#include "ClusteredPoints.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"

using ::testing::Return;

class ClusteredPointsTest : public ::testing::Test {
protected:
    ClusteredPointsTest() {

    }

    ~ClusteredPointsTest() override {

    }

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(ClusteredPointsTest, clusteredPointsAllocatesProperlyExpectedCapacities) {
    ClusteredPoints clusteredPoints(10,4);
    EXPECT_EQ(clusteredPoints.buckets.size(), 10);
    for(int i=0; i<10;++i){
        EXPECT_EQ(clusteredPoints.buckets.at(i)->capacity(),4);
    }
}

TEST_F(ClusteredPointsTest, clustedPointsProperlyPerformsInsertionOfSinglePointWhenNotConverted){
    // First, we create m points
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,0, 10;

    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);


    ClusteredPoints clusteredPoints(10,4);
    clusteredPoints.insertPoint(&p0);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),1);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->at(0), &p0);
}


TEST_F(ClusteredPointsTest, clustedPointsProperlyPerformsInsertionOfSinglePointWhenConverted){
    // First, we create m points
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,0, 10;

    double array[3] = {0,0,0};
    Point *p0 = Point::convertArrayToPoint(array, 3);


    ClusteredPoints clusteredPoints(10,4);
    clusteredPoints.insertPoint(p0);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),1);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->at(0), p0);
}

TEST_F(ClusteredPointsTest, insertingBelowMPointsFillsOnlyFirstBucket){
    // First, we create m points
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,0, 10;

    double array[3] = {0,0,0};
    Point *p0 = Point::convertArrayToPoint(array, 3);
    Point p1(&v1);


    ClusteredPoints clusteredPoints(10,3);
    clusteredPoints.insertPoint(p0);
    clusteredPoints.insertPoint(&p1);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),2);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->at(0), p0);
    for(int i=1; i < 10; i++){
        EXPECT_EQ(clusteredPoints.buckets.at(i)->size(),0);
    }
}

TEST_F(ClusteredPointsTest, insertingExactlyMPointsCausesSecondBucketToBeFilledWithExactlyMPoints){
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,0, 10;

    double array[3] = {0,0,0};
    Point *p0 = Point::convertArrayToPoint(array, 3);
    Point p1(&v1);


    ClusteredPoints clusteredPoints(10,2);
    clusteredPoints.insertPoint(p0);
    clusteredPoints.insertPoint(&p1);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),0);

}

TEST_F(ClusteredPointsTest, inserting2MPointsCausesThirdBucketToBeFilledWithExactlyMPoints){

}