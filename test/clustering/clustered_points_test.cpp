//
// Created by guibertf on 9/21/21.
//
#include "clustering/ClusteredPoints.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include <fstream>
#include "../utils.h"
#include <chrono>
#include <immintrin.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

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
    ClusteredPoints clusteredPoints(10, 4, Distance::Euclidean);
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

    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
    p2(std::make_unique<Eigen::VectorXd>(v2)), p3(std::make_unique<Eigen::VectorXd>(v3));


    ClusteredPoints clusteredPoints(10, 4, Distance::Euclidean);
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


    ClusteredPoints clusteredPoints(10, 4, Distance::Euclidean);
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
    Point p1(std::make_unique<Eigen::VectorXd>(v1));


    ClusteredPoints clusteredPoints(10, 3, Distance::Euclidean);
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
    Point p1(std::make_unique<Eigen::VectorXd>(v1));


    ClusteredPoints clusteredPoints(10, 2, Distance::Euclidean);
    clusteredPoints.insertPoint(p0);
    clusteredPoints.insertPoint(&p1);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),0);
    EXPECT_EQ(clusteredPoints.buckets.at(1)->size(), 2);

}
/*
TEST_F(ClusteredPointsTest, stressTest){
    ClusteredPoints clusteredPoints(10,10);

    for(int i=0; i < 10e6; ++i){
        Eigen::VectorXd vec = Eigen::VectorXd::Random(200000);

        Point *p0 = Point::convertArrayToPoint(vec.data(), 200000);
        //Point p1(std::make_unique<Eigen::VectorXd>(vec));

        try{
            clusteredPoints.insertPoint(p0);

        } catch(std::exception &e){
            std::cout << e.what() << std::endl,
            std::cout << i << std::endl;
            break;
        }
    }

}*/

TEST_F(ClusteredPointsTest, requestingZeroCentroidsThrowsException){
    ClusteredPoints clusteredPoints(10, 2, Distance::Euclidean);

    std::vector<double> output;

    try{
        clusteredPoints.getClustersAsFlattenedArray(output, 0, 20);
    } catch(std::exception &e){
        EXPECT_STREQ(e.what(), "Number of centroids cannot be zero.");
    }

    EXPECT_THROW(clusteredPoints.getClustersAsFlattenedArray(output, 0, 20), std::invalid_argument);
}

TEST_F(ClusteredPointsTest, inserting2MPointsCausesThirdBucketToBeFilledWithExactlyMPoints){
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,0, 10;

    double array[3] = {0,0,0};
    Point *p0 = Point::convertArrayToPoint(array, 3);
    Point p1(std::make_unique<Eigen::VectorXd>(v1)), p2(std::make_unique<Eigen::VectorXd>(v2)),
    p3(std::make_unique<Eigen::VectorXd>(v3)), p4(std::make_unique<Eigen::VectorXd>(v4));


    ClusteredPoints clusteredPoints(10, 2, Distance::Euclidean);
    clusteredPoints.insertPoint(p0);
    clusteredPoints.insertPoint(&p1);
    clusteredPoints.insertPoint(&p2);
    clusteredPoints.insertPoint(&p3);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),0);
    EXPECT_EQ(clusteredPoints.buckets.at(1)->size(), 0);
    EXPECT_EQ(clusteredPoints.buckets.at(2)->size(), 2);
}

TEST_F(ClusteredPointsTest, insertingNullPtrRaisesException){
    ClusteredPoints clusteredPoints(10, 2, Distance::Euclidean);
    EXPECT_ANY_THROW(clusteredPoints.insertPoint(nullptr));
}

TEST_F(ClusteredPointsTest, insertingEmptyArrayRaisesException){
    ClusteredPoints clusteredPoints(10, 2, Distance::Euclidean);
    Eigen::VectorXd v0(0);
    Point p0(std::make_unique<Eigen::VectorXd>(v0));
    EXPECT_ANY_THROW(clusteredPoints.insertPoint(&p0));
}

TEST_F(ClusteredPointsTest, insertingPointsShouldConformToDimensionalityOfFirstInsertedPoint){
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(2);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0;

    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
    p2(std::make_unique<Eigen::VectorXd>(v2));


    ClusteredPoints clusteredPoints(10, 4, Distance::Euclidean);
    clusteredPoints.insertPoint(&p0);
    clusteredPoints.insertPoint(&p1);
    EXPECT_ANY_THROW(clusteredPoints.insertPoint(&p2));
}

