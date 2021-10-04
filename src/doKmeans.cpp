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
#define REQUEST_TIMEOUT 20500 // msecs, (>1000!)

using namespace matlab::data;
using matlab::mex::ArgumentList;


class MexFunction : public matlab::mex::Function {
public:
    void operator()(ArgumentList outputs, ArgumentList inputs) {
        // First, we will create the request
        // Prepare request
        double x = GET_REQ;
        double simpleArray[1];
        simpleArray[0]=x;
        simpleArray[1]=inputs[0][0]; // Number of clusters
        std::cout << simpleArray[1] << std::endl;

        zmq::message_t request(2*sizeof(double));
        memcpy((void *) request.data(), (void*)(&simpleArray), 2*sizeof(double));

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
                int nElements(reply.size());
                char byteArray[nElements];
                double* array = reinterpret_cast<double*>(byteArray);
                switch((int)array[0]){
                    case Requests::GET_OK :{
                        // First element indicates the number of clusters
                        // Second element indicates the dimension of a centroid
                        // Other elements are the values, as a single array
                        int nClusters = (int)array[1];
                        int dimension = (int)array[2];
                        ArrayFactory f;
                        for(int i=0; i < nElements; ++i){
                            std::vector<double> currCentroid;
                            currCentroid.reserve(dimension);
                            for(int j=0; j < dimension; ++j){
                                currCentroid.push_back(array[i*dimension + j + 3]);
                            }
                            TypedArray<double> tmpCentroid = f.createArray<double>({nClusters*dimension,1});
                            for(int j=0; j < nClusters*dimension; ++j){
                                tmpCentroid[j] = currCentroid.at(j);
                            }
                            outputs[i] = tmpCentroid;
                        }
                        break;
                    }

                    case Requests::ERROR : {
                        // Something went wrong: print the exception
                        break;
                    }

                    default: {
                        // Invalid response to this query
                        break;
                    }
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