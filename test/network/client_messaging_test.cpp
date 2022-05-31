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

    // Free memory of vectors
    for(int i=0; i < 2; ++i){
        delete reps->at(i);
        reps->at(i) = nullptr;
    }
    delete reps;
    reps = nullptr;


    ClientMessaging::requestStop(*client_socket);
}
