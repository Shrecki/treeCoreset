//
// Created by guibertf on 5/30/22.
//

#ifndef TREECORESETPROJ_CLIENTMESSAGING_H
#define TREECORESETPROJ_CLIENTMESSAGING_H

#include <zmq.hpp>
#include <vector>

namespace ClientMessaging {
    std::vector<double>* requestCentroids(zmq::socket_t &socket, int n_centroids, int REQUEST_TIMEOUT);
    void requestRepresentatives(zmq::socket_t &socket);
    void requestPutPoint(zmq::socket_t &socket);
}

#endif //TREECORESETPROJ_CLIENTMESSAGING_H
