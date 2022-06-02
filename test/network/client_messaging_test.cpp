//
// Created by guibertf on 5/30/22.
//
#include "network/ClientMessaging.h"

#include "network/ServerMessaging.h"
#include "clustering/ClusteredPoints.h"
#include "network/Requests.h"
#include "../utils.h"


#include <fstream>
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "network/MessagingUtils.h"
#include <thread>

using ::testing::Return;

class ClientMessagingTest : public ::testing::Test {
protected:
    zmq::socket_t* server_socket;
    zmq::socket_t* client_socket;
    zmq::context_t* context;
    std::thread *t1;

    ClientMessagingTest(){
        context = new zmq::context_t(2);
        server_socket = new zmq::socket_t(*context, ZMQ_PAIR);
        client_socket = new zmq::socket_t(*context, ZMQ_PAIR);

    }

    ~ClientMessagingTest() override {
        delete context;
        delete server_socket;
        delete client_socket;
    }

    void SetUp() override {
        //zmq::context_t ct(2);
        //zmq::socket_t server_socket(context, ZMQ_REP);
        server_socket->bind("ipc:///tmp/test");
        client_socket->connect("ipc:///tmp/test");

        //zmq::socket_t client_socket(context, ZMQ_REQ);
        t1 = new std::thread(ServerMessaging::runServer, std::ref(*server_socket), 1000, 10);
    }

    void TearDown() override {
        t1->join();

        server_socket->close();
        client_socket->close();
        context->close();

        delete t1;
    }
};

TEST_F(ClientMessagingTest, requestingStopCorrectlyStopsServerAndReturnsExpectedMessage) {
    ClientMessaging::requestStop(*client_socket);

    // We expect now to receive a stop OK
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    double *array = extractDoubleArrayFromContent(resp);
    EXPECT_EQ((int)array[0], Requests::STOP_OK);
    delete [] array;
}

TEST_F(ClientMessagingTest, requestingToPutPointsIsReturningOKResponse){
    // We will request to put in a simple point
    double point[3] = {0.1, 0.2, 0.4};
    EXPECT_NO_THROW(ClientMessaging::requestPutPoint(*client_socket, point, 3, 1000));

    ClientMessaging::requestStop(*client_socket);
}

TEST_F(ClientMessagingTest, requestingCentroidsOnAShutDownServerTimesOut){
    ClientMessaging::requestStop(*client_socket);
    zmq::message_t resp;
    client_socket->recv(&resp,0);
    try{
        ClientMessaging::requestCentroids(*client_socket,2, 100);
    } catch(std::exception &e) {
        EXPECT_STREQ(e.what(), "Server seems to be offline, abandoning.");
    }
}

TEST_F(ClientMessagingTest, requestingRepsOnAShutDownServerTimesOut){
    ClientMessaging::requestStop(*client_socket);
    zmq::message_t resp;
    client_socket->recv(&resp,0);
    try{
        ClientMessaging::requestRepresentatives(*client_socket,100);
    } catch(std::exception &e) {
        EXPECT_STREQ(e.what(), "Server seems to be offline, abandoning.");
    }
}

TEST_F(ClientMessagingTest, requestingToPutPointOnAShutDownServerTimesOut){
    ClientMessaging::requestStop(*client_socket);
    zmq::message_t resp;
    client_socket->recv(&resp,0);

    double point[3] = {0.1, 0.2, 3};

    try{
        ClientMessaging::requestPutPoint(*client_socket, point, 3, 100);
    } catch(std::exception &e) {
        EXPECT_STREQ(e.what(), "Server seems to be offline, abandoning.");
    }
}


TEST_F(ClientMessagingTest, requestingToPutPointWithNanRaisesException){
    // We will request to put in a simple point
    double point[3] = {0.1, 0.2, std::nan("")};
    EXPECT_THROW(ClientMessaging::requestPutPoint(*client_socket, point, 3, 1000), std::runtime_error);

    try{
        ClientMessaging::requestPutPoint(*client_socket, point, 3, 1000);
    } catch(std::exception &e){
        EXPECT_STREQ(e.what(), "Server says: Array contains 1 NaN values. Please fix them.");
    }
    ClientMessaging::requestStop(*client_socket);
}


TEST_F(ClientMessagingTest, postingPointsAndGettingRepresentativesReturnsExactlyPostedPointsWithTwoPointsAndTenRepresentatives){
// We will request to put in a simple point
    double points[2][3] = {{0.1, 0.2, 0.4}, {0.3, 0.1, 0.7}};
    EXPECT_NO_THROW(ClientMessaging::requestPutPoint(*client_socket, points[0], 3, 1000));
    EXPECT_NO_THROW(ClientMessaging::requestPutPoint(*client_socket, points[1], 3, 1000));


    auto reps = ClientMessaging::requestRepresentatives(*client_socket, 1000);
    EXPECT_EQ(reps->size(), 2);
    EXPECT_EQ(reps->at(0)->size(), 3);
    for(int i=0; i < 2; ++i){
        for(int j=0; j < 3; j++){
            EXPECT_DOUBLE_EQ(reps->at(i)->at(j), points[i][j]);
        }
    }

    ClientMessaging::free_vector(reps);

    ClientMessaging::requestStop(*client_socket);
}

TEST_F(ClientMessagingTest, postingAPointOf0DimensionShouldReturnAnErrorMessage){
    double points[2][3] = {{0.1, 0.2, 0.4}, {0.3, 0.1, 0.7}};
    EXPECT_THROW(ClientMessaging::requestPutPoint(*client_socket, points[0], 0, 1000), std::invalid_argument);

    try{
        ClientMessaging::requestPutPoint(*client_socket, points[0], 0, 1000);
    } catch(std::exception &e){
        EXPECT_STREQ(e.what(), "Input dimension cannot be 0. Please pass a point with at least dimension 1.");
    }

    ClientMessaging::requestStop(*client_socket);
}

