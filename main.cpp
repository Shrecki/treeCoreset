#include <iostream>
#include <zmq.hpp>
#include <string>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "src/coreset_algorithms.h"
#include "src/ClusteredPoints.h"
#include "src/Point.h"
#include "src/kmeansplusplus.h"
#include "src/Requests.h"

#define N_SAMPLES 728*14.0
#define M 2400

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
        socket.recv (&request);
        int nElems(request.size()/sizeof(double));

        // Copy it back to memory
        char byteArray[request.size()];
        memcpy(byteArray, request.data(), request.size());

        // The first double defines the request that we want
        double* array = reinterpret_cast<double*>(byteArray);
        switch((int)array[0]){
            case Requests::POST_REQ : {
                std::cout << "Received a POST request." << std::endl;

                // Convert to a point
                Requests response;
                try{
                    // Convert received data to point
                    Point* p = Point::convertArrayToPoint(&array[1], nElems-1);
                    Eigen::VectorXd vec = p->getData();
                    for(int i=0; i < vec.size(); ++i){
                        assert(!std::isnan(vec(i)));
                    }
                    assert(p->getData().array().isNaN().sum() == 0);
                    // Perform insertion (this is the InsertPoint step in the stream)
                    clusteredPoints.insertPoint(p);

                    // Send back response: all went well
                    response = Requests::POST_OK;
                    zmq::message_t reply(1);
                    memcpy((void *) reply.data(), &response, 1);
                    socket.send(reply);
                    std::cout << "Post OK" << std::endl;

                } catch (std::exception &e) {
                    response = Requests::ERROR;

                    // Here, send also the exception (but we will worry about it later)
                    zmq::message_t reply(1);
                    memcpy((void *) reply.data(), &response, 1);
                    socket.send(reply);

                    // If something went wrong, we don't need to stop the stream
                    std::cout << e.what() << std::endl;
                    throw e;
                }
                break;
            }
            case Requests::GET_REQ : {
                std::cout << "Received a GET request." << std::endl;

                std::vector<double> data;
                try {
                    clusteredPoints.getClustersAsFlattenedArray(data, (int)array[1], 100);

                    // Reply contains the OK response, followed by the data to send
                    zmq::message_t reply(data.size()+3);
                    double tmpdata[data.size()+3];
                    tmpdata[0] = (double)Requests::GET_OK; // All went well
                    tmpdata[1] = 10; // Number of clusters
                    tmpdata[2] = data.size()/10; // Dimension of a single centroid
                    memcpy(tmpdata+3, data.data(), data.size()*sizeof(double)); // Copy back our vector
                    memcpy((void*) reply.data(), tmpdata, (data.size()+3)*sizeof(double)); // Put all elements into reply
                    socket.send(reply); // Issue reply

                } catch (std::exception &e){
                    std::cout << e.what() << std::endl;
                    // We'd need some code to actually issue the exception

                    // First entry is the requests error signal
                    // The rest is the exception, as a text
                }
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
    }
    socket.close();
    return 0;
}
