//
// Created by guibertf on 5/30/22.
//

#ifndef TREECORESETPROJ_CLIENTMESSAGING_H
#define TREECORESETPROJ_CLIENTMESSAGING_H

#include <zmq.hpp>
#include <vector>

namespace ClientMessaging {
    std::vector<std::vector<double>*>* requestCentroids(zmq::socket_t &socket, unsigned int n_centroids, int REQUEST_TIMEOUT);
    std::vector<std::vector<double>*>*  requestRepresentatives(zmq::socket_t &socket, int REQUEST_TIMEOUT);
    void requestPutPoint(zmq::socket_t &socket, double *point_data, unsigned int dimension, int REQUEST_TIMEOUT);
    void requestStop(zmq::socket_t &socket);
}

#endif //TREECORESETPROJ_CLIENTMESSAGING_H
