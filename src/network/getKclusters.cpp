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

#include <sstream> //for std::stringstream
#include <string>


class MexFunction : public matlab::mex::Function {
public:
    void operator()(ArgumentList outputs, ArgumentList inputs) {
        // Connect to port
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://localhost:5555");

        // Get centroids
        std::vector<double>* received_centroids = ClientMessaging::requestCentroids(socket, inputs[0][0], 50000);
        int n_centroids = received_centroids->at(0);
        int dimension = received_centroids->at(1);

        // Copy them back to output
        ArrayFactory f;
        for(int i=0; i < n_centroids; ++i){
            TypedArray<double> tmpCentroid = f.createArray<double>({dimension,1});

            for(int j=0; j < dimension; ++j){
                tmpCentroid[j] = received_centroids->at(i*dimension+j+2);
            }
            outputs[i] = tmpCentroid;
        }
        received_centroids->clear();
        delete received_centroids; // Ensures we do not have any memory leak on client side.

        // Disconnect
        socket.close();
        context.close();
    }

};