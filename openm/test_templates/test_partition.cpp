/**
* @file    test_partition.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <array>
#include "omc/Partition.h"
#include "omc/range_int.h"

namespace mm {

    extern const array<real, 4> om_cutpoints_AGE_GROUP = { 0.5, 10, 20, 30 };
    extern const set<real> om_splitter_AGE_GROUP = { 0.5, 10, 20, 30 };

    typedef Partition<int, 0, 5, om_splitter_AGE_GROUP> AGE_GROUP;

    AGE_GROUP a_age_group = 0;

    void test_partition()
    {
        AGE_GROUP ag1 = 0;

        double income[AGE_GROUP::size] = { 100, 200, 300, 400, 500 };
        double sum = 0;
        for (int j : AGE_GROUP::indices) {
            sum += income[j];
        }
    }
} // namespace mm
