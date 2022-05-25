#include <zmq.hpp>
#include "mex.hpp"
#include "mexAdapter.hpp"
#include "Requests.h"

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


class MexFunction : public matlab::mex::Function {
public:
    void operator()(ArgumentList outputs, ArgumentList inputs) {
        // First, we will create the request
        // Prepare request
        double x = GET_REQ;        
        double simpleArray[2];
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
                socket.recv(&reply, 0);
                int nElements(reply.size());
                char byteArray[nElements*sizeof(double)];
                memcpy(byteArray, reply.data(), nElements*sizeof(double));
                double* array = reinterpret_cast<double*>(byteArray);
                std::cout << array[0] << std::endl;
                switch((int)array[0]){

                    // Once we're here, we are expecting to receive a GET_OK response
                    case Requests::GET_OK :{
                        std::cout << "Was an OK response." << std::endl;
                        // First element indicates the number of clusters
                        // Second element indicates the dimension of a centroid
                        int nClusters = (int)array[1];
                        int dimension = (int)array[2];

                        // Start by issuing the POST_OK to signal we are ready to receive
                        zmq::message_t resp(sizeof(double));
                        memcpy((void *) resp.data(), (void*)(&post_ok), sizeof(double));
                        socket.send(resp, 0);

                        std::cout << "Sent out POST_OK:" << std::to_string(post_ok) << std::endl;

                        int64_t more;
                        size_t more_size = sizeof more;

                        // If it is a multi-part message, we will loop until it is finished
                        int rc;
                        int nc = 0;
                        do{
                            std::cout << "Expecting new message now." << std::endl;
                            rc = socket.recv(&reply, 0);
                            assert(rc == 0);
                            rc = zmq_getsockopt (socket, ZMQ_RCVMORE, &more, &more_size);

                            if(more){
                                std::cout << "Received centroid " << std::to_string(nc) << ". Dimension should be: " << reply.size()/sizeof(double) << std::endl;
                                // In this case put the centroid in our cute return array
                                ArrayFactory f;
                                char byteArr[reply.size()];
                                memcpy(byteArr, reply.data(), reply.size());
                                double* array = reinterpret_cast<double*>(byteArr);

                                TypedArray<double> tmpCentroid = f.createArray<double>({dimension,1});
                                for(int j=0; j < dimension; ++j){
                                    tmpCentroid[j] = *(array+j);
                                }
                                // Save this to the output
                                outputs[nc] = tmpCentroid;

                            } else {
                                std::cout << "Received last message: either an end or an error" << std::endl;
                            }
                            assert (rc == 0);
                            reply.rebuild();
                            nc = nc+1;
                        } while(more);

                        // Now, we send POST_OK to signal we're ready to receive all cluster points
                        /*zmq::message_t resp(sizeof(double));
                        memcpy((void *) resp.data(), (void*)(&post_ok), sizeof(double));
                        socket.send(resp);
                        
                        std::cout << "Sent out POST_OK:" << std::to_string(post_ok) << std::endl;
                        
                        for(int clus=0; clus<nClusters; ++clus){

                            ArrayFactory f;
                            // Now we expect to receive a new response, which should contain an array of exactly dimension points.
                            std::cout << "Expecting cluster now." << std::endl;
                            socket.recv(&reply);
                            std::cout << "Received new centroid. Dimension should be: " << reply.size()/sizeof(double) << std::endl;
                            char byteArr[reply.size()];
                            memcpy(byteArr, reply.data(), reply.size());
                            double* array = reinterpret_cast<double*>(byteArr);


                            TypedArray<double> tmpCentroid = f.createArray<double>({dimension,1});
                            for(int j=0; j < dimension; ++j){
                                tmpCentroid[j] = *(array+j);   
                            }

                            // Save this to the output
                            outputs[clus] = tmpCentroid;
                            

                            // Send ok response
                            memcpy((void *) resp.data(), (void*)(&post_ok), sizeof(double)); // Just in case it is consumed?
                            std::cout << ((double*)resp.data())[0] << std::endl;
                            socket.send(resp, 0);
                            std::cout << "Sent out POST_OK:" << std::to_string(post_ok) << std::endl;
                        }
                        socket.recv(&reply); // We await one last response from server before exiting
                         */
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