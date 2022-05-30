//
// Created by guibertf on 5/30/22.
//

#include "ClientMessaging.h"

#include "Requests.h"
#include <iostream>
#include <functional>

namespace ClientMessaging {
    std::vector<double> *centroid_multipart(double *array, zmq::socket_t &socket, Requests last_message){
        // First element indicates the number of clusters
        // Second element indicates the dimension of a centroid
        int nClusters = (int)array[1];
        int dimension = (int)array[2];

        // Let's allocate precisely that many elements in our return array
        auto *return_vec = new std::vector<double>();
        return_vec->reserve(nClusters*dimension+2);

        return_vec->push_back(nClusters);
        return_vec->push_back(dimension);

        int64_t more;
        size_t more_size = sizeof more;

        zmq::message_t reply;

        int rc;
        int nc = 0;
        int curr_elem=0;
        do{
            std::cout << "Expecting new message now." << std::endl;
            rc = socket.recv(&reply, 0);
            assert(rc == 0);
            rc = zmq_getsockopt (socket, ZMQ_RCVMORE, &more, &more_size);

            if(more){
                int n_dims = reply.size()/sizeof(double);

                // We should check that n_dims is equal to dimension here

                std::cout << "Received centroid " << std::to_string(nc) << ". Dimension should be: " << n_dims << std::endl;
                // In this case put the centroid in our cute return msg_data
                //ArrayFactory f;
                char msg_data_bytes[reply.size()];
                memcpy(msg_data_bytes, reply.data(), reply.size());
                double* msg_data = reinterpret_cast<double*>(msg_data_bytes);

                for(int j=0; j < n_dims; ++j){
                    return_vec->push_back(msg_data[j]);
                }
                curr_elem = curr_elem+1;
            } else {
                std::cout << "Received last message: either an end or an error" << std::endl;
                // We should check here whether it conforms to the expected message or not
            }
            assert (rc == 0);
            reply.rebuild();
            nc = nc+1;
        } while(more);
        std::cout << "Done with centroids" << std::endl;
        return return_vec;
    }

    std::vector<double> * pollForRequest(zmq::socket_t &socket, double* client_init_req_data, int client_init_req_size,
                        Requests server_init_resp, Requests client_resp, Requests server_last_resp, int REQUEST_TIMEOUT,
                        const std::function<std::vector<double>*(double*, zmq::socket_t&, Requests)> &handle_multipart_response){
        // In this method, we are provided with a start request data pointer.
        // Furthermore, we are provided with an expected server initial client response and a client's consequent response
        // Lastly, we're provided with the server last expected response
        zmq::message_t request(client_init_req_size*sizeof(double));
        memcpy((void *) request.data(), (void*)(&client_init_req_data), client_init_req_size * sizeof(double));
        socket.send(request);

        std::vector<double> * output_data = nullptr;

        int expect_reply = 1;
        while(expect_reply) {
            std::cout << "Expecting reply..." << std::endl;
            zmq::pollitem_t items[] = {{socket, 0, ZMQ_POLLIN, 0}};
            std::cout << "After polling" << std::endl;
            int rc = zmq::poll(items, 1, REQUEST_TIMEOUT);
            std::cout << "Polling done." << std::endl;

            if (rc == -1) { break; }
            if (items[0].revents & ZMQ_POLLIN) {
                std::cout << "Received response." << std::endl;
                zmq::message_t reply;
                socket.recv(&reply, 0);
                int nElements(reply.size());
                char byteArray[nElements * sizeof(double)];
                memcpy(byteArray, reply.data(), nElements * sizeof(double));
                double *response_data = reinterpret_cast<double *>(byteArray);
                if ((int) response_data[0] == server_init_resp) {
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
                    switch ((int) response_data[0]) {

                        // Once we're here, we are expecting to receive a GET_OK response
                        // The protocol works as follows:
                        // - Client (the one running this script) issues GET_REQ
                        // - Server, if ready, will issue a GET_OK, with number of elements to receive and dimension of each element (all elements have the same dimension)
                        // - Client then issues a GET_READY, signaling it is ready to receive
                        // - Server sends at most n_elements+1 messages
                        // - Last message sent by server is either an exception or a GET_DONE message.

                        case Requests::ERROR : {
                            // Something went wrong: print the exception
                            break;
                        }

                        default: {
                            // Invalid response to this query
                            break;
                        }
                    }
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


    std::vector<double>* requestCentroids(zmq::socket_t &socket, int n_centroids, int REQUEST_TIMEOUT){
        double request_data[2] = {GET_CENTROIDS, 1.0 * n_centroids};
        return pollForRequest(socket, request_data,2,
                       Requests::GET_OK,Requests::GET_READY, Requests::GET_DONE,
                       REQUEST_TIMEOUT,&centroid_multipart);
    }

}