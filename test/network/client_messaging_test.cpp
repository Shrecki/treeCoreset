//
// Created by guibertf on 5/30/22.
//
#include "clustering/ClusteredPoints.h"
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include <fstream>
#include "../utils.h"

#include "network/ClientMessaging.h"


using ::testing::Return;

class ClientMessagingTest : public ::testing::Test {
protected:
    zmq::socket_t* server_socket;
    zmq::socket_t* client_socket;
    zmq::context_t* context;

    ClientMessagingTest(){
        context = new zmq::context_t(2);
        server_socket = new zmq::socket_t(*context, ZMQ_REP);
        client_socket = new zmq::socket_t(*context, ZMQ_REQ);
        //zmq::context_t ct(2);
        //zmq::socket_t server_socket(context, ZMQ_REP);
        server_socket->bind("tcp://*:5555");

        //zmq::socket_t client_socket(context, ZMQ_REQ);
        client_socket->connect("tcp://localhost:5555");
    }

    ~ClientMessagingTest() override {
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

TEST_F(ClientMessagingTest, requestingCentroidsFromServerWhenNoCentroidsAvailableShouldRaiseException) {
    std::cout << "Hi" << std::endl;
}