//
// Created by guibertf on 9/15/21.
//
#include "clustering/Point.h"
#include <gtest/gtest.h>
#include <cmath>
#include <chrono>

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
    std::cout << *point2.getDataRef() << std::endl;

    EXPECT_FLOAT_EQ(point.computeDistance(point2, Distance::Cosine),1. - 42/sqrt(30*70));
}


TEST_F(PointTest, CorrelationDistanceWorksCorrectly) {
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Point point(std::make_unique<Eigen::VectorXd>(vector));

    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    // 10/4
    // 14/4
    // (1-10/4) + (2-10/4)(2-14/4) + (3-10/4)(7-14/4) + (4-10/4)(4-14/4)
    // -9/4 + -2/4*-6/4 + 2/4*14/4 + 6/4*6/4
    // 12 + 28

    Point point2(std::make_unique<Eigen::VectorXd>(v2));

    EXPECT_FLOAT_EQ(point.computeDistance(point2, Distance::Correlation),1. - 7.0/sqrt(105));
}


TEST_F(PointTest, convertingEmptyArrayRaisesException){
    double array[0] = {};
    EXPECT_ANY_THROW(Point::convertArrayToPoint(array, 0));
}

TEST_F(PointTest, benchmarkingNorm){
    Eigen::VectorXd v = Eigen::VectorXd::Zero(800000);
    Eigen::VectorXd v2 = Eigen::VectorXd::Zero(800000);
    for(int i =0; i < 800000; ++i){
        v(i) = i;
        v2(i) = i+7;
    }
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    auto t1 = high_resolution_clock::now();
    Point::computeEuclideanDistance(v, v2);
    auto t2 = high_resolution_clock::now();

    auto t3 = high_resolution_clock::now();
    (v-v2).norm();
    auto t4 = high_resolution_clock::now();

    duration<double, std::milli> ms_double = t2 - t1;
    duration<double, std::milli> ms_double_2 = t4 - t3;
    std::cout << ms_double.count() << "ms" << std::endl;
    std::cout << ms_double_2.count() << "ms" << std::endl;
}