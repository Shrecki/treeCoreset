//
// Created by guibertf on 10/12/21.
//

#ifndef UNTITLED_NETWORKPROTOCOL_H
#define UNTITLED_NETWORKPROTOCOL_H


class NetworkProtocol {
private:
    ClusteredPoints clusteredPoints;
    zmq::socket_t socket;
    zmq::context_t context;
    unsigned int binCapacity;
    unsigned int nBins;
public:


    static double* extractDoubleArrayFromContent(const zmq::message_t &msg);

    static int getNumberOfDoublesInReq(const zmq::message_t &msg);

    void sendResponseFromDoubleArray(int responseSize, double * responseContent);

    int startNetwork();

    int stopNetwork();

    explicit NetworkProtocol(unsigned int binCapacity, unsigned int nBins);

};


#endif //UNTITLED_NETWORKPROTOCOL_H
