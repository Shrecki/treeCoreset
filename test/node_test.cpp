//
// Created by guibertf on 9/15/21.
//
#include "Node.h"
#include <random>
#include "mock_RandomGenerator.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "math_framework.h"
#include <cmath>
using ::testing::Return;

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
    EXPECT_FLOAT_EQ(node->getCost(), distance*distance);
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

TEST_F(NodeTest, costOfNodeIsSumOfSquaredCosts){

    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << -3,6,1;
    v1 << 1, 1, 3;
    v2 << 1,0,0;
    v3 << 4,100, 28;

    Node testNode(10);
    Point p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(new Point(&v0), Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    EXPECT_FLOAT_EQ(testNode.getCost(), pow((v0-v0).norm(),2) + pow((v1-v0).norm(),2)+pow((v2-v0).norm(),2)+pow((v3-v0).norm(),2));
}

TEST_F(NodeTest, addPointThrowsExceptOnNullPtr){
    Node *node = new Node(10);
    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Point p1(&vector);
    node->setRepresentative(&p1, Distance::Euclidean);
    EXPECT_ANY_THROW(node->addPoint(nullptr, Distance::Euclidean));
}



TEST_F(NodeTest, addPointThrowsExceptOnNonLeaf){
    Node *node = new Node(10);
    Node *child = new Node(10);
    node->setAsChild(child, false);

    Eigen::VectorXd vector(4);
    vector << 1, 2, 3,  4;
    Point p1(&vector);
    EXPECT_ANY_THROW(node->addPoint(&p1, Distance::Euclidean));
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

TEST_F(NodeTest, setParentWorks){
    Node *node = new Node(10);
    Node *p = new Node(10);
    EXPECT_EQ(node->getParent(), nullptr);
    node->setParent(p);
    EXPECT_EQ(node->getParent(), p);
}

TEST_F(NodeTest, setParentThrowsExceptionOnNullptr){
    Node *node = new Node(10);
    EXPECT_ANY_THROW(node->setParent(nullptr));

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

TEST_F(NodeTest, isInIntervalThrowsExceptionsWhenMinBiggerMax){
    EXPECT_ANY_THROW(Node::isInInterval(0, -1, -2, true));
    EXPECT_ANY_THROW(Node::isInInterval(0, -1, -2, false));
}

TEST_F(NodeTest, isInIntervalWhenBoundsEqual){
    EXPECT_FALSE(Node::isInInterval(5, 5, 5, false));
    EXPECT_TRUE(Node::isInInterval(10, 10, 10, true));
    EXPECT_FALSE(Node::isInInterval(-1, 10, 10, false));
    EXPECT_FALSE(Node::isInInterval(-1, 10, 10, true));

    EXPECT_FALSE(Node::isInInterval(-5, -5, -5, false));
    EXPECT_TRUE(Node::isInInterval(-10, -10, -10, true));
    EXPECT_FALSE(Node::isInInterval(-1, -10, -10, false));
    EXPECT_FALSE(Node::isInInterval(-1, -10, -10, true));
}

TEST_F(NodeTest, isInIntervalReturnsTrueWhenInIntervalInclusive){
    EXPECT_TRUE(Node::isInInterval(0, -1, 2, true));
    EXPECT_TRUE(Node::isInInterval(-1, -10, 2, true));
    EXPECT_TRUE(Node::isInInterval(5, 4, 10, true));
}

TEST_F(NodeTest, isInIntervalReturnsFalseWhenOutIntervalInclusive){
    EXPECT_FALSE(Node::isInInterval(-8, -1, 2, true));
    EXPECT_FALSE(Node::isInInterval(4, -1, 2, true));

    EXPECT_FALSE(Node::isInInterval(-11, -10, 2, true));
    EXPECT_FALSE(Node::isInInterval(3.5, -10, 2, true));

    EXPECT_FALSE(Node::isInInterval(-0.5, 4, 10, true));
    EXPECT_FALSE(Node::isInInterval(11, 4, 10, true));
}


TEST_F(NodeTest, isInIntervalReturnsTrueWhenOnBoundsInclusive){
    EXPECT_TRUE(Node::isInInterval(-1, -1, 2, true));
    EXPECT_TRUE(Node::isInInterval(-10, -10, 2, true));
    EXPECT_TRUE(Node::isInInterval(4, 4, 10, true));

    EXPECT_TRUE(Node::isInInterval(2, -1, 2, true));
    EXPECT_TRUE(Node::isInInterval(2, -10, 2, true));
    EXPECT_TRUE(Node::isInInterval(10, 4, 10, true));
}

TEST_F(NodeTest, isInIntervalReturnsTrueWhenInIntervalExclusive){
    EXPECT_TRUE(Node::isInInterval(0, -1, 2, false));
    EXPECT_TRUE(Node::isInInterval(-1, -10, 2, false));
    EXPECT_TRUE(Node::isInInterval(5, 4, 10, false));
}

TEST_F(NodeTest, isInIntervalReturnsFalseWhenOnBoundsExclusive){
    EXPECT_FALSE(Node::isInInterval(-1, -1, 2, false));
    EXPECT_FALSE(Node::isInInterval(-10, -10, 2, false));
    EXPECT_FALSE(Node::isInInterval(4, 4, 10, false));

    EXPECT_FALSE(Node::isInInterval(2, -1, 2, false));
    EXPECT_FALSE(Node::isInInterval(2, -10, 2, false));
    EXPECT_FALSE(Node::isInInterval(10, 4, 10, false));
}

TEST_F(NodeTest, setCostCorrectlyUpdatesCost){
    Node *node = new Node(10);

    EXPECT_FLOAT_EQ(node->getCost(), -1.0);
    node->setCost(10.2314);
    EXPECT_FLOAT_EQ(node->getCost(), 10.2314);
}

TEST_F(NodeTest, setCostThrowsExcOnNegativeVal){
    Node *node = new Node(10);
    EXPECT_ANY_THROW(node->setCost(-10));
}

TEST_F(NodeTest, selectingRandomChildOnLeaf){
    Node root(10);
    EXPECT_EQ(&root, root.getRandomChild());
}

TEST_F(NodeTest, costPropagationWorksOK){

    Node root(10);

    /*
     * We will now create a bunch of nodes, setting them as a tree. For each, we will set up a cost manually.
     */
    Node children[8];
    root.setAsChild(&children[0], true);
    root.setAsChild(&children[1], false);
    children[0].setAsChild(&children[2], true);
    children[0].setAsChild(&children[3], false);

    children[1].setAsChild(&children[4], true);
    children[1].setAsChild(&children[5], false);

    children[5].setAsChild(&children[6], true);
    children[5].setAsChild(&children[7], false);

    root.setCost(1.0);
    children[0].setCost(0.2);
    children[1].setCost(0.8);
    children[2].setCost(0.05);
    children[3].setCost(0.15);
    children[4].setCost(0.3);
    children[5].setCost(0.5);
    children[6].setCost(0.2);
    children[7].setCost(0.3);

    // Now, we will modify children 7's cost to be instead 0.6.
    // We expect to see costs as follow:
    // 5 : 0.5 -> 0.8
    // 1: 0.8 -> 1.1
    // 0: 1 -> 1.3
    // Other costs should remain unaffected
    children[7].setCost(0.6);
    children[7].propagateUpCostsParentOneSided();
    double expectedCosts[8] = {0.2, 1.1, 0.05, 0.15, 0.3, 0.8, 0.2, 0.6};
    EXPECT_FLOAT_EQ(root.getCost(), 1.3);
    for(int i=0;i<8;++i){
        EXPECT_FLOAT_EQ(children[i].getCost(), expectedCosts[i]);
    }

}


TEST_F(NodeTest, costPropagationCalledFromNeighborNodeStillYieldsCorrectValues){

    Node root(10);

    /*
     * We will now create a bunch of nodes, setting them as a tree. For each, we will set up a cost manually.
     */
    Node children[8];
    root.setAsChild(&children[0], true);
    root.setAsChild(&children[1], false);
    children[0].setAsChild(&children[2], true);
    children[0].setAsChild(&children[3], false);

    children[1].setAsChild(&children[4], true);
    children[1].setAsChild(&children[5], false);

    children[5].setAsChild(&children[6], true);
    children[5].setAsChild(&children[7], false);

    root.setCost(1.0);
    children[0].setCost(0.2);
    children[1].setCost(0.8);
    children[2].setCost(0.05);
    children[3].setCost(0.15);
    children[4].setCost(0.3);
    children[5].setCost(0.5);
    children[6].setCost(0.2);
    children[7].setCost(0.3);

    // Now, we will modify children 7's cost to be instead 0.6.
    // We expect to see costs as follow:
    // 5 : 0.5 -> 0.8
    // 1: 0.8 -> 1.1
    // 0: 1 -> 1.3
    // Other costs should remain unaffected
    children[7].setCost(0.6);
    children[6].propagateUpCostsParentOneSided();
    double expectedCosts[8] = {0.2, 1.1, 0.05, 0.15, 0.3, 0.8, 0.2, 0.6};
    EXPECT_FLOAT_EQ(root.getCost(), 1.3);
    for(int i=0;i<8;++i){
        EXPECT_FLOAT_EQ(children[i].getCost(), expectedCosts[i]);
    }

}

TEST_F(NodeTest, costPropagationOnRootNodeShouldDoNothing){
    Node root(10);
    root.setCost(1.0);
    root.propagateUpCostsParentOneSided();
    EXPECT_FLOAT_EQ(root.getCost(), 1.0);
}

TEST_F(NodeTest, costPropagationOnNodeWithNoInitializedCostThrowsException){
    Node root(10);
    EXPECT_ANY_THROW(root.propagateUpCostsParentOneSided());
}

TEST_F(NodeTest, costPropagationThrowsExceptionIfACostIsNotInitialized){
    Node root(10);

    /*
     * We will now create a bunch of nodes, setting them as a tree. For each, we will set up a cost manually.
     */
    Node children[8];
    root.setAsChild(&children[0], true);
    root.setAsChild(&children[1], false);
    children[0].setAsChild(&children[2], true);
    children[0].setAsChild(&children[3], false);

    children[1].setAsChild(&children[4], true);
    children[1].setAsChild(&children[5], false);

    children[5].setAsChild(&children[6], true);
    children[5].setAsChild(&children[7], false);

    root.setCost(1.0);
    children[0].setCost(0.2);
    children[1].setCost(0.8);
    children[2].setCost(0.05);
    children[4].setCost(0.3);
    children[5].setCost(0.5);
    children[7].setCost(0.3);

    // Now, we will modify children 7's cost to be instead 0.6.
    // We expect to see costs as follow:
    // 5 : 0.5 -> 0.8
    // 1: 0.8 -> 1.1
    // 0: 1 -> 1.3
    // Other costs should remain unaffected
    children[7].setCost(0.6);
    EXPECT_ANY_THROW(children[7].propagateUpCostsParentOneSided());
    EXPECT_ANY_THROW(children[2].propagateUpCostsParentOneSided());
}

TEST_F(NodeTest, updatingNodeCostBasedOnChildrenWorksWell){

    Node root(10);

    /*
     * We will now create a bunch of nodes, setting them as a tree. For each, we will set up a cost manually.
     */
    Node children[2];
    root.setAsChild(&children[0], true);
    root.setAsChild(&children[1], false);
    children[0].setCost(0.2);
    children[1].setCost(0.8);


    EXPECT_FLOAT_EQ(root.getCost(), -1.0);
    root.updateCostBasedOnChildren();
    EXPECT_FLOAT_EQ(root.getCost(), 1.0);
}


TEST_F(NodeTest, updatingCostWithLeftChildCostNotInitThrowsException){

    Node root(10);

    /*
     * We will now create a bunch of nodes, setting them as a tree. For each, we will set up a cost manually.
     */
    Node children[2];
    root.setAsChild(&children[0], true);
    root.setAsChild(&children[1], false);
    children[1].setCost(0.8);
    EXPECT_ANY_THROW(root.updateCostBasedOnChildren());
}


TEST_F(NodeTest, updatingCostWithRightChildCostNotInitThrowsException){

    Node root(10);

    /*
     * We will now create a bunch of nodes, setting them as a tree. For each, we will set up a cost manually.
     */
    Node children[2];
    root.setAsChild(&children[0], true);
    root.setAsChild(&children[1], false);
    children[0].setCost(0.8);
    EXPECT_ANY_THROW(root.updateCostBasedOnChildren());
}

TEST_F(NodeTest, updatingNodeCostThrowsExceptionIfLeftChildIsNullPtr){
    Node root(10);
    /*
     * We will now create a bunch of nodes, setting them as a tree. For each, we will set up a cost manually.
     */
    Node children[1];
    root.setAsChild(&children[0], true);
    children[0].setCost(0.2);
    EXPECT_ANY_THROW(root.updateCostBasedOnChildren());
}


TEST_F(NodeTest, updatingNodeCostThrowsExceptionIfRightChildIsNullPtr){
    Node root(10);
    /*
     * We will now create a bunch of nodes, setting them as a tree. For each, we will set up a cost manually.
     */
    Node children[1];
    root.setAsChild(&children[0], false);
    children[0].setCost(0.2);
    EXPECT_ANY_THROW(root.updateCostBasedOnChildren());
}

TEST_F(NodeTest, selectingRandomChildWorksAsIntended){
    /*
     * This will test that, provided a random choice which we know a priori thanks to mocking, we can indeed get to the expected child node.
     * Recall this choice is based upon the cost of a node. While we could setup our node with various points, we will instead setup their costs manually.
     * This way, we only test that the logic of the function is correct.
     */

    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());

    mock_RandomGenerator dist_mock(0.0, 1.0);
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.5));
    EXPECT_FLOAT_EQ(dist_mock.getRandom(), 0.5);

    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.7));
    EXPECT_FLOAT_EQ(dist_mock.getRandom(), 0.7);

    Node root(10);
    root.setRng(&dist_mock);

    /*
     * We will now create a bunch of nodes, setting them as a tree. For each, we will set up a cost manually.
     */
    Node children[8];
    for(int i=0; i< 8; i++){
        children[i].setRng(&dist_mock);
    }
    root.setAsChild(&children[0], true);
    root.setAsChild(&children[1], false);
    children[0].setAsChild(&children[2], true);
    children[0].setAsChild(&children[3], false);

    children[1].setAsChild(&children[4], true);
    children[1].setAsChild(&children[5], false);

    children[5].setAsChild(&children[6], true);
    children[5].setAsChild(&children[7], false);

    root.setCost(1.0);
    children[0].setCost(0.2);
    children[1].setCost(0.8);
    children[2].setCost(0.05);
    children[3].setCost(0.15);
    children[4].setCost(0.3);
    children[5].setCost(0.5);
    children[6].setCost(0.2);
    children[7].setCost(0.3);
    /*
     * Now that we have our tree with costs, we will assess whether we indeed walk down all the way to the node we expect.
     * Let us walk down node 7. We know we select right node only if our random sample is above cost(left)/cost(parent).
     * Let's compute what we need:
     *      First level, we need sample above 0.2/1.0 = 0.2
     *      Second level, we need it above 0.3/0.8 = 0.375
     *      Third level we need it above 0.2/0.5 = 0.4
     */
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.45));
    Node* randChild = root.getRandomChild();
    EXPECT_EQ(randChild, &children[7]);

    /*
     * To prove it further, now consider we want node 6. We will obtain it by selecting a probability anywhere between 0.375 and 0.4
     */
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.38));
    randChild = root.getRandomChild();
    EXPECT_EQ(randChild, &children[6]);
    /*
     * Node 4 is with p in (0.2, 0.375]
     */
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.24));
    randChild = root.getRandomChild();
    EXPECT_EQ(randChild, &children[4]);

    /*
     * For node 2 and 3, we have an interesting case:
     * We need a probability above 0.25 to select 3. If we pass a new random generator to node 0, with a biased
     * probability that is different from root, we can recover again the nodes we want.
     */
    mock_RandomGenerator dist_mock2(0.0, 1.0);
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.15)); // Selects left node

    ON_CALL(dist_mock2, CallOp).WillByDefault(Return(0.01));
    children[0].setRng(&dist_mock2);
    randChild = root.getRandomChild();
    EXPECT_EQ(randChild, &children[2]);

    ON_CALL(dist_mock2, CallOp).WillByDefault(Return(0.26));
    randChild = root.getRandomChild();
    EXPECT_EQ(randChild, &children[3]);
}

