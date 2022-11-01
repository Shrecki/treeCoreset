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
        // Connect to port
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_PAIR);
        socket.connect("ipc:///tmp/0");

        // Get data to pass from input
        int n  = inputs[0].getNumberOfElements();
        double point_data[n];
        //point_data = std::move(inputs[0]);
        //memcpy(point_data, &inputs[0], n);
        for(int i=0; i < n; ++i){
            point_data[i] = inputs[0][i]; //doubleArray[i];
        }

        ClientMessaging::requestPutPoint(socket, point_data, n, REQUEST_TIMEOUT);
        socket.close();
        context.close();
    }

};