//
// Created by guibertf on 5/30/22.
//
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include <fstream>
#include <thread>
#include "../utils.h"

#include "network/ServerMessaging.h"
#include "network/Requests.h"


using ::testing::Return;

class ServerMessagingTest : public ::testing::Test {
protected:
    zmq::socket_t* server_socket;
    zmq::socket_t* client_socket;
    zmq::context_t* context;
    std::thread *t1;

    ServerMessagingTest(){
        context = new zmq::context_t(2);
        server_socket = new zmq::socket_t(*context, ZMQ_REP);
        client_socket = new zmq::socket_t(*context, ZMQ_REQ);

    }

    ~ServerMessagingTest() override {
        delete context;
        delete server_socket;
        delete client_socket;
    }

    void SetUp() override {
        //zmq::context_t ct(2);
        //zmq::socket_t server_socket(context, ZMQ_REP);
        server_socket->bind("tcp://*:5555");
        client_socket->connect("tcp://localhost:5555");

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

TEST_F(ServerMessagingTest, stoppingServerWorks) {
    //std::thread t1(ServerMessaging::runServer, std::ref(*server_socket), 1000, 10);

    double stop = Requests::STOP_REQ;
    zmq::message_t request(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    // We expect to receive a STOP_OK response
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    double *array = ServerMessaging::extractDoubleArrayFromContent(resp);
    EXPECT_EQ((int)array[0], Requests::STOP_OK);
    //t1.join();
}

TEST_F(ServerMessagingTest, sendingAnUnknownRequestReturnsAnErrorOnClientSide){

    //std::thread t1(ServerMessaging::runServer, std::ref(*server_socket), 1000, 10);

    double stop = 121; // This request does not exist
    zmq::message_t request(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    // We expect to receive a STOP_OK response
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    std::string array = resp.to_string();

    // Now we stop the server
    stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    //t1.join();

    std::string expected_string("Request not implemented.");
    EXPECT_STREQ(array.c_str(), expected_string.c_str());
}

TEST_F(ServerMessagingTest, getCentroidRequestWithoutHavingPutAnyPointsInReturnsAnErrorMessageOnClientSide){
    //std::thread t1(ServerMessaging::runServer, std::ref(*server_socket), 1000, 10);

    //double stop = Requests::GET_CENTROIDS; // This request does not exist
    double centroid_req[2] = {Requests::GET_CENTROIDS, 2};
    zmq::message_t request(2*sizeof(double));
    memcpy((void *) request.data(), (void*)(&centroid_req), 2 * sizeof(double));
    client_socket->send(request);

    // We should receive here an error message
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    std::string array = resp.to_string();

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    //t1.join();

    std::string expected_string("Asking for 2 centroids with only 0 representative points available. Please add more points or consider increasing number of representatives.");
    EXPECT_STREQ(array.c_str(), expected_string.c_str());
}

TEST_F(ServerMessagingTest, getCentroidRequestWithMalformedRequestThrowsErrorOnClientSide){
    //A centroid request MUST have at least two elements, to indicate first the request, second how many centroids.
    double centroid_req[1] = {Requests::GET_CENTROIDS};
    zmq::message_t request(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&centroid_req), 1 * sizeof(double));
    client_socket->send(request);


    // We should receive here an error message
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    std::string array = resp.to_string();

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    std::string expected_string("Malformed request. Should contain number of centroids as second argument "
                                "in request payload, but instead only contained GET_CENTROIDS request and no "
                                "centroid indication.");
    EXPECT_STREQ(array.c_str(), expected_string.c_str());
}


TEST_F(ServerMessagingTest, getRepresentativeRequestWithoutHavingPutAnyPointsInReturnsAnErrorMessageOnClientSide){
    double centroid_req[1] = {Requests::GET_REPS};
    zmq::message_t request(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&centroid_req), 1 * sizeof(double));
    client_socket->send(request);


    // We should receive here an error message
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    std::string array = resp.to_string();


    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    std::string expected_string("Coreset does not contain any point. Please add points before attempting to extract points.");
    EXPECT_STREQ(array.c_str(), expected_string.c_str());
}


TEST_F(ServerMessagingTest, postingPointWithNaNsReturnsAnErrorMessageOnClientSide){
    double data[4] = {Requests::POST_REQ, 0.0, std::nan(""), 0.0};
    zmq::message_t request(4*sizeof(double));
    memcpy((void *) request.data(), (void*)(&data), 4 * sizeof(double));
    client_socket->send(request);


    // We should receive here an error message
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    std::string array = resp.to_string();

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    std::string expected_string("Array contains NaN values. Please fix them.");
    EXPECT_STREQ(array.c_str(), expected_string.c_str());
}


TEST_F(ServerMessagingTest, postingTwoSuccessivePointsOfDifferentDimensionsReturnsAnErrorMessageOnClientSide){
    double data[3] = {Requests::POST_REQ, 12, 10};
    zmq::message_t request(3*sizeof(double));
    memcpy((void *) request.data(), (void*)(&data), 3 * sizeof(double));
    client_socket->send(request);


    // We should receive here a success message
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    std::string array = resp.to_string();

    double data_sec[4] = {Requests::POST_REQ, -1, 4, 5};
    request.rebuild(4*sizeof(double));
    memcpy((void *) request.data(), (void*)(&data_sec), 4 * sizeof(double));
    client_socket->send(request);


    // We should receive here a success message
    client_socket->recv(&resp, 0);
    array = resp.to_string();

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    std::string expected_string("Cannot add a point with this dimension (3) when the first point "
                                "had a different dimension (2)");
    EXPECT_STREQ(array.c_str(), expected_string.c_str());
}


TEST_F(ServerMessagingTest, getCentroidsOfKPointsWithLessThanKPointsInRepresentationReturnsAnErrorMessageOnClientSide){
    // Get many many points
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
    int n_p =0;
    for(int i=0; i < vectors.size(); ++i){
        double data[3] = {Requests::POST_REQ, vectors.at(i)(0), vectors.at(i)(1)};
        zmq::message_t request(3*sizeof(double));
        memcpy((void *) request.data(), (void*)(&data), 3 * sizeof(double));
        client_socket->send(request);


        // We should receive here a success message
        zmq::message_t resp;
        client_socket->recv(&resp, 0);
        std::string array = resp.to_string();
        n_p++;
    }

    // Now request this number of points+1 as centroids should throw an error immediately
    double n_centroids = n_p+2;
    double centroid_req[2] = {Requests::GET_CENTROIDS, n_centroids};
    zmq::message_t request(2*sizeof(double));
    memcpy((void *) request.data(), (void*)(&centroid_req), 2 * sizeof(double));
    client_socket->send(request);

    // Receive potential error message
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    std::string array = resp.to_string();

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    std::string expected_string("Asking for 15002 centroids with only 10 representative points available. "
                                "Please add more points or consider increasing number of representatives.");
    EXPECT_STREQ(array.c_str(), expected_string.c_str());

}


TEST_F(ServerMessagingTest, getRepresentativeOfNPointsReturnsCorrectlyTheNPointsOnClientSide){
    double data[4] = {Requests::POST_REQ, 12, 10, 11};
    zmq::message_t request(4*sizeof(double));
    memcpy((void *) request.data(), (void*)(&data), 4 * sizeof(double));
    client_socket->send(request);


    // We should receive here a success message
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    std::string array = resp.to_string();

    double data_sec[4] = {Requests::POST_REQ, -1, 4, 5};
    request.rebuild(4*sizeof(double));
    memcpy((void *) request.data(), (void*)(&data_sec), 4 * sizeof(double));
    client_socket->send(request);


    // We should receive here a success message
    client_socket->recv(&resp, 0);
    array = resp.to_string();

    // Now query the points!
    double centroid_req[1] = {Requests::GET_REPS};
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&centroid_req), 1 * sizeof(double));
    client_socket->send(request);

