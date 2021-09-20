//
// Created by guibertf on 9/17/21.
//
#include "coreset_algorithms.h"
#include "mocks/mock_RandomGenerator.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"

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

    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    points.push_back(&p0);
    points.push_back(&p1);
    points.push_back(&p2);
    points.push_back(&p3);

    // Then, we will ask for m representatives, which should come up as our points
    std::set<Point*> result = coreset::treeCoresetReduce(&points, points.size());
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

    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    points.push_back(&p0);
    points.push_back(&p1);
    points.push_back(&p2);
    points.push_back(&p3);

    // Then, we will ask for m representatives, which should come up as our points
    std::set<Point*> result;
    for(int i=1; i<=points.size();++i){
        result = coreset::treeCoresetReduce(&points, i);
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

    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3), p4(&v4),p5(&v5),p6(&v6);
    points.push_back(&p0);
    points.push_back(&p1);
    points.push_back(&p2);
    points.push_back(&p3);
    points.push_back(&p4);
    points.push_back(&p5);
    points.push_back(&p6);

    // Then, we will ask for m representatives, which should come up as our points
    std::set<Point*> result = coreset::treeCoresetReduce(&points, 2);

    // Because p5 is such a huge outlier, it must be into the clusters.
    // Furthermore, there should be two points in the cluster.
    EXPECT_TRUE(result.find(&p5) != result.end());
    EXPECT_EQ(result.size(),2);
}

TEST_F(CoresetTest, convertingPointWorks){
    double array[5] = {1,4,6,772.3, -10.2};
    Eigen::VectorXd *data = coreset::getMapFromArray(array, 5);
    std::cout << data->size() << std::endl;
    for(int i=0; i<data->size();++i){
        std::cout << (*data)[i] << std::endl;
    }

    delete (Eigen::Map<Eigen::VectorXd>*)data;
}