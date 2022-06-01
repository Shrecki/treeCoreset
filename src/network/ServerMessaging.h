//
// Created by guibertf on 5/30/22.
//

#ifndef TREECORESETPROJ_SERVERMESSAGING_H
#define TREECORESETPROJ_SERVERMESSAGING_H


#include "../clustering/ClusteredPoints.h"

namespace ServerMessaging {
    void handleStopReq(bool &mustContinue, zmq::socket_t &socket);
    void handlePostRequest(double *array, int nElems, ClusteredPoints &clusteredPoints, zmq::socket_t &socket);
    void handleGetCentroids(double *array, int nElems, ClusteredPoints &clusteredPoints, zmq::socket_t &socket, bool &mustContinue);
    void handleGetRepresentatives(double *array, int nElems, ClusteredPoints &clusteredPoints, zmq::socket_t &socket, int M, bool &mustContinue);
    void sendResponseFromDoubleArray(zmq::socket_t &socket, int responseSize, double * responseContent, bool hasMore=false);
    void sendSingleMessage(zmq::socket_t &socket, int response);
    void sendException(zmq::socket_t &socket, std::exception &exception);

    int getNumberOfDoublesInReq(zmq::message_t &msg);

    void runServer(zmq::socket_t &socket, unsigned int N_SAMPLES, unsigned int M);
}


#endif //TREECORESETPROJ_SERVERMESSAGING_H
