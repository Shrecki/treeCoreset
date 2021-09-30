//
// Created by guibertf on 9/15/21.
//
#include "Point.h"
#include <gtest/gtest.h>
#include <cmath>

class PointTest : public ::testing::Test {
protected:
    PointTest() {

    }

    ~PointTest() override {

    }

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(PointTest, EuclideanNormShouldWorkCorrectly) {
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Point point(std::make_unique<Eigen::VectorXd>(vector));

    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    Point point2(std::make_unique<Eigen::VectorXd>(v2));

    EXPECT_FLOAT_EQ(point.computeDistance(point2, Distance::Euclidean),4);
}

TEST_F(PointTest, CosineNormShouldWorkCorrectly) {
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Point point(std::make_unique<Eigen::VectorXd>(vector));

    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    Point point2(std::make_unique<Eigen::VectorXd>(v2));

    EXPECT_FLOAT_EQ(point.computeDistance(point2, Distance::Cosine),42/sqrt(30*70));
}


TEST_F(PointTest, CorrelationDistanceWorksCorrectly) {
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Point point(std::make_unique<Eigen::VectorXd>(vector));

    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    Point point2(std::make_unique<Eigen::VectorXd>(v2));

    EXPECT_FLOAT_EQ(point.computeDistance(point2, Distance::Correlation),7.0/sqrt(105));
}


TEST_F(PointTest, convertingEmptyArrayRaisesException){
    double array[0] = {};
    EXPECT_ANY_THROW(Point::convertArrayToPoint(array, 0));
}