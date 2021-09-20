#include <iostream>
#include <zmq.hpp>
#include <string>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "src/State.h"


int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:5555");

    std::string resp("World");
    zmq::const_buffer message((void*)&resp,5);
    //socket.send(message, static_cast<zmq::send_flags>(0));

    bool mustContinue(true);
    while(mustContinue){
        // Await a request
        zmq::message_t request;
        socket.recv (&request);
        int nElems(request.size());

        // Copy it back to memory
        char byteArray[nElems];
        memcpy(byteArray, request.data(), nElems);

        // The first double defines the request that we want
        double* array = reinterpret_cast<double*>(byteArray);
        switch((int)array[0]){
            case Requests::POST_REQ : {
                std::cout << "Received a POST request. First 100 elements:" << std::endl;

                for(int i=1; i<100;++i){
                    std::cout << array[i] << std::endl;
                }

                // Response is a simple POST_OK with nothing else
                break;
            }
            case Requests::GET_REQ : {
                std::cout << "Received a GET request." << std::endl;
                // Response should contain relevant elements
                break;
            }
            case Requests::LOAD_REQ: {
                std::cout << "Received a LOAD request." << std::endl;
                // Response should be a
                break;
            }
            case Requests::SAVE_REQ: {
                std::cout << "Received a SAVE request." << std::endl;
                break;
            }
            case Requests::STOP_REQ: {
                std::cout << "Received a STOP request." << std::endl;
                mustContinue = false;
                break;
            }
            case Requests::POST_OK: {
                std::cout << "Simulating big CPU overload." << std::endl;
                sleep(3);
                break;
            }
            default: {
                std::cout << "Unknown request." << std::endl;
                break;
            }
        }

        // Do main algorithmic work here

        zmq::message_t reply(5);
        memcpy((void *) reply.data(), "World", 5);
        socket.send(reply);
    }
    socket.close();
    return 0;
}
