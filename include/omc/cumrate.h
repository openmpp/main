/**
* @file    cumrate.h
* Declares templates which implement run-time functionality related to cumrate parameters
*
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include <numeric>
#include <algorithm>
#include <functional>
#include <array>

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
        : is_degenerate(false)
        , is_initialized(false)
    {}

    /**
     * Initialize the distribution
     *
     * @param freq The frequency of each value
     */
    void initialize(const double *freq)
    {
        assert(freq); // logic guarantee
        is_degenerate = false;
        is_initialized = false;

        // initialize the permutation vector
        std::iota(value.begin(), value.end(), 0);

        // determine the permutation required to sort the frequencies in descending order, breaking ties using the value of the index
        std::sort(
                value.begin(),
                value.end(),
                [freq](size_t a, size_t b) {
                    return (freq[a] == freq[b])
                            ? (a > b)
                            : (freq[a] > freq[b]);
                }
            );

        // calculate the total of all frequencies
        double total = 0.0;
        for (int i = 0; i < N; ++i) {
            total += freq[i];
        }
 
        // Compute and store the cumulative probabilities
        if (total != 0.0) {
            double running_sum = 0.0;
            for (int i = 0; i < N; ++i) {
                running_sum += (total == 0.0) ? 0.0 : freq[value[i]] / total;
                cumprob[i] = running_sum;
            }
        }
        else {
            // If the distribution is degenerate,
            // set all cumulative probabilities 1.0, which will make draw() always return the first element.
            cumprob.fill(1.0);
            is_degenerate = true;
        }
        is_initialized = true;
    }

    /**
     * Draws an element from the discrete probability distribution
     * 
     * @param uniform A random uniform on (0,1)
     *
     * @return The integer value in {0,...,N-1}
     */
    int draw(double uniform)
    {
        assert(is_initialized); // must be initialized before use
        if (cumprob.front() >= uniform) {
            // check first element directly, for efficiency.
            // Often there is a single dominating probability which occurs frequently
            // and it will always be the first.  Modgen expanded this to do a linear search
            // of a certain length, before falling back to a binary search on the entire distribution.
            return value[0];
        }

        auto it = std::lower_bound(
                   cumprob.begin(),
                   cumprob.end(),
                   uniform
        );
        size_t index = std::distance(cumprob.begin(), it);
        return value[index];
    }

    /**
     * true if distribution is degenerate (all frequencies are zero)
     */
    bool is_degenerate;

    /**
     * The value corresponding to each cumulated probability
     */
    std::array<size_t,N> value;

    /**
     * The cumulated probabilities (partial sums)
     */
    std::array<double,N> cumprob;

    /**
     * true if distribution has been initialized
     */
    bool is_initialized;

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
        : is_initialized(false)
    {}

    /**
     * Initialize the array of M distributions, each of size N
     *
     * @param freq For each of the M distributions, the N frequencies of each value
     */
    void initialize(const double *freq)
    {
        is_initialized = false;
        for (int j = 0; j < M; ++j) {
            distns[j].initialize(freq);
            freq += N;
        }
        is_initialized = true;
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
        assert(is_initialized); // must be initialized before use
        return distns[dist].draw(uniform);
    }

    /**
     * Query if 'dist' is degenerate (all frequencies are zero)
     *
     * @param dist The distance.
     *
     * @return true if degenerate, false if not.
     */
    bool is_degenerate(int dist)
    {
        assert(is_initialized); // must be initialized before use
        return distns[dist].is_degenerate;
    }

private:

    /**
     * The M distributions, each with N values.
     */
    cumrate_distribution<N> distns[M];

    /**
     * true if cumrate has been initialized
     */
    bool is_initialized;
};
