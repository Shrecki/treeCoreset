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

    ServerMessagingTest(){
        context = new zmq::context_t(2);
        server_socket = new zmq::socket_t(*context, ZMQ_REP);
        client_socket = new zmq::socket_t(*context, ZMQ_REQ);
        //zmq::context_t ct(2);
        //zmq::socket_t server_socket(context, ZMQ_REP);
        server_socket->bind("tcp://*:5555");

        //zmq::socket_t client_socket(context, ZMQ_REQ);
        client_socket->connect("tcp://localhost:5555");
    }

    ~ServerMessagingTest() override {
        server_socket->close();
        client_socket->close();
        context->close();

        delete server_socket;
        delete client_socket;
        delete context;
    }

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(ServerMessagingTest, stoppingServerWorks) {
    std::thread t1(ServerMessaging::runServer, std::ref(*server_socket), 1000, 10);

    double stop = Requests::STOP_REQ;
    zmq::message_t request(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    // We expect to receive a STOP_OK response
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    double *array = ServerMessaging::extractDoubleArrayFromContent(resp);
    EXPECT_EQ((int)array[0], Requests::STOP_OK);
    t1.join();
}

TEST_F(ServerMessagingTest, sendingAnUnknownRequestReturnsAnErrorOnClientSide){

    std::thread t1(ServerMessaging::runServer, std::ref(*server_socket), 1000, 10);

    double stop = 121; // This request does not exist
    zmq::message_t request(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    // We expect to receive a STOP_OK response
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    std::string array = resp.to_string();
    //double *array = ServerMessaging::extractDoubleArrayFromContent(resp);


    // Now we stop the server
    stop = Requests::STOP_REQ;
    request.rebuild(1*sizeof(double));
    memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
    client_socket->send(request);

    t1.join();

    std::string expected_string("Unknown request");
    EXPECT_STREQ(array.c_str(), expected_string.c_str());

}