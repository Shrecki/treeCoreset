//
// Created by guibertf on 9/15/21.
//
#include "Node.h"
#include <gtest/gtest.h>


class NodeTest : public ::testing::Test {
protected:
    NodeTest() {

    }

    ~NodeTest() override {

    }

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(NodeTest, ConstructorSetsCapacity) {
    Node testNode(50);
    EXPECT_EQ(testNode.getPointSet()->capacity(), 50);
}

TEST_F(NodeTest, DefaultNodeIsLeaf) {
    Node testNode(50);
    EXPECT_EQ(testNode.isLeaf(), true);
}

TEST_F(NodeTest, SomePointerMagic) {
    Node* testPtr = new Node(50);
    Node *otherPtr = testPtr;
    testPtr = nullptr;

    EXPECT_TRUE(otherPtr != nullptr);
    EXPECT_EQ(testPtr, nullptr);
}

TEST_F(NodeTest, nodeSizeAtInit){
    Node * node = new Node(10);
    EXPECT_EQ(node->getSize(),0);
}

TEST_F(NodeTest, addPointCorrectlyUpdatesCostWithSingleVector){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    Point p1(&vector);
    Point p2(&v2);

    node->setRepresentative(&p1, Distance::Euclidean);
    double distance = p2.computeDistance(p1, Distance::Euclidean);
    node->addPoint(&p2, Distance::Euclidean);
    EXPECT_FLOAT_EQ(node->getCost(), distance);
}

TEST_F(NodeTest, addPointCorrectlyUpdatesSizeWithSingleVector){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    Point p1(&vector);
    Point p2(&v2);

    node->setRepresentative(&p1, Distance::Euclidean);
    double distance = p2.computeDistance(p1, Distance::Euclidean);
    node->addPoint(&p2, Distance::Euclidean);
    EXPECT_FLOAT_EQ(node->getSize(), 2);
}

TEST_F(NodeTest, addPointCorrectlyAppendsPointToPointSet){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    Point p1(&vector);
    Point p2(&v2);

    node->setRepresentative(&p1, Distance::Euclidean);
    double distance = p2.computeDistance(p1, Distance::Euclidean);
    node->addPoint(&p2, Distance::Euclidean);
    EXPECT_EQ(&p2, node->getPointSet()->at(1));
}

TEST_F(NodeTest, addPointThrowsExceptOnNullPtr){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Point p1(&vector);
    node->setRepresentative(&p1, Distance::Euclidean);
    EXPECT_ANY_THROW(node->addPoint(nullptr, Distance::Euclidean));
}


TEST_F(NodeTest, setRepresentativeShouldSetCostToZeroWithNoPointsAvailableYet){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    Point p1(&vector);
    Point p2(&vector);

    EXPECT_EQ(node->getCost(), -1);
    node->setRepresentative(&p1, Distance::Euclidean);
    EXPECT_EQ(node->getCost(), 0);
}

TEST_F(NodeTest, setRepresentativeSetsRepresentativeToCorrectPointer){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;

    Point p1(&vector);
    node->setRepresentative(&p1, Distance::Euclidean);
    EXPECT_EQ(node->getRepresentative(), &p1);
}

TEST_F(NodeTest, setReprFailsOnNullPtr){
    Node *node = new Node(10);
    EXPECT_ANY_THROW(node->setRepresentative(nullptr, Distance::Euclidean));
}

TEST_F(NodeTest, SetReprMustBeBeforeAnyAddPoint){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Point p1(&vector);
    EXPECT_ANY_THROW(node->addPoint(&p1, Distance::Euclidean));
}

TEST_F(NodeTest, reprCanOnlyBeSetOnce){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    Point p1(&vector);
    Point p2(&v2);
    node->setRepresentative(&p1, Distance::Euclidean);
    EXPECT_ANY_THROW(node->setRepresentative(&p2, Distance::Euclidean));
}

TEST_F(NodeTest, settingTwiceSameReprThrowsException){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Eigen::VectorXd v2(4);
    v2 << 1, 2, 7,  4;

    Point p1(&vector);
    node->setRepresentative(&p1, Distance::Euclidean);
    EXPECT_ANY_THROW(node->setRepresentative(&p1, Distance::Euclidean));
}

TEST_F(NodeTest, settingReprOnNonLeafThrowsException){
    Node *node = new Node(10);
    Node *child = new Node(10);
    node->setAsChild(child, false);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Point p1(&vector);
    EXPECT_ANY_THROW(node->setRepresentative(&p1, Distance::Euclidean));
}

TEST_F(NodeTest, setAsChildLeftSetsLeftChild) {
    Node *node = new Node(10);
    Node *child = new Node(10);
    node->setAsChild(child, true);
    EXPECT_EQ(node->getLeftChild(), child);
    EXPECT_EQ(node->getRightChild(), nullptr);
}

TEST_F(NodeTest, setAsChildLeftSetsParent) {
    Node *node = new Node(10);
    Node *child = new Node(10);
    node->setAsChild(child, true);
    EXPECT_EQ(node->getLeftChild()->getParent(), node);
}

TEST_F(NodeTest, setAsChildRightSetsParent) {
    Node *node = new Node(10);
    Node *child = new Node(10);
    node->setAsChild(child, false);
    EXPECT_EQ(node->getRightChild()->getParent(), node);
}

TEST_F(NodeTest, setAsChildRightSetsRightChild){
    Node *node = new Node(10);
    Node *child = new Node(10);
    node->setAsChild(child, false);
    EXPECT_EQ(node->getRightChild(), child);
    EXPECT_EQ(node->getLeftChild(), nullptr);
}

TEST_F(NodeTest, setAsChildWithNullPointer){
    Node *node = new Node(10);
    EXPECT_ANY_THROW(node->setAsChild(nullptr, true));
    EXPECT_ANY_THROW(node->setAsChild(nullptr, false));
}

TEST_F(NodeTest, nodeWithoutChildIsLeaf){
    Node *node = new Node(10);
    EXPECT_TRUE(node->isLeaf());
}

TEST_F(NodeTest, nodeWithLeftChildIsLeaf){
    Node *node = new Node(10);
    Node *child = new Node(10);
    node->setAsChild(child, true);
    EXPECT_FALSE(node->isLeaf());
}

TEST_F(NodeTest, nodeWithRightChildIsLeaf){
    Node *node = new Node(10);
    Node *child = new Node(10);
    node->setAsChild(child, false);
    EXPECT_FALSE(node->isLeaf());
}

TEST_F(NodeTest, childNodeIsLeaf){
    Node *node = new Node(10);
    Node *child = new Node(10);
    node->setAsChild(child, true);
    EXPECT_TRUE(child->isLeaf());
}