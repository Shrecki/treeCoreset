#include <iostream>
#include <zmq.hpp>
#include <string>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "src/clustering/coreset_algorithms.h"
#include "src/clustering/ClusteredPoints.h"
#include "src/clustering/Point.h"
#include "src/clustering/kmeansplusplus.h"
#include "src/network/ServerMessaging.h"
#include "src/network/Requests.h"

#define N_SAMPLES 728*14.0
#define M 132


int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:5555");

    std::string resp("World");
    zmq::const_buffer message((void*)&resp,5);
    //socket.send(message, static_cast<zmq::send_flags>(0));

    ClusteredPoints clusteredPoints(ceil(log2(N_SAMPLES/M)+2), M);

    bool mustContinue(true);
    while(mustContinue){
        // Await a request
        zmq::message_t request;
        socket.recv (&request, 0);
        int nElems(ServerMessaging::getNumberOfDoublesInReq(request));
        double *array = ServerMessaging::extractDoubleArrayFromContent(request);
        switch((int)array[0]){
            case Requests::POST_REQ : {
                ServerMessaging::handlePostRequest(array, nElems, clusteredPoints, socket);
                break;
            }
            case Requests::GET_CENTROIDS : {
                ServerMessaging::handleGetCentroids(array, nElems, clusteredPoints, socket);
                break;
            }
            case Requests::GET_REPS: {
                std::cout << "Received a Get Representatives request" << std::endl;
                ServerMessaging::handleGetRepresentatives(array, nElems, clusteredPoints, socket, M);
                break;
            }
            case Requests::LOAD_REQ: {
                std::cout << "Received a LOAD request." << std::endl;
                zmq::message_t reply(5);
                memcpy((void *) reply.data(), "World", 5);
                socket.send(reply);
                break;
            }
            case Requests::SAVE_REQ: {
                std::cout << "Received a SAVE request." << std::endl;
                zmq::message_t reply(5);
                memcpy((void *) reply.data(), "World", 5);
                socket.send(reply);
                break;
            }
            case Requests::STOP_REQ: {
                std::cout << "Received a STOP request." << std::endl;
                // Send back response: we know we got the stop
                mustContinue = false;
                zmq::message_t reply(5);
                memcpy((void *) reply.data(), "World", 5);
                socket.send(reply);
                break;
            }
            case Requests::POST_OK: {
                std::cout << "Simulating big CPU overload." << std::endl;
                sleep(3);
                zmq::message_t reply(5);
                memcpy((void *) reply.data(), "World", 5);
                socket.send(reply);
                break;
            }
            default: {
                std::cout << "Unknown request." << std::endl;
                zmq::message_t reply(5);
                memcpy((void *) reply.data(), "World", 5);
                socket.send(reply);
                break;
            }
        }

        delete array;
    }
    socket.close();
    return 0;
}
