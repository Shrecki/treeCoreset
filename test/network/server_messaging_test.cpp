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


    std::string expected_string("vector::_M_range_check: __n (which is 0) >= this->size() (which is 0)");
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
    zmq::message_t request(2*sizeof(double));

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);
}


TEST_F(ServerMessagingTest, postingPointWithNaNsReturnsAnErrorMessageOnClientSide){
    zmq::message_t request(2*sizeof(double));

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);
}


TEST_F(ServerMessagingTest, postingTwoSuccessivePointsOfDifferentDimensionsReturnsAnErrorMessageOnClientSide){
    zmq::message_t request(2*sizeof(double));

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);
}


TEST_F(ServerMessagingTest, getCentroidsOfKPointsWithLessThanKPointsInRepresentationReturnsAnErrorMessageOnClientSide){
    zmq::message_t request(2*sizeof(double));

    // Now we stop the server
    double stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);
}


TEST_F(ServerMessagingTest, getRepresentativeOfNPointsReturnsCorrectlyTheNPointOnClientSide){
    zmq::message_t request(2*sizeof(double));

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