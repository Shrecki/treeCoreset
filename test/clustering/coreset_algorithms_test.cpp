//
// Created by guibertf on 9/17/21.
//
#include "clustering/coreset_algorithms.h"
#include "../mocks/mock_RandomGenerator.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "clustering/Point.h"
#include <chrono>

using ::testing::Return;

class CoresetTest : public ::testing::Test {
protected:
    CoresetTest() {

    }

    ~CoresetTest() override {

    }

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(CoresetTest, coresetReturnsExactlyNRepresentatives) {
    // First, we create m points
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,0, 10;
    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
    p2(std::make_unique<Eigen::VectorXd>(v2)), p3(std::make_unique<Eigen::VectorXd>(v3));
    points.push_back(&p0);
    points.push_back(&p1);
    points.push_back(&p2);
    points.push_back(&p3);

    // Then, we will ask for m representatives, which should come up as our points
    std::set<Point*> result = coreset::treeCoresetReduce(&points, points.size(), Distance::Euclidean);
    for(Point* p: points){
        EXPECT_TRUE(result.find(p) != result.end());
    }
}


TEST_F(CoresetTest, coresetReturnsSpecifiedNumberOfRepresentatives) {
    // First, we create m points
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,0, 10;

    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
    p2(std::make_unique<Eigen::VectorXd>(v2)), p3(std::make_unique<Eigen::VectorXd>(v3));
    points.push_back(&p0);
    points.push_back(&p1);
    points.push_back(&p2);
    points.push_back(&p3);

    // Then, we will ask for m representatives, which should come up as our points
    std::set<Point*> result;
    for(int i=1; i<=points.size();++i){
        result = coreset::treeCoresetReduce(&points, i, Distance::Euclidean);
        EXPECT_EQ(result.size(), i);

    }
}





TEST_F(CoresetTest, inTwoClusterCaseCoresetIdentifiesOutlyingPointAsCluster) {
    // First, we create m points
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,4,0;
    v4 << 10, 3, 10;
    v5 << 0,0, 8000;
    v6 << -4, -20, -5;

    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
    p2(std::make_unique<Eigen::VectorXd>(v2)), p3(std::make_unique<Eigen::VectorXd>(v3)),
    p4(std::make_unique<Eigen::VectorXd>(v4)),p5(std::make_unique<Eigen::VectorXd>(v5)),
    p6(std::make_unique<Eigen::VectorXd>(v6));
    points.push_back(&p0);
    points.push_back(&p1);
    points.push_back(&p2);
    points.push_back(&p3);
    points.push_back(&p4);
    points.push_back(&p5);
    points.push_back(&p6);

    // Then, we will ask for m representatives, which should come up as our points
    std::set<Point*> result = coreset::treeCoresetReduce(&points, 2, Distance::Euclidean);

    // Because p5 is such a huge outlier, it must be into the clusters.
    // Furthermore, there should be two points in the cluster.
    EXPECT_TRUE(result.find(&p5) != result.end());
    EXPECT_EQ(result.size(),2);
}

TEST_F(CoresetTest, optimizedCoresetWorksWithProvidedNodes){
    // First, we create m points
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,4,0;
    v4 << 10, 3, 10;
    v5 << 0,0, 8000;
    v6 << -4, -20, -5;

    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
    p2(std::make_unique<Eigen::VectorXd>(v2)), p3(std::make_unique<Eigen::VectorXd>(v3)),
    p4(std::make_unique<Eigen::VectorXd>(v4)),p5(std::make_unique<Eigen::VectorXd>(v5)),
    p6(std::make_unique<Eigen::VectorXd>(v6));
    points.push_back(&p0);
    points.push_back(&p1);
    points.push_back(&p2);
    points.push_back(&p3);
    points.push_back(&p4);
    points.push_back(&p5);
    points.push_back(&p6);

    Node n1(10), n2(10), n3(10), n4(10), n5(10);
    std::vector<Node *> nodes = {&n1, &n2, &n3, &n4, &n5};

    // Then, we will ask for m representatives, which should come up as our points
    std::set<Point*> result = coreset::treeCoresetReduceOptim(&points, 2, nodes, Distance::Euclidean);

    // Because p5 is such a huge outlier, it must be into the clusters.
    // Furthermore, there should be two points in the cluster.
    EXPECT_TRUE(result.find(&p5) != result.end());
    EXPECT_EQ(result.size(),2);
}


TEST_F(CoresetTest, coresetWhenMGreaterThanNumberOfPoints){
    // First, we create m points
    std::vector<Point*> points;
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0;
    v2 << 4,0,0;
    v3 << 0,4,0;
    v4 << 10, 3, 10;
    v5 << 0,0, 8000;
    v6 << -4, -20, -5;

    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
            p2(std::make_unique<Eigen::VectorXd>(v2)), p3(std::make_unique<Eigen::VectorXd>(v3)),
            p4(std::make_unique<Eigen::VectorXd>(v4)),p5(std::make_unique<Eigen::VectorXd>(v5)),
            p6(std::make_unique<Eigen::VectorXd>(v6));
    points.push_back(&p0);
    points.push_back(&p1);
    points.push_back(&p2);
    points.push_back(&p3);
    points.push_back(&p4);
    points.push_back(&p5);
    points.push_back(&p6);

    Node n1(10), n2(10), n3(10), n4(10), n5(10);
    std::vector<Node *> nodes = {&n1, &n2, &n3, &n4, &n5};

    // Then, we will ask for m representatives, which should come up as our points
    std::set<Point*> result = coreset::treeCoresetReduceOptim(&points, 8, nodes, Distance::Euclidean);

    // In this case, we expect the set to have exactly the size of original data and to contain all data
    EXPECT_EQ(result.size(), points.size());

    for(auto &p: points){
        EXPECT_TRUE(result.find(p) != result.end());
    }

}