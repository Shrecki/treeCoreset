//
// Created by guibertf on 5/30/22.
//


#include "ServerMessaging.h"

#include "Requests.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "iostream"

namespace ServerMessaging{
    double* extractDoubleArrayFromContent(zmq::message_t &msg){
        // Copy it back to memory
        char * byteArray = new char[msg.size()];
        memcpy(byteArray, msg.data(), msg.size());

        return reinterpret_cast<double*>(byteArray);
    }

    int getNumberOfDoublesInReq(zmq::message_t &msg){
        return msg.size()/sizeof(double);
    }

    void sendResponseFromDoubleArray(zmq::socket_t &socket, int responseSize, double * responseContent, bool hasMore){
        zmq::message_t reply(responseSize*sizeof(double));
        memcpy((void*)reply.data(), (void*)responseContent, responseSize*sizeof(double));
        socket.send(reply, hasMore? ZMQ_SNDMORE : 0); // Issue reply
    }

    void sendException(zmq::socket_t &socket, std::exception &exception) {
        // For now: just send some error signal.
        // Later: send whole exception!
        std::string exception_msg(exception.what());
        zmq::message_t resp(exception_msg.size());
        memcpy((void *) resp.data(), exception_msg.data(), exception_msg.size());
        socket.send(resp, 0);
    }

    void sendSingleMessage(zmq::socket_t &socket, int response){
        zmq::message_t reply(1);
        memcpy((void *) reply.data(), &response, 1);
        socket.send(reply, 0);
    }

    void handlePostRequest(double *array, int nElems, ClusteredPoints &clusteredPoints, zmq::socket_t &socket){
        //std::cout << "Received a POST request." << std::endl;
        // Convert to a point
        Requests response;
        Point* p = nullptr;
        // Convert received data to point
        double * array_target =  &array[1];
        try {
            p = Point::convertArrayToPoint(array_target, nElems-1);
        } catch(std::exception &e){
            if(p!= nullptr){
                p->cleanupData();
                delete p;
                p = nullptr;
            }
            throw;
        }
        //std::cout << "Converting point..." << std::endl;
        if(p != nullptr){
            try {
                //std::cout << " Converted to point of " << nElems -1 << " elements." << std::endl;
                Eigen::VectorXd vec = p->getData();
                for (int i = 0; i < vec.size(); ++i) {
                    if (std::isnan(vec(i))) {
                        throw std::invalid_argument(
                                "Point argument contains NaNs. Please remove the NaNs before passing any point");
                    }
                }
                assert(p->getData().array().isNaN().sum() == 0);

            } catch(std::exception &e) {
                p->cleanupData();
                delete p;
                p = nullptr;
                throw;
            }
            // Perform insertion (this is the InsertPoint step in the stream)
            //std::cout << "Convert OK" << std::endl;
            try {
                clusteredPoints.insertPoint(p);
                response = Requests::POST_OK;
                sendSingleMessage(socket, response);

            } catch(std::exception &e){
                delete p;
                p = nullptr;
                throw;
            }

            //std::cout << "Insert went ok" << std::endl;

            // Send back response: all went well
                //std::cout << "Response sent " << std::endl;
                //std::cout << "Post OK" << std::endl;
        }
    }

    void sendSeveralPoints(zmq::socket_t &socket, const std::vector<double> &data, int n_points, int point_dimensionality, bool &mustContinue){
        double okResponse[3] = {Requests::GET_OK, (double)n_points, 1.0 * point_dimensionality};
        double * reqQuery = nullptr;
        zmq::message_t request;

        sendResponseFromDoubleArray(socket, 3, okResponse, false);

        // The protocol works as follows:
        // When send is ready, the server first issues 'GET_OK' with:
        //    - number of points to transmit
        //    - dimensionality of each point
        // It then expects to receive from the client a 'POST_OK' signaling that it is ready to receive the points.
        // If this goes as expected, then it will simply issue the points one by one as an n_points-part message.
        // The last N+1 message will be a 'GET_OK' to signal the end.
        socket.recv(&request, 0);
        reqQuery = extractDoubleArrayFromContent(request);

        if(reqQuery[0] == GET_READY) {
            // Transmit all clusters, each as a one-part message
            for (int i = 0; i < n_points; ++i) {
                double currClusterVector[point_dimensionality];
                for (int j = 0; j < point_dimensionality; ++j) {
                    currClusterVector[j] = data.at(i * point_dimensionality + j);
                }
                sendResponseFromDoubleArray(socket, point_dimensionality, currClusterVector, true);
            }

            double doneMessage[1] = {Requests::GET_DONE};
            sendResponseFromDoubleArray(socket, 1, doneMessage, false);
        } else {
            if(reqQuery[0] == STOP_REQ){
                handleStopReq(mustContinue, socket);
            } else {
                double errorResponse[1] = {Requests::ERROR};
                sendResponseFromDoubleArray(socket, 1, errorResponse, false);
            }
        }
        delete [] reqQuery;
        reqQuery = nullptr;
    }

