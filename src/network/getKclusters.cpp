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
        // First, we will create the request to get the centroids
        // This request has the flag GET_CENTROIDS but also the number of clusters to get (ie: how many in k-means)
        //double x = GET_CENTROIDS;
        double simpleArray[2];
        simpleArray[0]= GET_CENTROIDS;
        simpleArray[1]=inputs[0][0]; // Number of clusters

        // With this, the request is ready.
        zmq::message_t request(2*sizeof(double));
        memcpy((void *) request.data(), (void*)(&simpleArray), 2*sizeof(double));

        // Connect to port
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://localhost:5555");

        // Send the GET_CENTROIDS request to server
        socket.send(request);

        int expect_reply = 1;

        // Start polling for a response and timeout if nothing received
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
                    // The protocol works as follows:
                    // - Client (the one running this script) issues GET_REQ
                    // - Server, if ready, will issue a GET_OK, with number of elements to receive and dimension of each element (all elements have the same dimension)
                    // - Client then issues a GET_READY, signaling it is ready to receive
                    // - Server sends at most n_elements+1 messages
                    // - Last message sent by server is either an exception or a GET_DONE message.
                    case Requests::GET_OK :{
                        std::cout << "Was an OK response." << std::endl;
                        // First element indicates the number of clusters
                        // Second element indicates the dimension of a centroid
                        int nClusters = (int)array[1];
                        int dimension = (int)array[2];

                        // Start by issuing the GET_READY to signal we are ready to receive
                        double get_ready = GET_READY;
                        zmq::message_t resp(sizeof(double));
                        memcpy((void *) resp.data(), (void*)(&get_ready), sizeof(double));
                        socket.send(resp, 0);
                        std::cout << "Sent out GET_READY:" << std::to_string(get_ready) << std::endl;

                        int64_t more;
                        size_t more_size = sizeof more;

                        // We should expect now a multi part message: we will receive all messages one by one.
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
                                // We should check here whether it conforms to the expected message or not
                            }
                            assert (rc == 0);
                            reply.rebuild();
                            nc = nc+1;
                        } while(more);

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