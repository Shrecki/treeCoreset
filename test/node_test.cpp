//
// Created by guibertf on 9/15/21.
//
#include "Node.h"
#include <random>
#include "mock_RandomGenerator.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"

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