TEST_F(NodeTest, selectClusterThrowsErrorIfNoPointAvailable){
    Node testNode(10);
    EXPECT_ANY_THROW(testNode.selectNewClusterRep(Distance::Euclidean));
}

TEST_F(NodeTest, selectClusterRepHasUniformDistribWithEvenlySpacedPoints){
    /*
     * The probability to select a point as representative is basically nothing but the distance to the current point.
     * The higher the distance, the better.
     * The intervals are ordered in order of points, meaning if we know the distance of the point and its order of insertion,
     * we can deduce probability of it being chosen (as in: the exact interval).
     * However, since we're interested in probabilities, we will instead adopt this time another approach.
     * Namely, we will check that the amount of times a given point is selected as
     */

    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 1, 0;
    v2 << 1,0,0;
    v3 << 0,0, 1;

    Node testNode(10);
    Point p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(new Point(&v0), Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    int count[3] = {0,0,0};
    int n_samples = 3.0*10e4;
    int decimal_places = 2;
    double factor = pow(10, decimal_places);
    for(int i=0; i < n_samples; ++i){
        Point * rep = testNode.selectNewClusterRep(Distance::Euclidean);
        if(rep == &p1){ count[0]++;}
        if(rep == &p2){ count[1]++;}
        if(rep == &p3){ count[2]++;}
    }

    int expectedFreq[3] = {n_samples/3, n_samples/3, n_samples/3};
    EXPECT_TRUE(statistics::chiSquareTest(count, expectedFreq, 3, 3-1) > 0.05);
}



TEST_F(NodeTest, selectClusterConformsToMoreComplexDistributions){
    /*
     * The probability to select a point as representative is basically nothing but the distance to the current point.
     * The higher the distance, the better.
     * The intervals are ordered in order of points, meaning if we know the distance of the point and its order of insertion,
     * we can deduce probability of it being chosen (as in: the exact interval).
     * However, since we're interested in probabilities, we will instead adopt this time another approach.
     * Namely, we will check that the amount of times a given point is selected as
     */

    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10

    // As a consequence, we know that total cost is 25+16+100 = 141.
    // Therefore we know the expected probabilities, as 25/141=0.1773, 16/141=0.113475177, 100/141=0.709219858

    Node testNode(10);
    Point p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(new Point(&v0), Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    int count[3] = {0,0,0};
    int n_samples = 3.0*10e4;
    int decimal_places = 2;
    double factor = pow(10, decimal_places);
    for(int i=0; i < n_samples; ++i){
        Point * rep = testNode.selectNewClusterRep(Distance::Euclidean);
        if(rep == &p1){ count[0]++;}
        if(rep == &p2){ count[1]++;}
        if(rep == &p3){ count[2]++;}
    }

    int f1 = ceil(n_samples*25.0/141.0);
    int f2 = ceil(n_samples*16.0/141.0);
    int f3 = n_samples - f1 - f2;
    int expectedFreq[3] = {f1, f2, f3};
    double pval = statistics::chiSquareTest(count, expectedFreq, 3, 3-1);
    EXPECT_TRUE( pval > 0.05);
}

TEST_F(NodeTest, splittingNodeCreatesChildren){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    Point* newRep = testNode.splitNode(Distance::Euclidean);
    EXPECT_TRUE(testNode.getLeftChild() != nullptr);
    EXPECT_TRUE(testNode.getRightChild() != nullptr);
}


TEST_F(NodeTest, splittingNodeRemovesRepresentativeOfParentNode){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10

    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);
    Point* newRep = testNode.splitNode(Distance::Euclidean);
    EXPECT_EQ(testNode.getRepresentative(), nullptr);
}


TEST_F(NodeTest, splittingNodeEmptiesPointSetOfNode){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    EXPECT_TRUE(testNode.isLeaf());
    EXPECT_EQ(testNode.getPointSet()->size(), 4);
    Point* newRep = testNode.splitNode(Distance::Euclidean);
    EXPECT_EQ(testNode.getPointSet()->size(),0);
}


TEST_F(NodeTest, splittingNodeSetsRepresentativeDependingOnSelectedPoint){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    // We know that total cost is 25+16+100 = 141.
    // Therefore we know the expected probabilities, as 25/141=0.1773, 16/141=0.113475177, 100/141=0.709219858
    // Hence, intervals would be [0, 25/141], [25/141, (16+25)/141], [(16+25)/141, 1]

    // Setup our random number mock, to select exactly the probability we want
    mock_RandomGenerator dist_mock(0.0, 1.0);

    // We select p1
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.01));
    testNode.setRng(&dist_mock);

    Point* newRep = testNode.splitNode(Distance::Euclidean);

    EXPECT_EQ(testNode.getLeftChild()->getRepresentative(), &p1);
    EXPECT_EQ(testNode.getRightChild()->getRepresentative(), &p0);

    // We select p2
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.2));
    testNode = Node(10);
    testNode.setRng(&dist_mock);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    newRep = testNode.splitNode(Distance::Euclidean);

    EXPECT_EQ(testNode.getLeftChild()->getRepresentative(), &p2);
    EXPECT_EQ(testNode.getRightChild()->getRepresentative(), &p0);

    // We select p3
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.4));
    testNode = Node(10);
    testNode.setRng(&dist_mock);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    newRep = testNode.splitNode(Distance::Euclidean);

    EXPECT_EQ(testNode.getLeftChild()->getRepresentative(), &p3);
    EXPECT_EQ(testNode.getRightChild()->getRepresentative(), &p0);
}