    // Receive first the GET_OK response that we expect
    client_socket->recv(&resp, 0);
    double * reqQuery = ServerMessaging::extractDoubleArrayFromContent(resp);
    EXPECT_EQ(reqQuery[0], Requests::GET_OK);
    EXPECT_EQ(reqQuery[1], 2);
    EXPECT_EQ(reqQuery[2], 3);

    delete reqQuery;

    // Send GET_READY
    request.rebuild(1*sizeof(double));
    double ready_req = Requests::GET_READY;
    memcpy((void *) request.data(), (void*)(&ready_req), 1 * sizeof(double));
    client_socket->send(request);

    // Now we will get 3 messages back to back, the points and a GET_DONE message, supposedly
    client_socket->recv(&resp, 0);
    reqQuery = ServerMessaging::extractDoubleArrayFromContent(resp);
    for(int i=0; i < 3; ++i){
        EXPECT_DOUBLE_EQ(reqQuery[i], data[i+1]);
    }
    delete reqQuery;

    // Receive next one
    client_socket->recv(&resp, 0);
    reqQuery = ServerMessaging::extractDoubleArrayFromContent(resp);
    for(int i=0; i < 3; ++i){
        EXPECT_DOUBLE_EQ(reqQuery[i], data_sec[i+1]);
    }
    delete reqQuery;

    // Receive last point
    client_socket->recv(&resp, 0);
    reqQuery = ServerMessaging::extractDoubleArrayFromContent(resp);
    EXPECT_DOUBLE_EQ(reqQuery[0], Requests::GET_DONE);

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);
}


TEST_F(ServerMessagingTest, getCentroidsOfKPointsReturnsCorrectlyTheCentroidsOnClientSide){
    zmq::message_t request(2*sizeof(double));

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);
}


TEST_F(ServerMessagingTest, gettingRepresentativesFollowedByGetCentroidsDoesNotAffectTheFollowingRepresentativesReturned){
    zmq::message_t request(2*sizeof(double));

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);
}