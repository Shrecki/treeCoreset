//
// Created by guibertf on 9/17/21.
//
#include <bits/stdc++.h>
#include <vector>
#include <boost/math/distributions/chi_squared.hpp>
#include "math_framework.h"
using namespace std;
namespace statistics {

    /**
     * @brief Given an unsigned integer n, return all primes in interval [1, n] using Sieve of Eratosthenes
     *
     * @param n Unsigned integer denoting the upper integer up to which one wishes to find prime numbers
     *
     * @return primes Array of primes in the interval [1, n]
     */
    std::vector<unsigned int> sieve(unsigned int n){
         bool prime[n+1];
         std::memset(prime, true, sizeof(prime));
         std::vector<unsigned int> primes;

         for(int p=2; p*p <=n; p++){
             if(prime[p]){
                 primes.push_back(p);
                 // Starting from square of p, mark all its multiples as being not prime
                 // Smaller multiples are marked by previous p (e.g : 10 is eliminated as being a multiple of 2 before being a multiple of 5)
                 for(int i= p*p; i <=n; i+=p){
                     prime[i] = false;
                 }
             }
         }
         return primes;
    }

    /**
     * Returns p-value of chi-square test between observed frequencies and expected frequencies.
     * Starts with an initial number of DOF, and remove one DOF for each bin that has an expected frequency of 0.
     * @param observedFrequencies
     * @param expectedFrequencies
     * @param nBins
     * @param degreesFreedom
     * @return
     */
    double chiSquareTest(int observedFrequencies[], int expectedFrequencies[], int nBins, int degreesFreedom){
        double count(0.0), diff(0.0);
        for (int i=0; i < nBins; ++i){
            if(expectedFrequencies[i] != 0.0){
                diff=observedFrequencies[i]-expectedFrequencies[i];
                count += 1.0*(diff*diff)/expectedFrequencies[i];
            } else {
                degreesFreedom -=1;
            }
        }

        boost::math::chi_squared_distribution<double> dist(degreesFreedom);
        double pValue = 1.0 - boost::math::cdf(dist, count);
        return pValue;
    }
}