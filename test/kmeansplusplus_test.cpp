//
// Created by guibertf on 9/22/21.
//

#include "kmeansplusplus.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include <fstream>
#include "utils.h"

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

    std::vector<Point*> startCentroids;
    startCentroids.push_back(&p0);
    startCentroids.push_back(&p9);
    startCentroids.push_back(&p4);
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

    for(int i=0; i < 10; ++i) {
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

    std::vector<Point*> startCentroids;
    Eigen::VectorXd v0(2), v1(2), v2(2);
    v0 << 0,0;
    v1 << 5,0;
    v2 << 0,5;
    Point p0(&v0), p1(&v1), p2(&v2);
    startCentroids.push_back(&p0);
    startCentroids.push_back(&p1);
    startCentroids.push_back(&p2);

    Threeple *t = kmeans::kMeans(inputPoints, &startCentroids, 3, 100);


    // Free memory once we're done :)
    for(int i=0; i < inputPoints.size(); ++i){
        delete inputPoints.at(i);
    }



}