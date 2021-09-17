//
// Created by guibertf on 9/16/21.
//

#ifndef UNTITLED_MOCK_RANDOMGENERATOR_H
#define UNTITLED_MOCK_RANDOMGENERATOR_H
#include <random>
#include "gmock/gmock.h"  // Brings in gMock.
#include "RandomGenerator.h"
class mock_RandomGenerator: public RandomGenerator{
public:
    MOCK_METHOD0(CallOp, double());

    mock_RandomGenerator(double d, double d1) : RandomGenerator(d, d1){

    };

    double getRandom(){ return CallOp();}

};


#endif //UNTITLED_MOCK_RANDOMGENERATOR_H
