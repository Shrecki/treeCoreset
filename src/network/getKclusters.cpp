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
            socket.connect("inproc://#1");

            // Get centroids
            std::vector<std::vector<double>*>* received_centroids = ClientMessaging::requestCentroids(socket, inputs[0][0], 50000);
            int n_centroids = received_centroids->size();
            int dimension = received_centroids->at(0)->size();

            // Copy them back to output
            ArrayFactory f;
            int transmittable_size = outputs.size();
            if(n_centroids > transmittable_size){
                std::cerr << "Output size (" << std::to_string(transmittable_size) << ") smaller than number of points received (" << std::to_string(n_centroids) << "). Output will be truncated." << std::endl;
            }
            if(transmittable_size > n_centroids) {
                std::cerr << "Output size (" << std::to_string(transmittable_size) << ") bigger than number of points received (" << std::to_string(n_centroids) << "). Output will be truncated." << std::endl;
            }
            int n_transmit = std::min(transmittable_size, n_centroids);
            for(int i=0; i < n_transmit; ++i){
                TypedArray<double> tmpCentroid = f.createArray<double>({1, dimension}, received_centroids->at(i)->data(), received_centroids->at(i)->data() + received_centroids->at(i)->size()); //std::move(received_representatives->at(i)->data());//f.createArray<double>({dimension,1});
                outputs[i] = tmpCentroid;
            }

            ClientMessaging::free_vector(received_centroids);
            // Disconnect
            socket.close();
            context.close();
        } catch(std::exception &e){
            std::cerr << e.what() << std::endl;
        }

    }

};