/**
* @file    test_partition.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <array>
#include <cmath>
#include "om_types0.h"
#include "omc/Partition.h"
#include "omc/integer_counter.h"

namespace mm {

    //partition AGE_GROUP //EN Age group
    //{
    //    0.5,
    //    10,
    //    20,
    //    30
    //};

    // omc-generated code fragments:

    // play with storage type for testing
    typedef uchar storage;

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
    extern const std::map<real, storage> om_splitter_AGE_GROUP = {
        { 0.5, 0 },
        { 10, 1 },
        { 20, 2 },
        { 30, 3 }
    };

    typedef Partition<storage, 5, om_lower_AGE_GROUP, om_upper_AGE_GROUP, om_splitter_AGE_GROUP> AGE_GROUP;

    // Tests:

    AGE_GROUP a_age_group = 0;

    void test_partition()
    {
        // test of object creation/initialization
        AGE_GROUP ag1 = 0;

        // test of operator overloading
        ag1++;
        assert(ag1 == 1);

        // test of value protection
        ag1 = 42;
        assert(ag1 == 4);

        // test of value protection
        ag1 -= 100;
        assert(ag1 == 0);

        // test of arithmetic
        AGE_GROUP ag2 = ag1 + 2;
        assert(ag2 == 2);

        // test of compile-time constant array limits for declarations
        double income[AGE_GROUP::size] = { 100, 200, 300, 400, 500 };

        // test of support for ranged-based for
        double sum1 = 0;
        for (int j : AGE_GROUP::indices()) {
            // test of upper() & lower()
            real lower = AGE_GROUP::lower_bounds()[j];
            real upper = AGE_GROUP::upper_bounds()[j];
            sum1 += income[j];
        }
        assert(sum1 == 1500);

        // test of support for ranged-based for
        double sum2 = 0;
        for (int j : AGE_GROUP::indices()) {
            AGE_GROUP age_grp;
            age_grp = j;
            // test of upper() & lower()
            real lower = age_grp.lower();
            real upper = age_grp.upper();
            real width = age_grp.width();
            sum2 += income[j];
        }
        assert(sum2 == 1500);

        // test of to_index (find interval given value)
        ag1 = AGE_GROUP::to_index(-100);
        assert(ag1 == 0);
        ag1 = AGE_GROUP::to_index(0.5);
        assert(ag1 == 1);
        ag1 = AGE_GROUP::to_index(9.99999);
        assert(ag1 == 1);
        ag1 = AGE_GROUP::to_index(10.00000);
        assert(ag1 == 2);
        ag1 = AGE_GROUP::to_index(10.00001);
        assert(ag1 == 2);
        ag1 = AGE_GROUP::to_index(10.5);
        assert(ag1 == 2);
        ag1 = AGE_GROUP::to_index(25);
        assert(ag1 == 3);
        ag1 = AGE_GROUP::to_index(1000);
        assert(ag1 == 4);
    }
} // namespace mm
