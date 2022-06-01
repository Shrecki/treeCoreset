//
// Created by guibertf on 6/1/22.
//

#include "MessagingUtils.h"

double* extractDoubleArrayFromContent(zmq::message_t &msg){
    // Copy it back to memory
    char * byteArray = new char[msg.size()];
    memcpy(byteArray, msg.data(), msg.size());

    return reinterpret_cast<double*>(byteArray);
}