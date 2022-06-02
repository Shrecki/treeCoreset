//
// Created by guibertf on 9/17/21.
//

#ifndef UNTITLED_MATH_FRAMEWORK_H
#define UNTITLED_MATH_FRAMEWORK_H

#endif //UNTITLED_MATH_FRAMEWORK_H


using namespace std;
namespace statistics {

    /**
     * @brief Given an unsigned integer n, return all primes in interval [1, n] using Sieve of Eratosthenes
     *
     * @param n Unsigned integer denoting the upper integer up to which one wishes to find prime numbers
     *
     * @return primes Array of primes in the interval [1, n]
     */
    std::vector<unsigned int> sieve(unsigned int n);

    double chiSquareTest(int observedFrequencies[], int expectedFrequencies[], int nBins, int degreesFreedom);
}