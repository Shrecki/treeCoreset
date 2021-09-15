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


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}