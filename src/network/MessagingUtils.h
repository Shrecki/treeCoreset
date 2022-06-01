//
// Created by guibertf on 6/1/22.
//

#ifndef TREECORESETPROJ_MESSAGINGUTILS_H
#define TREECORESETPROJ_MESSAGINGUTILS_H

#include <zmq.hpp>
double* extractDoubleArrayFromContent(zmq::message_t &msg);


#endif //TREECORESETPROJ_MESSAGINGUTILS_H