TEST_F(NodeTest, splitNodeReturnsLeftChildRepresentative){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    Point* newRep = testNode.splitNode(Distance::Euclidean);

    EXPECT_EQ(newRep,testNode.getLeftChild()->getRepresentative());
    EXPECT_TRUE(newRep != testNode.getRightChild()->getRepresentative());
}

TEST_F(NodeTest, splitNodeKeepsParentSizeUnchanged){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    Point* newRep = testNode.splitNode(Distance::Euclidean);
    EXPECT_EQ(testNode.getSize(), 4);
}

TEST_F(NodeTest, splitNodeSetsChildrenSizesCorrectly){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    mock_RandomGenerator dist_mock(0.0, 1.0);

    // We select p1
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.01));
    testNode.setRng(&dist_mock);

    Point* newRep = testNode.splitNode(Distance::Euclidean);
    EXPECT_EQ(testNode.getLeftChild()->getPointSet()->size(), 1);
    EXPECT_EQ(testNode.getRightChild()->getPointSet()->size(), 3);
}


TEST_F(NodeTest, splitNodeSetChildrenCostCorrectly){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    mock_RandomGenerator dist_mock(0.0, 1.0);

    // We select p1
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.01));
    testNode.setRng(&dist_mock);

    Point* newRep = testNode.splitNode(Distance::Euclidean);
    EXPECT_EQ(testNode.getLeftChild()->getCost(), 0);
    EXPECT_EQ(testNode.getRightChild()->getCost(), 116);
}

