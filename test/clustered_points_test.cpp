//
// Created by guibertf on 9/21/21.
//
#include "ClusteredPoints.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include <fstream>
#include "utils.h"
#include <chrono>
#include <immintrin.h>


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

    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
    p2(std::make_unique<Eigen::VectorXd>(v2)), p3(std::make_unique<Eigen::VectorXd>(v3));


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
    Point p1(std::make_unique<Eigen::VectorXd>(v1));


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
    Point p1(std::make_unique<Eigen::VectorXd>(v1));


    ClusteredPoints clusteredPoints(10,2);
    clusteredPoints.insertPoint(p0);
    clusteredPoints.insertPoint(&p1);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),0);
    EXPECT_EQ(clusteredPoints.buckets.at(1)->size(), 2);

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


    ClusteredPoints clusteredPoints(10,2);
    clusteredPoints.insertPoint(p0);
    clusteredPoints.insertPoint(&p1);
    clusteredPoints.insertPoint(&p2);
    clusteredPoints.insertPoint(&p3);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),0);
    EXPECT_EQ(clusteredPoints.buckets.at(1)->size(), 0);
    EXPECT_EQ(clusteredPoints.buckets.at(2)->size(), 2);
}

TEST_F(ClusteredPointsTest, insertingNullPtrRaisesException){
    ClusteredPoints clusteredPoints(10,2);
    EXPECT_ANY_THROW(clusteredPoints.insertPoint(nullptr));
}

TEST_F(ClusteredPointsTest, insertingEmptyArrayRaisesException){
    ClusteredPoints clusteredPoints(10, 2);
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


    ClusteredPoints clusteredPoints(10,4);
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

    ClusteredPoints clusteredPoints(10,2);
    clusteredPoints.insertPoint(p0);
    clusteredPoints.insertPoint(p1);
    clusteredPoints.insertPoint(p2);
    clusteredPoints.insertPoint(p3);
    EXPECT_EQ(clusteredPoints.buckets.at(0)->size(),0);
    EXPECT_EQ(clusteredPoints.buckets.at(1)->size(), 0);
    EXPECT_EQ(clusteredPoints.buckets.at(2)->size(), 2);
}


