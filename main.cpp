#include <iostream>
#include <zmq.hpp>
#include <string>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif


int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:5555");

    std::string resp("World");
    zmq::const_buffer message((void*)&resp,5);
    //socket.send(message, static_cast<zmq::send_flags>(0));

    while(true){
        zmq::message_t request;
        socket.recv (&request);
        // This code allows to read pointer addresses
        int nElems(request.size());
        char byteArray[nElems];
        memcpy(byteArray, request.data(), nElems);
        //std::string str(byteArray, nElems);
        //std::cout << str << std::endl;
        double* array = reinterpret_cast<double*>(byteArray);
        for(int i=0; i<100;++i){
            std::cout << array[i] << std::endl;

        }


        // Do main algorithmic work here

        zmq::message_t reply(5);
        memcpy((void *) reply.data(), "World", 5);
        socket.send(reply);
    }
    socket.close();
    return 0;
}
