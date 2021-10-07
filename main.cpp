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
                    int k = (int)array[1];
                    clusteredPoints.getClustersAsFlattenedArray(data, k, 100);
                    //clusteredPoints.~ClusteredPoints();

                    int n = data.size();
                    zmq::message_t reply(3*sizeof(double));

                    // First step is sending the number of clusters along with GET_OK to signal that all went well
                    double tmpData[3] = {Requests::GET_OK, (double)k, 1.0*n/k};

                    memcpy((void*)reply.data(), tmpData, 3*sizeof(double)); // First we send OK, with all relevant sizes so that the other side knows what to wait for
                    socket.send(reply); // Issue reply

                    // Now, we await an OK response
                    socket.recv (&request);
                    char byteArr[request.size()];
                    memcpy(byteArr, request.data(), request.size());

                    // The first double defines the request that we want
                    array = reinterpret_cast<double*>(byteArr);
                    int dim = n/k;
                    std::cout << dim << std::endl;
                    if(array[0] == POST_OK){
                        // We can start transmitting the different cluster values
                        for(int i=0; i < k; ++i){
                            //tmpVec = new double[n/k];
                            double tmpVec[dim];
                            for(int j=0;j< dim; ++j){
                                tmpVec[j] = data.at(i*dim +j);
                            }
                            //memcpy(&tmpVec, data.data()+i*dim, dim*sizeof(double));
                            zmq::message_t rep(dim*sizeof(double));
                            memcpy((void*)rep.data(), (void*)(&tmpVec), dim*sizeof(double)); // First we send OK, with all relevant sizes so that the other side knows what to wait for
                            socket.send(rep);
                            std::cout << "Transmitted cluster " << i << std::endl;
                            // Await OK response
                            socket.recv(&request);
                            char resp[request.size()];
                            memcpy(resp, request.data(), request.size());
                            array = reinterpret_cast<double*>(byteArr);
                            if(array[0] != POST_OK){
                                throw std::logic_error("Did not receive an OK response");
                            }
                            std::cout << "Received POST_OK" << std::endl;
                        }
                        // To signal that we are done, we send back one last OK
                        socket.send(reply);
                    } else {
                        // Something baad happened! Throw an exception
                    }


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
