#include <zmq.hpp>
#include "mex.hpp"
#include "mexAdapter.hpp"
#include <iostream>
#include "Requests.h"
#include <sstream> //for std::stringstream
#include <string>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#define REQUEST_TIMEOUT 15500 // msecs, (>1000!)

using namespace matlab::data;
using matlab::mex::ArgumentList;


class MexFunction : public matlab::mex::Function {
public:
    void operator()(ArgumentList outputs, ArgumentList inputs) {
        // First, we will create the request
        // Prepare request
        double x = POST_REQ;

        // Get data to pass from input
        TypedArray<double> doubleArray = std::move(inputs[0]);
        // Convert to a format any C++ program can understand
        int n = doubleArray.getNumberOfElements();
        std::cout << "Number of elements : " << n << std::endl;

        double simpleArray[n+1];
        simpleArray[0]=x;
        for(int i=0; i<n; i++){
            simpleArray[i+1] = doubleArray[i];
        }

        zmq::message_t request((n+1)*sizeof(double));
        memcpy((void *) request.data(), (void*)(&simpleArray), (n+1)*sizeof(double));

        // Connect to port
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://localhost:5555");

        // Send request
        socket.send(request);

        int expect_reply = 1;
        while(expect_reply){
            std::cout << "Expecting reply..." << std::endl;
            zmq::pollitem_t  items[] = {{socket, 0, ZMQ_POLLIN, 0}};
            std::cout << "After polling" << std::endl;
            int rc = zmq::poll(items, 1, REQUEST_TIMEOUT);
            std::cout << "Polling done." << std::endl;

            if(rc == -1){ break;}
            if(items[0].revents & ZMQ_POLLIN){
                zmq::message_t reply;
                socket.recv(&reply);
                int nElems(reply.size());
                char byteArray[nElems];
                memcpy(byteArray, reply.data(), nElems);
                if((int)byteArray[0] == Requests::POST_OK){
                    std::cout << "Point added successfully" << std::endl;
                } else {
                    std::cout << "Point addition failed!" << std::endl;
                }
                expect_reply=0;
            } else {
                std::cout << "Server seems to be offline, abandoning." << std::endl;
                expect_reply=0;
            }
        }

        socket.close();
        context.close();
    }

};