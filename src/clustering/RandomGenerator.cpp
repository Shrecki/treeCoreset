//
// Created by guibertf on 9/16/21.
//

#include "RandomGenerator.h"

RandomGenerator::RandomGenerator(double low, double high): dist(low, high), gen(rd()) {

}

double RandomGenerator::getRandom() {
    return dist(gen);
}
