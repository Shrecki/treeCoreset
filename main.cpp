#include <zmq.hpp>
#include "src/network/ServerMessaging.h"

#define N_SAMPLES 728*14.0
#define M 132


int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:5555");

    ServerMessaging::runServer(socket, N_SAMPLES, M);

    socket.close();
    context.close();
    return 0;
}
