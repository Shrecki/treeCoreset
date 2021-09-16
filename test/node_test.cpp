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

TEST_F(NodeTest, addPointCorrectlyUpdatesCost){

}