TEST_F(ClusteredPointsTest, insertingPointsDynamicallyAllocatedShouldNotCauseMemLeak){
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,0, 10;

    double array0[3] = {0,0,0};
    double array1[3] = {0,5,0};
    double array2[3] = {4,0,0};
    double array3[3] = {0,0,10};

    Point *p0 = Point::convertArrayToPoint(array0, 3);
    Point *p1 = Point::convertArrayToPoint(array1, 3);
    Point *p2 = Point::convertArrayToPoint(array2, 3);
    Point *p3 = Point::convertArrayToPoint(array3, 3);

    ClusteredPoints clusteredPoints(10, 2, Distance::Euclidean);
    clusteredPoints.insertPoint(p0);
    clusteredPoints.insertPoint(p1);
    clusteredPoints.insertPoint(p2);
    clusteredPoints.insertPoint(p3);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),0);
    EXPECT_EQ(clusteredPoints.buckets.at(1)->size(), 0);
    EXPECT_EQ(clusteredPoints.buckets.at(2)->size(), 2);
}


TEST_F(ClusteredPointsTest, pointsFromFile){
    fs::path full_path(fs::current_path());
    full_path = full_path.remove_leaf();
    full_path /= "test";
    full_path /= "clustering";
    full_path /= "exampledata.csv";

    std::cout << full_path << std::endl;

    std::ifstream pointFile(full_path.string());

    std::vector<std::string> results = getNextLineAndSplitIntoTokens(pointFile);
    Eigen::VectorXd vectorTest(2);
    std::vector<Eigen::VectorXd> vectors;
    std::vector<Point*> inputPoints;
    while(!results.at(results.size()-1).empty()){
        for(int i=0; i < 2; ++i){
            std::string s = results.at(i);
            vectorTest(i) = std::stod(s);
        }
        vectors.push_back(vectorTest);
        results = getNextLineAndSplitIntoTokens(pointFile);
    }

    pointFile.close();

    for(int i=0; i < vectors.size(); ++i) {
        inputPoints.push_back(new Point(std::make_unique<Eigen::VectorXd>(vectors.at(i)), false));
    }

    int m = 3;
    int l = ceil(log2(inputPoints.size()*1.0/m)+2);
    ClusteredPoints clusteredPoints(l, m, Distance::Euclidean);
    for(int i=0; i < vectors.size(); ++i){
        clusteredPoints.insertPoint(inputPoints.at(i));
    }

    std::vector<Point*> res = clusteredPoints.getRepresentatives();
    std::cout << "Size:" << res.size() << std::endl;

    for(Point* p: res){
        std::cout << p->getData() << std::endl;
    }

    std::cout << "All done!" << std::endl;

    for(auto & inputPoint : inputPoints){
        delete inputPoint;
    }

    clusteredPoints.setAllToNullPtr();
}


TEST_F(ClusteredPointsTest, runningOnReducedVersionIsNotTooFarFromActualClusters){
    fs::path full_path(fs::current_path());
    full_path = full_path.remove_leaf();
    full_path /= "test";
    full_path /= "clustering";
    full_path /= "exampledata.csv";

    std::cout << full_path << std::endl;

    std::ifstream pointFile(full_path.string());

    std::vector<std::string> results = getNextLineAndSplitIntoTokens(pointFile);
    Eigen::VectorXd vectorTest(2);
    std::vector<Eigen::VectorXd> vectors;
    std::vector<Point*> inputPoints;
    while(!results.at(results.size()-1).empty()){
        for(int i=0; i < 2; ++i){
            std::string s = results.at(i);
            vectorTest(i) = std::stod(s);
        }
        vectors.push_back(vectorTest);
        results = getNextLineAndSplitIntoTokens(pointFile);
    }

    for(int i=0; i < vectors.size(); ++i) {
        inputPoints.push_back(new Point(std::make_unique<Eigen::VectorXd>(vectors.at(i))));
    }

    pointFile.close();

    std::ifstream labelFile("/home/guibertf/CLionProjects/treeCoreset/test/clustering/expectedLabels.csv");
    int expectedLabels[inputPoints.size()];
    int i =0;
    std::string line;
    while(std::getline(labelFile, line)){
        expectedLabels[i] = std::stoi(line);
        i = i+1;
    }

    labelFile.close();

    std::vector<Eigen::VectorXd> startCentroids;
    Eigen::VectorXd v0(2), v1(2), v2(2);
    v0 << 0,0;
    v1 << 5,0;
    v2 << 0,5;
    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
    p2(std::make_unique<Eigen::VectorXd>(v2));
    startCentroids.push_back(v0);
    startCentroids.push_back(v1);
    startCentroids.push_back(v2);

    Threeple *t = kmeans::kMeans(inputPoints, &startCentroids, 3, 100);

    int m = 3;
    int l = ceil(log2(inputPoints.size()*1.0/m)+2);
    ClusteredPoints clusteredPoints(l, m, Distance::Euclidean);
    for(int i=0; i < vectors.size(); ++i){
        clusteredPoints.insertPoint(inputPoints.at(i));
    }

    std::vector<double> array;
    clusteredPoints.getClustersAsFlattenedArray(array, 3, 100);


    // Free memory once we're done :)
    for(auto & inputPoint : inputPoints){
        delete inputPoint;
    }

    delete t;
    clusteredPoints.setAllToNullPtr();
}