    // Representatives and clusters work on a similar premise, in that they send several points without expecting a ping-pong
    // between every send. It means that a single function can handle both.
    void handleGetCentroids(double *array, int nElems, ClusteredPoints &clusteredPoints, zmq::socket_t &socket, bool &mustContinue){
        std::cout << "Received a GET request." << std::endl;
        zmq::message_t request;

        if(nElems < 2 ){
            throw std::invalid_argument("Malformed request. Should contain number of centroids as second argument in "
                                        "request payload, but instead only contained GET_CENTROIDS request and no "
                                        "centroid indication.");
        }

        std::vector<double> data;
        try {
            int k = (int)array[1];

            // Prepare clusters for transmission
            std::cout << "Expecting " << std::to_string(k) << " centroids" << std::endl;
            clusteredPoints.getClustersAsFlattenedArray(data, k, 100);
            int dim = data.size()/k;
            std::cout << dim << std::endl;
            sendSeveralPoints(socket,data,k,dim, mustContinue);
        } catch (std::exception &e){
            throw;
        }
    }

    void handleGetRepresentatives(double *array, int nElems, ClusteredPoints &clusteredPoints, zmq::socket_t &socket, int M, bool &mustContinue){
        zmq::message_t request;
        std::vector<double> data;
        try {
            clusteredPoints.performUnionCoresetAndGetRepresentativesAsFlattenedArray(data);
            int dim = clusteredPoints.getDimension();
            sendSeveralPoints(socket,data,data.size()/dim,dim, mustContinue);
        } catch (std::exception &e){
            throw;
        }
    }

    void handleStopReq(bool &mustContinue, zmq::socket_t &socket){
        std::cout << "Received a STOP request." << std::endl;
        // Send back response: we know we got the stop
        mustContinue = false;
        zmq::message_t reply(1*sizeof(double));
        double stop_val = Requests::STOP_OK;
        memcpy((void *) reply.data(), (void*)&stop_val, 1*sizeof(double));
        socket.send(reply);
    }

    void runServer(zmq::socket_t &socket, unsigned int N_SAMPLES, unsigned int M) {
        std::cout << "Started server!" << std::endl;
        ClusteredPoints clusteredPoints(ceil(log2(N_SAMPLES/M)+2), M);

        bool mustContinue(true);
        double *array = nullptr;
        while(mustContinue){
            // Await a request
            zmq::message_t request;
            socket.recv (&request, 0);
            int nElems(ServerMessaging::getNumberOfDoublesInReq(request));
            array = ServerMessaging::extractDoubleArrayFromContent(request);
            try{
                switch((int)array[0]){
                    case Requests::POST_REQ : {
                        ServerMessaging::handlePostRequest(array, nElems, clusteredPoints, socket);
                        break;
                    }
                    case Requests::GET_CENTROIDS : {
                        ServerMessaging::handleGetCentroids(array, nElems, clusteredPoints, socket, mustContinue);
                        break;
                    }
                    case Requests::GET_REPS: {
                        ServerMessaging::handleGetRepresentatives(array, nElems, clusteredPoints, socket, M, mustContinue);
                        break;
                    }/*
                    case Requests::LOAD_REQ: {
                        std::cout << "Received a LOAD request." << std::endl;
                        zmq::message_t reply(5);
                        memcpy((void *) reply.data(), "World", 5);
                        socket.send(reply);
                        break;
                    }
                    case Requests::SAVE_REQ: {
                        std::cout << "Received a SAVE request." << std::endl;
                        zmq::message_t reply(5);
                        memcpy((void *) reply.data(), "World", 5);
                        socket.send(reply);
                        break;
                    }*/
                    case Requests::STOP_REQ: {
                        handleStopReq(mustContinue, socket);
                        break;
                    }/*
                    case Requests::POST_OK: {
                        std::cout << "Simulating big CPU overload." << std::endl;
                        sleep(3);
                        zmq::message_t reply(5);
                        memcpy((void *) reply.data(), "World", 5);
                        socket.send(reply);
                        break;
                    }*/
                    default: {
                        std::cout << "Request not implemented." << std::endl;
                        std::string str_error("Request not implemented.");
                        zmq::message_t reply(str_error.size());
                        memcpy((void *) reply.data(), str_error.data(), str_error.size());
                        socket.send(reply);
                        break;
                    }
                }
            } catch(std::exception &e){
                //std::cout << e.what() << std::endl;
                sendException(socket, e);
            }
            delete [] array;
            array = nullptr;
        }
    }
}