TEST_F(NodeTest, splitNodeCalledTwiceSplitsPointsAsExpectedAndImprovingCosts){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << -4,2,0;
    v1 << 0, 5, 0; // Distance squared to v0 would be 25
    v2 << 4,0,0; // Distance squared to v0 would be 68
    v3 << 0,0, 10; // Distance squared to v0 would be 120


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    mock_RandomGenerator dist_mock(0.0, 1.0);

    // We select p1
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.01));
    testNode.setRng(&dist_mock);

    Point* newRep = testNode.splitNode(Distance::Euclidean);

    // Now, we will split again on left child
    // Because of how everything is setup distance-wise, p1 should be clustered with p2 and p3 should be clustered with
    // p0. Therefore, splitting on left child will create a node with new representative p2 and another node with only
    // p1 as node (representative)
    Node *left = testNode.getLeftChild();
    Point* secondRep = left->splitNode(Distance::Euclidean);

    EXPECT_EQ(secondRep, &p2);
    EXPECT_EQ(left->getLeftChild()->getRepresentative(), secondRep);
    EXPECT_EQ(left->getRightChild()->getRepresentative(), &p1);
    EXPECT_EQ(left->getRightChild()->getPointSet()->size(), 1);
    EXPECT_EQ(testNode.getRightChild()->getPointSet()->size(), 2);

}