TEST_F(ClusteredPointsTest, pointsFromFile){
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

    pointFile.close();

    for(int i=0; i < vectors.size(); ++i) {
        inputPoints.push_back(new Point(std::make_unique<Eigen::VectorXd>(vectors.at(i)), false));
    }

    int m = 3;
    int l = ceil(log2(inputPoints.size()*1.0/m)+2);
    ClusteredPoints clusteredPoints(l,m);
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

__attribute__((always_inline)) __m256d computeSquaredDiff(__m256d &reg, const double * const d1, const double * const d2){
    const __m256d a = _mm256_loadu_pd(d1); // Latency = 7 cycles
    const __m256d b = _mm256_loadu_pd(d2);// Latency = 7 cycles
    const __m256d diff =  _mm256_sub_pd(a, b); // (a-b)  Latency = 4 cycles
    return _mm256_fmadd_pd(diff, diff , reg);
}


double hsum_double_avx(__m256d v){
    // [x1 x2 x3 x4]
    // [ x3 x4 ]
    __m128d low = _mm256_castpd256_pd128(v);
    // [ x1 x2 ]
    __m128d high = _mm256_extractf128_pd(v, 1);
    // [x1 + x3  x2 + x4]
    low = _mm_add_pd(low, high);
    // high&4 = [x1 + x3]
    __m128d high64 = _mm_unpackhi_pd(low, low);
    // result = x1 + x3 + x2 + x4
    return _mm_cvtsd_f64(_mm_add_sd(low, high64));
}

double computeSum(const double *d1, const double *d2, int size, int nAccumulators){
    __m256d acc = _mm256_setzero_pd();
    __m256d acc2 = _mm256_setzero_pd();
    __m256d acc3 = _mm256_setzero_pd();
    __m256d acc4 =  _mm256_setzero_pd();
    __m256d acc5, acc6, acc7, acc8, acc9, acc10, acc11, acc12, acc13, acc14, acc15, acc16;

    __m256d diff;
    const double * const d1End = d1 + size;
    for(; d1 < d1End; d1 += 64, d2 += 64){
        acc = computeSquaredDiff(acc, d1, d2);
        acc2 = computeSquaredDiff(acc2, (d1+4), (d2+4));
        acc3 = computeSquaredDiff(acc3, (d1+8), (d2+8));
        acc4 = computeSquaredDiff(acc4, (d1+12), (d2+12));
        acc5 = computeSquaredDiff(acc4, (d1+16), (d2+16));
        acc6 = computeSquaredDiff(acc4, (d1+20), (d2+20));
        acc7 = computeSquaredDiff(acc4, (d1+24), (d2+24));
        acc8 = computeSquaredDiff(acc4, (d1+28), (d2+28));
        acc9 = computeSquaredDiff(acc4, (d1+32), (d2+32));
        acc10 = computeSquaredDiff(acc4, (d1+36), (d2+36));
        acc11 = computeSquaredDiff(acc4, (d1+40), (d2+40));
        acc12 = computeSquaredDiff(acc4, (d1+44), (d2+44));
        acc13 = computeSquaredDiff(acc4, (d1+48), (d2+48));
        acc14 = computeSquaredDiff(acc4, (d1+52), (d2+52));
        acc15 = computeSquaredDiff(acc4, (d1+56), (d2+56));
        acc16 = computeSquaredDiff(acc4, (d1+60), (d2+60));
    }

    // 1+ 2
    acc2 = _mm256_add_pd(acc, acc2);
    // 3+ 4
    acc4 = _mm256_add_pd(acc3, acc4);
    // 5 + 6
    acc6= _mm256_add_pd(acc6, acc5);
    // 7 + 8
    acc8 = _mm256_add_pd(acc8, acc7);

    acc10 = _mm256_add_pd(acc9, acc10);

    acc12 = _mm256_add_pd(acc11, acc12);

    acc14 = _mm256_add_pd(acc13, acc14);

    acc16 = _mm256_add_pd(acc15, acc16);

    // 13 + 14 + 15 + 16
    acc16 = _mm256_add_pd(acc16, acc14);

    // 9 + 10 + 11 + 12
    acc12 = _mm256_add_pd(acc12, acc10);

    // 5 + 6 + 7 + 8
    acc8 = _mm256_add_pd(acc8, acc6);

    // 1 + 2 + 3 + 4
    acc4 = _mm256_add_pd(acc2, acc4);

    // 1 + 2 + 3 + 4 + 5 + 6 + 7 +8
    acc8 = _mm256_add_pd(acc4, acc8);

    // 9 + 10 + 11 + 12 + 14 + 15 + 16
    acc12 = _mm256_add_pd(acc12, acc16);

    acc = _mm256_add_pd(acc8, acc12);
    return hsum_double_avx(acc);
}


TEST_F(ClusteredPointsTest, someRandomTest){
    Eigen::VectorXd r1 = Eigen::VectorXd::Random(902629);
    Eigen::VectorXd r2 = Eigen::VectorXd::Random(902629);
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    auto t1 = high_resolution_clock::now();
    double dur1= (r1-r2).norm();
    auto t2 = high_resolution_clock::now();
    duration<double, std::milli> t2t1 = t2 - t1;
    std::cout << "Took " << t2t1.count() << " ms" << std::endl;
    double *d1 = r1.data();
    double *d2 = r2.data();

    auto t3 = high_resolution_clock::now();
    double res = computeSum(d1, d2, r1.size(),4);
    auto t4 = high_resolution_clock::now();
    duration<double, std::milli> t4t3 = t4 - t3;
    std::cout << "Took " << t4t3.count() << " ms" << std::endl;

    std::cout << dur1 << std::endl;
    std::cout << std::sqrt(res) << std::endl;

}


TEST_F(ClusteredPointsTest, runningOnReducedVersionIsNotTooFarFromActualClusters){
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
        inputPoints.push_back(new Point(std::make_unique<Eigen::VectorXd>(vectors.at(i))));
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
    Point p0(std::make_unique<Eigen::VectorXd>(v0)), p1(std::make_unique<Eigen::VectorXd>(v1)),
    p2(std::make_unique<Eigen::VectorXd>(v2));
    startCentroids.push_back(v0);
    startCentroids.push_back(v1);
    startCentroids.push_back(v2);

    Threeple *t = kmeans::kMeans(inputPoints, &startCentroids, 3, 100);

    int m = 3;
    int l = ceil(log2(inputPoints.size()*1.0/m)+2);
    ClusteredPoints clusteredPoints(l,m);
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


