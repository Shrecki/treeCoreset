#include <zmq.hpp>
#include "mex.hpp"
#include "mexAdapter.hpp"
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif


using namespace matlab::data;
using matlab::mex::ArgumentList;

#include <sstream> //for std::stringstream

class MexFunction : public matlab::mex::Function {
public:
    void operator()(ArgumentList outputs, ArgumentList inputs) {
        // Should put a sendImage request first here then await for an OK

        // This is to pass an array
        // Now, we will try passing in an array
        TypedArray<double> doubleArray = std::move(inputs[0]);

        double simpleArray[doubleArray.getNumberOfElements()];
        for(int i=0; i<doubleArray.getNumberOfElements(); i++){
            simpleArray[i] = doubleArray[i];
        }

        // First connect to the port
        // Then send the request
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://localhost:5555");


        int n = doubleArray.getNumberOfElements();
        std::cout << n << std::endl;
        zmq::message_t request(n*sizeof(double));
        memcpy((void *) request.data(), (void*)(&simpleArray), n*sizeof(double));

        socket.send(request);

        zmq::message_t reply(5);
        socket.recv(&reply);
        std::cout << reply << std::endl;
        socket.close();
    }

};