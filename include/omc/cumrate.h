/**
* @file    cumrate.h
* Declares templates which implement run-time functionality related to cumrate parameters
*
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <numeric>
#include <algorithm>
#include <array>

using namespace std;

/**
 * A finite discrete distribution.
 * 
 * The algorithm produces the same results as the Modgen lookup functions for cumrate parameters.
 * See Modgen release notes for version 7.0.1.9
 *
 * @tparam N Number of values.
 */
template<int N>
class cumrate_distribution
{
public:
    cumrate_distribution()
    {}

    /**
     * Initialize the distribution
     *
     * @param freq The frequency of each value
     */
    void initialize(const array<double, N> freq)
    {
        // transform the frequencies to probabilities
        double total = accumulate(freq.begin(), freq.end(), 0);
        transform(freq.begin(),
                  freq.end(),
                  cumprob.begin(),
                  [total](double &f) {
            return (total == 0.0) ? 0.0 : f / total;
        }
        );

        // continue initialization
        initialize2();
    }

    /**
     * Initialize the distribution
     *
     * @param freq The frequency of each value
     */
    void initialize(double * freq)
    {
        assert(freq); // logic guarantee

        double total = 0.0;
        for (int i = 0; i < N; ++i) {
            total += freq[i];
        }
        for (int i = 0; i < N; ++i) {
            cumprob[i] = (total == 0.0) ? 0.0 : freq[i] / total;
        }

        // continue initialization
        initialize2();
    }

    /**
     * Draws an element from the discrete probability distribution
     * 
     * Note that the argument *this accesses the functor for the class, defined by overloading ().
     *
     * @param uniform A random uniform on (0,1)
     *
     * @return The integer value in {0,...,N-1}
     */
    int draw(double uniform)
    {
        auto it = lower_bound(
                   index.begin(),
                   index.end(),
                   uniform,
                   *this
        );
        return *it;
    }

    /**
     * Comparison function for std::sort and std::lower_bound
     * 
     * The comparison function needs to access class data.  The solution used here is to overload (),
     * which makes the comparison function a functor of the class itself. That means that *this can
     * be used to supply the comparison function to std::sort and std::lower_bound, which can then
     * access the class data.
     * 
     * The comparison function sorts in decreasing order.  Ties in values are broken explicitly
     * by comparing indices, in decreasing order.
     *
     * @return The result of the operation.
     */
    bool operator()(const int &a, const int &b)
    {
        return (cumprob[a] == cumprob[b])
                ? (a > b)
                : (cumprob[a] > cumprob[b]);
    }

    /**
     * Initialize the distribution (part 2)
     */
    void initialize2() {

        // initialize the permutation vector
        iota(index.begin(), index.end(), 0);

        // sort the probabilities in descending order, breaking ties using index
        // *this accesses the functor for the class, defined by overloading ().
        sort(index.begin(), index.end(), *this);

        // cumulate the probabilities into partial sums
        double sum = 0.0;
        for(auto j : index) {
            sum += cumprob[j];
            cumprob[j] = sum;
        }
    }

    /**
     * The permutation vector
     */
    array<int,N> index;

    /**
     * The cumulated probabilities (partial sums)
     */
    array<double,N> cumprob;
};

/**
 * Encapsulates an array of finite discrete distributions.
 * 
 * @tparam M Number of distributions.
 * @tparam N Number of values in each distribution.
 */
template<int M, int N>
class cumrate
{
public:
    cumrate()
    {}

    /**
     * Initialize the array of M distributions, each of size N
     *
     * @param freq For each of the M distributions, the N frequencies of each value
     */
    void initialize(double * freq)
    {
        for (int j = 0; j < M; ++j) {
            distns[j].initialize(freq);
            freq += N;
        }
    }

    /**
     * Draws from one of the M distributions
     *
     * @param dist    The distribution from which to draw.
     * @param uniform A random uniform on (0,1) used to draw.
     *
     * @return An int.
     */
    int draw(int dist, double uniform)
    {
        return distns[dist].draw(uniform);
    }

private:

    /**
     * The M distributions, each with N values.
     */
    cumrate_distribution<N> distns[M];
};
