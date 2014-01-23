/**
* @file    test_partition.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <array>
#include <cmath>
#include "omc/Partition.h"
#include "omc/range_int.h"

namespace mm {

    //partition AGE_GROUP //EN Age group
    //{
    //    0.5,
    //    10,
    //    20,
    //    30
    //};

    // omc-generated code fragments:
    extern const array<real, 5> om_upper_AGE_GROUP = {
        0.5,
        10,
        20,
        30,
        REAL_MAX
    };
    extern const array<real, 5> om_lower_AGE_GROUP = {
        -REAL_MAX,
        0.5,
        10,
        20,
        30
    };
    extern const std::map<real, int> om_splitter_AGE_GROUP = {
        { 0.5, 0 },
        { 10, 1 },
        { 20, 2 },
        { 30, 3 }
    };

    typedef Partition<int, 0, 4, om_lower_AGE_GROUP, om_upper_AGE_GROUP, om_splitter_AGE_GROUP> AGE_GROUP;

    // Tests:



    AGE_GROUP a_age_group = 0;

    void test_partition()
    {
        // test of object creation/initialization
        AGE_GROUP ag1 = 0;

        // test of operator overloading
        ag1++;

        // test of operator overloading
        ag1 = 42;

        // test of arithmetic
        AGE_GROUP ag2 = ag1 - 2;

        // test of compile-time constant array limits for declarations
        double income[AGE_GROUP::size] = { 100, 200, 300, 400, 500 };

        // test of support for ranged-based for
        double sum = 0;
        for (int j : AGE_GROUP::indices()) {
            // test of upper() & lower()
            double lower = AGE_GROUP::lower_bounds()[j];
            double upper = AGE_GROUP::upper_bounds()[j];
            sum += income[j];
        }

        // test of to_index (find interval given value)
        ag1 = AGE_GROUP::to_index(-100);
        ag1 = AGE_GROUP::to_index(0.5);
        ag1 = AGE_GROUP::to_index(9.99999);
        ag1 = AGE_GROUP::to_index(10.00000);
        ag1 = AGE_GROUP::to_index(10.00001);
        ag1 = AGE_GROUP::to_index(10.5);
        ag1 = AGE_GROUP::to_index(25);
        ag1 = AGE_GROUP::to_index(1000);
    }
} // namespace mm
