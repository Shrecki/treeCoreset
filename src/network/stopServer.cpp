//
// Created by guibertf on 6/1/22.
//
#include "mex.hpp"
#include "mexAdapter.hpp"

#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

//#define REQUEST_TIMEOUT 50000 // msecs, (>1000!)

using namespace matlab::data;
using matlab::mex::ArgumentList;


#include "ClientMessaging.h"
#include <zmq.hpp>

#include <vector>
#include <sstream> //for std::stringstream
#include <string>


class MexFunction : public matlab::mex::Function {
public:
    void operator()(ArgumentList outputs, ArgumentList inputs) {
        // Connect to port
        try{
            zmq::context_t context(1);
            zmq::socket_t socket(context, ZMQ_PAIR);
            socket.connect("ipc:///tmp/test");

            ClientMessaging::requestStop(socket);
            // Disconnect
            socket.close();
            context.close();
        } catch(std::exception &e){
            std::cerr << e.what() << std::endl;
        }

    }

};