TEST_F(ClientMessagingTest, postingMorePointsThanRepresentativesShouldNotTriggerException){
    std::ifstream pointFile("/home/guibertf/CLionProjects/treeCoreset/test/clustering/exampledata.csv");

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

    std::cout << vectors.size() << std::endl;

    pointFile.close();

    // Now push back the points
    zmq::message_t request(3*sizeof(double));
    for(int i=0; i < vectors.size(); ++i){
        //std::cout << "Point " << std::to_string(i);
        double data[2] = {vectors.at(i)(0), vectors.at(i)(1)};
        EXPECT_NO_THROW(ClientMessaging::requestPutPoint(*client_socket, data, 2, 1000));
    }
    ClientMessaging::requestStop(*client_socket);
}

TEST_F(ClientMessagingTest, zeroNumberOfCentroidsRaisesException){
    EXPECT_THROW(ClientMessaging::requestCentroids(*client_socket, 0, 1000), std::invalid_argument);
    ClientMessaging::requestStop(*client_socket);
}

TEST_F(ClientMessagingTest, tryingToGetNClustersWhenLessThanNPointsPostedShouldReturnAnErrorMessage){
    std::ifstream pointFile("/home/guibertf/CLionProjects/treeCoreset/test/clustering/exampledata.csv");

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

    std::cout << vectors.size() << std::endl;

    pointFile.close();

    // We shall push back 10 points
    for(int i=0; i < 8; ++i){
        //std::cout << "Point " << std::to_string(i);
        double data[2] = {vectors.at(i)(0), vectors.at(i)(1)};
        ClientMessaging::requestPutPoint(*client_socket, data, 2, 1000);
    }

    // Now we ask for 11 centroids
    EXPECT_THROW(ClientMessaging::requestCentroids(*client_socket, 11, 1000), std::runtime_error);
    try{
        ClientMessaging::requestCentroids(*client_socket, 11, 1000);
    }catch(std::exception &e){
        EXPECT_STREQ(e.what(), "Server says: Asking for 11 centroids with only 8 representative points available. Please add more points or consider increasing number of representatives.");
    }

    ClientMessaging::requestStop(*client_socket);
}

TEST_F(ClientMessagingTest, tryingToGetMoreClustersThanAvailableRepresentativesShouldReturnAnErrorMessage){
    std::ifstream pointFile("/home/guibertf/CLionProjects/treeCoreset/test/clustering/exampledata.csv");

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

    std::cout << vectors.size() << std::endl;

    pointFile.close();

    // We shall push back 10 points
    for(int i=0; i < vectors.size() ; ++i){
        //std::cout << "Point " << std::to_string(i);
        double data[2] = {vectors.at(i)(0), vectors.at(i)(1)};
        ClientMessaging::requestPutPoint(*client_socket, data, 2, 1000);
    }

    // Now we ask for 11 centroids
    EXPECT_THROW(ClientMessaging::requestCentroids(*client_socket, 11, 1000), std::runtime_error);
    try{
        ClientMessaging::requestCentroids(*client_socket, 11, 1000);
    }catch(std::exception &e){
        EXPECT_STREQ(e.what(), "Server says: Asking for 11 centroids with only 10 representative points available. Please add more points or consider increasing number of representatives.");
    }

    ClientMessaging::requestStop(*client_socket);
}

TEST_F(ClientMessagingTest, gettingCentroidsReturnValidCentroidsAfterPostingManyPoints){
    std::ifstream pointFile("/home/guibertf/CLionProjects/treeCoreset/test/clustering/exampledata.csv");

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

    std::cout << vectors.size() << std::endl;

    pointFile.close();

    // We shall push back 10 points
    for(int i=0; i < vectors.size() ; ++i){
        //std::cout << "Point " << std::to_string(i);
        double data[2] = {vectors.at(i)(0), vectors.at(i)(1)};
        ClientMessaging::requestPutPoint(*client_socket, data, 2, 1000);
    }

    // Now we ask for 3 centroids
    auto centroid_results = ClientMessaging::requestCentroids(*client_socket, 3, 1000);

    // Proper centroids should be within, ie they should not be NaNs
    for(auto &v: *centroid_results){
        EXPECT_EQ(v->size(), 2); // All vectors should have size 2 exactly
        for(int i=0; i < v->size(); ++i){
            // No vector should contain a NaN
            EXPECT_FALSE(std::isnan(v->at(i)));
            EXPECT_FALSE(std::isinf(v->at(i)));
        }
    }

    // Check that the vectors are pair-wise different
    for(int i=0; i < centroid_results->size()-1; ++i){
        for(int j=0; j < 2; ++j){
            EXPECT_NE(centroid_results->at(i)->at(j), centroid_results->at(i+1)->at(j));
        }
    }

    ClientMessaging::free_vector(centroid_results);
    ClientMessaging::requestStop(*client_socket);
}


TEST_F(ClientMessagingTest, stressTest){
// We will request to put in a simple point

    for(int i=0; i < 10e3; ++i){
        Eigen::VectorXd vec = Eigen::VectorXd::Random(200000);
        try{
            ClientMessaging::requestPutPoint(*client_socket, vec.data(), 200000, 1000);
        } catch(std::exception &e){
            std::cout << e.what() << std::endl,
                    std::cout << i << std::endl;
            break;
        }
    }


    auto reps = ClientMessaging::requestRepresentatives(*client_socket, 1000);
    EXPECT_EQ(reps->size(), 10);
    EXPECT_EQ(reps->at(0)->size(), 200000);

    ClientMessaging::free_vector(reps);

    ClientMessaging::requestStop(*client_socket);
}

