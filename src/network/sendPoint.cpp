#include <zmq.hpp>
#include "mex.hpp"
#include "mexAdapter.hpp"
#include <iostream>
#include "Requests.h"
#include "ClientMessaging.h"
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

        // Connect to port
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://localhost:5555");

        // TODO: CHECK IF WE CAN DIRECTLY CAST THE TYPEDARRAY TO PASS THROUGH THE DATA INSTEAD OF COPYING
        double point_data[n];
        for(int i=0; i < n; ++i){
            point_data[i] = doubleArray[i];
        }

        ClientMessaging::requestPutPoint(socket, point_data, n, REQUEST_TIMEOUT);
        socket.close();
        context.close();
    }

};