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
        server_socket = new zmq::socket_t(*context, ZMQ_REP);
        client_socket = new zmq::socket_t(*context, ZMQ_REQ);

    }

    ~ClientMessagingTest() override {
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

TEST_F(ClientMessagingTest, requestingStopCorrectlyStopsServerAndReturnsExpectedMessage) {
    ClientMessaging::requestStop(*client_socket);

    // We expect now to receive a stop OK
    zmq::message_t resp;
    client_socket->recv(&resp, 0);
    double *array = ServerMessaging::extractDoubleArrayFromContent(resp);
    EXPECT_EQ((int)array[0], Requests::STOP_OK);
    delete [] array;
}

TEST_F(ClientMessagingTest, requestingToPutPointsIsReturningOKResponse){
    // We will request to put in a simple point
    double point[3] = {0.1, 0.2, 0.4};
    EXPECT_NO_THROW(ClientMessaging::requestPutPoint(*client_socket, point, 3, 1000));

    ClientMessaging::requestStop(*client_socket);
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

TEST_F(ClientMessagingTest, postingPointAndGettingRepresentativesReturnsExactlyPostedPoint){

}
