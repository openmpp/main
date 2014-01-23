/**
* @file    test_range.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <array>
#include <cmath>
#include "omc/Range.h"
#include "omc/range_int.h"

namespace mm {

    //range YEARS //EN Year
    //{
    //    -10,
    //    10
    //};

    // omc-generated code fragments:
    
    typedef Range<int, -3, 4> YEAR;

    // Tests:

    void test_range()
    {
        // test of object creation/initialization
        YEAR y1 = 0;

        // test of operator overloading
        y1++;

        // test of operator overloading
        y1 = 42;
        y1 -= 100;

        // test of arithmetic
        YEAR ag2 = y1 - 2;

        // test of compile-time constant array limits for declarations
        double income[YEAR::size] = { 100, 200, 300, 400, 500, 600, 700, 800 };

        // test of support for ranged-based for
        double sum = 0;
        for (int j : YEAR::indices()) {
            sum += income[j];
        }

        double sum2 = 0;
        for (int y : YEAR::values()) {
            size_t j = YEAR::to_index(y);
            sum2 += income[j];
        }
    }
} // namespace mm
