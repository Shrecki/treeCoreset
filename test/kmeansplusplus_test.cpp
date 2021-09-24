//
// Created by guibertf on 9/22/21.
//

#include "kmeansplusplus.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include <fstream>
#include "utils.h"
#include "math_framework.h"

using ::testing::Return;

class KMeansTest : public ::testing::Test {
protected:
    KMeansTest() {

    }

    ~KMeansTest() override {

    }

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(KMeansTest, kmeansSimpleCaseAssignsClustersCorrectly) {
    // Just to test, we will create three clusters, each of 4 points in 2D
    std::vector<Point*> points;
    Eigen::VectorXd v0(2), v1(2), v2(2), v3(2), v4(2), v5(2), v6(2), v7(2), v8(2),v9(2),v10(2),v11(2);
    v0 << 0,0;
    v1 << 0,-0.5;
    v2 << 0,0.5;
    v3 << 0.5,0.5;
    v4 << 0,10;
    v5 << 0,9.5;
    v6 << 0,10.5;
    v7 << -0.5,9.5;
    v8 << 10,0;
    v9 << 9.5,0;
    v10 << 10,-0.5;
    v11 << 10,0.5;
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3),p4(&v4), p5(&v5), p6(&v6), p7(&v7),p8(&v8), p9(&v9), p10(&v10), p11(&v11);
    std::vector<Point*> inputPoints;
    inputPoints.push_back(&p0);
    inputPoints.push_back(&p1);
    inputPoints.push_back(&p2);
    inputPoints.push_back(&p3);
    inputPoints.push_back(&p4);
    inputPoints.push_back(&p5);
    inputPoints.push_back(&p6);
    inputPoints.push_back(&p7);
    inputPoints.push_back(&p8);
    inputPoints.push_back(&p9);
    inputPoints.push_back(&p10);
    inputPoints.push_back(&p11);

    std::vector<Eigen::VectorXd> startCentroids;
    startCentroids.push_back(v0);
    startCentroids.push_back(v9);
    startCentroids.push_back(v4);
    Threeple *t = kmeans::kMeans(inputPoints, &startCentroids, 3, 100);
    Eigen::VectorXd c1 = t->points.at(0);
    Eigen::VectorXd c2 = t->points.at(1);
    Eigen::VectorXd c3 = t->points.at(2);

    unsigned int firstExpAssign(t->assignments[0]), secondClustAssign(t->assignments[4]), thirdClustAssign(t->assignments[8]);
    EXPECT_TRUE(firstExpAssign != secondClustAssign);
    EXPECT_TRUE(firstExpAssign != thirdClustAssign);

    for(int i=0; i < 4; ++i){
        EXPECT_EQ(firstExpAssign, t->assignments[i]);
        EXPECT_EQ(secondClustAssign, t->assignments[i+4]);
        EXPECT_EQ(thirdClustAssign, t->assignments[i+8]);
    }
    delete t;
}

TEST_F(KMeansTest, resultsAlignWithExternallyRanClustering){
    std::ifstream pointFile("/home/guibertf/CLionProjects/treeCoreset/test/exampledata.csv");

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
        inputPoints.push_back(new Point(&vectors.at(i)));
    }

    pointFile.close();


    std::ifstream labelFile("/home/guibertf/CLionProjects/treeCoreset/test/expectedLabels.csv");
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
    Point p0(&v0), p1(&v1), p2(&v2);
    startCentroids.push_back(v0);
    startCentroids.push_back(v1);
    startCentroids.push_back(v2);

    Threeple *t = kmeans::kMeans(inputPoints, &startCentroids, 3, 100);

    std::map<int, int> clusterKeysCorresp;
    for(int key=0; key<3;++key){
        for(int m=0; m < inputPoints.size(); ++m){
            if(expectedLabels[m] == key){
                clusterKeysCorresp.insert({key, t->assignments[m]});
                break;
            }
        }
    }

    int nPosEqs=0;
    for(int m=0; m < inputPoints.size(); ++m){
        if(clusterKeysCorresp[expectedLabels[m]] == t->assignments[m]){
            nPosEqs++;
        }
    }
    // We are willing to allow for some tolerance, because of floating point differences,
    // tie breaks and other implementation details between SKLearn and our results.
    EXPECT_TRUE(1.0- (1.0*nPosEqs/inputPoints.size()) < 0.01);

    // Free memory once we're done :)
    for(int i=0; i < inputPoints.size(); ++i){
        delete inputPoints.at(i);
    }
}

