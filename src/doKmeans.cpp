#include <zmq.hpp>
#include "mex.hpp"
#include "mexAdapter.hpp"
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#define REQUEST_TIMEOUT 50000 // msecs, (>1000!)

using namespace matlab::data;
using matlab::mex::ArgumentList;

#include <sstream> //for std::stringstream
#include <string>


enum Requests {
    POST_REQ, GET_REQ, LOAD_REQ, SAVE_REQ, STOP_REQ, POST_OK, GET_OK, LOAD_OK, SAVE_OK, STOP_OK, ERROR
};

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
        
        double post_ok = POST_OK;

        int expect_reply = 1;
        while(expect_reply){
            std::cout << "Expecting reply..." << std::endl;
            zmq::pollitem_t  items[] = {{socket, 0, ZMQ_POLLIN, 0}};
            std::cout << "After polling" << std::endl;
            int rc = zmq::poll(items, 1, REQUEST_TIMEOUT);
            std::cout << "Polling done." << std::endl;
            
            if(rc == -1){ break;}
            if(items[0].revents & ZMQ_POLLIN){
                std::cout << "Received response." << std::endl;
                zmq::message_t reply;
                socket.recv(&reply);
                int nElements(reply.size());
                char byteArray[nElements*sizeof(double)];
                memcpy(byteArray, reply.data(), nElements*sizeof(double));
                double* array = reinterpret_cast<double*>(byteArray);
                std::cout << array[0] << std::endl;
                switch((int)array[0]){
                    case Requests::GET_OK :{
                        std::cout << "Was an OK response." << std::endl;
                        // First element indicates the number of clusters
                        // Second element indicates the dimension of a centroid
                        int nClusters = (int)array[1];
                        int dimension = (int)array[2];
                        
                        // Now, we send POST_OK to signal we're ready to receive all cluster points
                        zmq::message_t resp(sizeof(double));
                        memcpy((void *) resp.data(), (void*)(&post_ok), sizeof(double));
                        socket.send(resp);
                        
                        std::cout << "Replied OK" << std::endl;

                        ArrayFactory f;
                        for(int i=0; i < nClusters; ++i){
                            std::vector<double> currCentroid;
                            currCentroid.reserve(dimension);
                            // Now we expect to receive a new response, which should contain an array of exactly dimension points.
                            socket.recv(&reply);
                            std::cout << "Received new centroid." << std::endl;
                            char byteArr[dimension*sizeof(double)];
                            memcpy(byteArr, reply.data(), dimension*sizeof(double));
                            double* array = reinterpret_cast<double*>(byteArray);
                            
                            // Copy back this array to a new tmpCentroid array
                            for(int j=0; j < dimension; ++j){
                                currCentroid.push_back(array[j]);
                            }
                            TypedArray<double> tmpCentroid = f.createArray<double>({dimension,1});
                            for(int j=0; j < dimension; ++j){
                                tmpCentroid[j] = currCentroid.at(j);   
                            }
                            
                            // Save this to the output
                            outputs[i] = tmpCentroid;
                            
                            // Send ok response
                            socket.send(resp);
                            std::cout << "Sent out OK" << std::endl;
                        }
                        std::cout << "Done with centroids" << std::endl;
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