TEST_F(NodeTest, splitNodeUpdatesParentCostCorrectlyWithZeroOrigin){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    mock_RandomGenerator dist_mock(0.0, 1.0);

    // We select p1
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.01));
    testNode.setRng(&dist_mock);


    Point* newRep = testNode.splitNode(Distance::Euclidean);
    EXPECT_EQ(testNode.getCost(), 116);
}

TEST_F(NodeTest, splitNodeThrowsExceptionIfCalledOnNodeWithASinglePoint){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);

    EXPECT_ANY_THROW(testNode.splitNode(Distance::Euclidean));
}

TEST_F(NodeTest, splitNodeSetsUpNodeAsParentOfChildren){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    Point* newRep = testNode.splitNode(Distance::Euclidean);

    EXPECT_EQ(&testNode, testNode.getLeftChild()->getParent());
    EXPECT_EQ(&testNode, testNode.getRightChild()->getParent());
}

TEST_F(NodeTest, splitNodeOnNonLeafNodeThrowsException){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10), childNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    testNode.setAsChild(&childNode, true);
    EXPECT_ANY_THROW(testNode.splitNode(Distance::Euclidean));
}

TEST_F(NodeTest, splitNodeClustersPointsInChildrenAsItShould){
    Eigen::VectorXd v0(3), v1(3), v2(3), v3(3), v4(3), v5(3), v6(3);
    v0 << 0,0,0;
    v1 << 0, 5, 0; // Norm would be 5
    v2 << 4,0,0; // Norm would be 4
    v3 << 0,0, 10; // Norm would be 10


    Node testNode(10);
    Point p0(&v0), p1(&v1), p2(&v2), p3(&v3);
    testNode.setRepresentative(&p0, Distance::Euclidean);
    testNode.addPoint(&p1, Distance::Euclidean);
    testNode.addPoint(&p2, Distance::Euclidean);
    testNode.addPoint(&p3, Distance::Euclidean);

    mock_RandomGenerator dist_mock(0.0, 1.0);

    // We select p1
    ON_CALL(dist_mock, CallOp).WillByDefault(Return(0.01));
    testNode.setRng(&dist_mock);

    Point* newRep = testNode.splitNode(Distance::Euclidean);

    EXPECT_EQ(testNode.getLeftChild()->getPointSet()->at(0), &p1);

    EXPECT_EQ(testNode.getRightChild()->getPointSet()->at(0), &p0);
    EXPECT_EQ(testNode.getRightChild()->getPointSet()->at(1), &p2);
    EXPECT_EQ(testNode.getRightChild()->getPointSet()->at(2), &p3);
}


