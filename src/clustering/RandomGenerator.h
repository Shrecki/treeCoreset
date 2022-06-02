//
// Created by guibertf on 9/16/21.
//

#ifndef UNTITLED_RANDOMGENERATOR_H
#define UNTITLED_RANDOMGENERATOR_H

#include <random>
class RandomGenerator {
private:
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen;
    std::uniform_real_distribution<> dist;

public:
    RandomGenerator(double low, double high);

    virtual double getRandom();
};


#endif //UNTITLED_RANDOMGENERATOR_H
