//
// Created by guibertf on 5/30/22.
//

#include "ClientMessaging.h"

#include "Requests.h"
#include "ServerMessaging.h"
#include <iostream>
#include <functional>

namespace ClientMessaging {
    std::vector<std::vector<double>*> *centroid_multipart(double *array, zmq::socket_t &socket, Requests last_message){
        // First element indicates the number of clusters
        // Second element indicates the dimension of a centroid
        int nClusters = (int)array[1];
        int dimension = (int)array[2];

        // Let's allocate precisely that many elements in our return array
        auto *return_vec = new std::vector<std::vector<double>*>();
        return_vec->reserve(nClusters);

        int64_t more;
        size_t more_size = sizeof more;

        zmq::message_t reply;

        int nc = 0;
        do {
            socket.recv(&reply, 0);
            zmq_getsockopt (socket, ZMQ_RCVMORE, &more, &more_size);

            if(more){
                int n_dims = reply.size()/sizeof(double);
                // We should check that n_dims is equal to dimension here
                if(n_dims != dimension){
                    throw std::runtime_error("Dimension announced by server and actual point dimension do not match. Exiting.");
                }
                // In this case put the centroid in our cute return msg_data
                char msg_data_bytes[reply.size()];
                memcpy(msg_data_bytes, reply.data(), reply.size());
                double* msg_data = reinterpret_cast<double*>(msg_data_bytes);

                auto new_vec = new std::vector<double>();
                new_vec->reserve(dimension);

                for(int j=0; j < n_dims; ++j){
                    new_vec->push_back(msg_data[j]);
                }
                return_vec->push_back(new_vec);
            } else {
                // Last message must be a GET_DONE, otherwise something is off
                auto reqQuery = ServerMessaging::extractDoubleArrayFromContent(reply);
                if(reqQuery[0] != Requests::GET_DONE){
                    // Likely, the message is actually an exception:
                    delete [] reqQuery;
                    throw std::runtime_error("Server says: " + reply.to_string());
                }
                delete [] reqQuery;
                // std::cout << "Received last message: either an end or an error" << std::endl;
                // We should check here whether it conforms to the expected message or not
            }
            reply.rebuild();
            nc = nc+1;
        } while(more);
        return return_vec;
    }

    std::vector<std::vector<double>*> * pollForRequest(zmq::socket_t &socket, double* client_init_req_data, int client_init_req_size,
                        Requests server_init_resp, Requests client_resp, Requests server_last_resp, int REQUEST_TIMEOUT,
                        const std::function<std::vector<std::vector<double>*>*(double*, zmq::socket_t&, Requests)> &handle_multipart_response){
        // In this method, we are provided with a start request data pointer.
        // Furthermore, we are provided with an expected server initial client response and a client's consequent response
        // Lastly, we're provided with the server last expected response
        zmq::message_t request(client_init_req_size*sizeof(double));
        memcpy((void *) request.data(), (void*)(client_init_req_data), client_init_req_size * sizeof(double));
        socket.send(request);

        std::vector<std::vector<double>*> * output_data = nullptr;

        int expect_reply = 1;
        while(expect_reply) {
            zmq::pollitem_t items[1] = {{socket, 0, ZMQ_POLLIN, 0}};
            int rc = zmq::poll(items, 1, REQUEST_TIMEOUT);

            if (rc == -1) { break; }
            if (items[0].revents & ZMQ_POLLIN) {
                // Check for a reply
                zmq::message_t reply;
                socket.recv(&reply, 0);

                int nElements(reply.size());
                char byteArray[nElements * sizeof(double)];
                memcpy(byteArray, reply.data(), nElements * sizeof(double));
                double *response_data = reinterpret_cast<double *>(byteArray);

                if ((int) response_data[0] == server_init_resp) {
                    // If server reply conforms to expectations, send the client response.
                    double get_ready = client_resp;
                    zmq::message_t resp(sizeof(double));
                    memcpy((void *) resp.data(), (void *) (&get_ready), sizeof(double));
                    socket.send(resp, 0);

                    // We should expect now a multi part message: we will receive all messages one by one.
                    // However for this to work, we need to put everything in a single response_data.
                    try{
                        output_data = handle_multipart_response(response_data, socket, server_last_resp);
                    } catch(std::exception &e){
                        throw e;
                    }

                    break;
                } else {
                    throw std::runtime_error("Server says: " + reply.to_string());
                }
                expect_reply = 0;
            } else {
                std::cout << "Server seems to be offline, abandoning." << std::endl;
                expect_reply = 0;
            }
        }

        return output_data;

    }

    void requestStop(zmq::socket_t &socket){
        double stop = Requests::STOP_REQ;
        zmq::message_t request(1*sizeof(double));
        memcpy((void *) request.data(), (void*)(&stop), 1 * sizeof(double));
        socket.send(request);
    }

    void requestPutPoint(zmq::socket_t &socket, double *point_data, unsigned int dimension, int REQUEST_TIMEOUT){
        if(dimension == 0){
            throw std::invalid_argument("Input dimension cannot be 0. Please pass a point with at least dimension 1.");
        }
        double request_array[dimension+1];
        request_array[0]=Requests::POST_REQ;

        memcpy(request_array+1, point_data, dimension*sizeof(double));
        /*for(int i=0; i<dimension; i++){
            request_array[i+1] = point_data[i];
        }*/

        zmq::message_t request((dimension+1)*sizeof(double));
        memcpy((void *) request.data(), (void*)(&request_array), (dimension+1)*sizeof(double));

        // Send request
        socket.send(request);

        int expect_reply = 1;
        while(expect_reply){
            zmq::pollitem_t  items[] = {{socket, 0, ZMQ_POLLIN, 0}};
            int rc = zmq::poll(items, 1, REQUEST_TIMEOUT);

            if(rc == -1){ break;}
            if(items[0].revents & ZMQ_POLLIN){
                zmq::message_t reply;
                socket.recv(&reply);
                int nElems(reply.size());
                char byteArray[nElems];
                memcpy(byteArray, reply.data(), nElems);
                if((int)byteArray[0] != Requests::POST_OK){
                  // We might have gotten an exception in here. Print it!
                  throw std::runtime_error("Server says: " + reply.to_string());
                }
                expect_reply=0;
            } else {
                throw std::runtime_error("Server timeout. Check the program did not crash.");
            }
        }

    }


    std::vector<std::vector<double>*>* requestCentroids(zmq::socket_t &socket, unsigned int n_centroids, int REQUEST_TIMEOUT){
        if(n_centroids == 0){
            throw std::invalid_argument("Number of centroids cannot be zero.");
        }
        double request_data[2] = {GET_CENTROIDS, 1.0 * n_centroids};
        return pollForRequest(socket, request_data,2,
                       Requests::GET_OK,Requests::GET_READY, Requests::GET_DONE,
                       REQUEST_TIMEOUT,&centroid_multipart);
    }

    std::vector<std::vector<double>*>*  requestRepresentatives(zmq::socket_t &socket, int REQUEST_TIMEOUT){
        double request_data[1] = {GET_REPS};
        return pollForRequest(socket, request_data,1,
                              Requests::GET_OK,Requests::GET_READY, Requests::GET_DONE,
                              REQUEST_TIMEOUT,&centroid_multipart);
    }

}