//
// Created by guibertf on 5/30/22.
//

#ifndef TREECORESETPROJ_MESSAGING_H
#define TREECORESETPROJ_MESSAGING_H


#include "../clustering/ClusteredPoints.h"

namespace Messaging {
    void handlePostRequest(double *array, int nElems, ClusteredPoints &clusteredPoints, zmq::socket_t &socket);
    void handleGetCentroids(double *array, int nElems, ClusteredPoints &clusteredPoints, zmq::socket_t &socket);
    void handleGetRepresentatives(double *array, int nElems, ClusteredPoints &clusteredPoints, zmq::socket_t &socket, int M);
    void sendResponseFromDoubleArray(zmq::socket_t &socket, int responseSize, double * responseContent, bool hasMore=false);
    void sendSingleMessage(zmq::socket_t &socket, int response);
    void sendException(zmq::socket_t &socket, std::exception &exception);
    double* extractDoubleArrayFromContent(zmq::message_t &msg);
    int getNumberOfDoublesInReq(zmq::message_t &msg);
}


#endif //TREECORESETPROJ_MESSAGING_H