TEST_F(KMeansTest, randomInitOfNodes) {
    // Just to test, we will create three clusters, each of 4 points in 2D
    std::vector<Point*> points;
    Eigen::VectorXd v0(2), v1(2), v2(2), v3(2), v4(2), v5(2), v6(2), v7(2), v8(2),v9(2),v10(2),v11(2);
    v0 << 0,0;
    v1 << 0,1.8;
    v2 << 0,-1.5;
    v3 << 1,0;
    v4 << -2,0;

    Eigen::VectorXd vectors[5] = {v0, v1, v2, v3, v4};
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3),p4(&v4);
    std::vector<Point*> inputPoints;
    inputPoints.push_back(&p0);
    inputPoints.push_back(&p1);
    inputPoints.push_back(&p2);
    inputPoints.push_back(&p3);
    inputPoints.push_back(&p4);

    std::vector<Eigen::VectorXd> centroids;
    Eigen::VectorXd secCentroid, thirdCentroid;
    int nSteps = 10e5;
    int timesFirstPointWasV0(0);
    int countWhoWasSecondPoint[5] = {0,0,0,0,0};
    int conditionalProbsThirdStep[5][5] = {{0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}};
    int indexSec=0;
    int indexThird=0;
    for(int i=0; i < nSteps; ++i){
        centroids = kmeans::generateStartCentroids(v0, inputPoints, 3);
        if(centroids.at(0) == vectors[0]) timesFirstPointWasV0++;
        secCentroid = centroids.at(1);
        thirdCentroid = centroids.at(2);

        for(int j=0; j < 5; j++){
            if(secCentroid == vectors[j]){
                indexSec=j;
            }
            if(thirdCentroid == vectors[j]){
                indexThird=j;
            }
        }
        countWhoWasSecondPoint[indexSec]++;
        conditionalProbsThirdStep[indexSec][indexThird]++;
    }


    // We expect the second point to be chosen according to its distance from nearest centroid (which is the origin)
    int expectedFreqs[5] = {0,
                            (int)round((1.8/6.3)*nSteps),
                            (int)round((1.5/6.3)*nSteps),
                            (int)round((1.0/6.3)*nSteps),
                            (int)round((2.0/6.3)*nSteps)};

    double probsConditionals[5][5] = {{0,0,0,0,0},
                                      {0,0,1.5/4.5,1/4.5, 2/4.5},
                                      {0,1.8/4.8, 0,1/4.8, 2/4.8},
                                      {0,1.8/5.3,1.5/5.3,0,2/5.3},
                                      {0,1.8/4.3,1.5/4.3,1/4.3,0}};
    int expectedConditionals[5][5];
    for(int i=0; i < 5; ++i){
        int totalQty(0);
        for(int j=0; j < 5; ++j){
            totalQty+=conditionalProbsThirdStep[i][j];
        }
        for(int j=0;j<5;++j){
            expectedConditionals[i][j] = (int)round(probsConditionals[i][j]*totalQty);
        }
    }

    double pval = statistics::chiSquareTest(countWhoWasSecondPoint, expectedFreqs,5, 4);
    EXPECT_TRUE(pval > 0.01);
    for(int i=1; i < 5; ++i){
        pval =  statistics::chiSquareTest(conditionalProbsThirdStep[i], expectedConditionals[i],5, 4);
        EXPECT_TRUE(pval > 0.01);
        std::cout << pval << std::endl;
    }
}

TEST_F(KMeansTest, closestPointsOptimizedYieldsExpectedShortestPoint){
    Eigen::VectorXd v0(2), v1(2), v2(2), v3(2), v4(2), v5(2), v6(2), v7(2), v8(2),v9(2),v10(2),v11(2);
    v0 << 0,1.0;
    v1 << 0,5.0;
    v2 << 100,100;
    v3 << 1,0;
    v4 << -2,0;

    v5 << 0.1, 10.2;
    v6 << 0, 0;
    v7 << -1.5, 0;
    v8 << -0.5,0;
    v9 << 0.5,0;


    Eigen::VectorXd vectors[5] = {v0, v1, v2, v3, v4};
    std::vector<Eigen::VectorXd> centroids;
    for(int i=0; i < 5; ++i){
        centroids.push_back(vectors[i]);
    }
    int index = kmeans::findNearestClusterIndex(centroids, v5);
    EXPECT_EQ(index, 1);

    index = kmeans::findNearestClusterIndex(centroids, v6);
    EXPECT_EQ(index, 0);

    index = kmeans::findNearestClusterIndex(centroids, v7);
    EXPECT_EQ(index, 4);


    index = kmeans::findNearestClusterIndex(centroids, v8);
    EXPECT_EQ(index, 0);

    index = kmeans::findNearestClusterIndex(centroids, v9);
    EXPECT_EQ(index, 3);
}




TEST_F(KMeansTest, kmeansplusplusDoesntBreak) {
    // Just to test, we will create three clusters, each of 4 points in 2D
    std::vector<Point *> points;
    Eigen::VectorXd v0(2), v1(2), v2(2), v3(2), v4(2), v5(2), v6(2), v7(2), v8(2), v9(2), v10(2), v11(2);
    v0 << 0, 0;
    v1 << 0, 1.8;
    v2 << 0, -1.5;
    v3 << 1, 0;
    v4 << -2, 0;

    Eigen::VectorXd vectors[5] = {v0, v1, v2, v3, v4};
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3), p4(&v4);
    std::vector<Point *> inputPoints;
    inputPoints.push_back(&p0);
    inputPoints.push_back(&p1);
    inputPoints.push_back(&p2);
    inputPoints.push_back(&p3);
    inputPoints.push_back(&p4);

    std::vector<Eigen::VectorXd> centroids;

    Threeple *t = kmeans::kMeansPlusPlus(inputPoints, 3, 100);

    std::vector<Eigen::VectorXd> bestCenters = kmeans::getBestClusters(5, inputPoints, 3, 100);
}