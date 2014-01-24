/**
* @file    test_range.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "om_types0.h"
#include "omc/Range.h"
#include "omc/range_int.h"

namespace mm {

    //range YEARS //EN Year
    //{
    //    -10,
    //    10
    //};

    // omc-generated code fragments:
    
    // play with storage type for testing
    typedef int storage;

    typedef Range<storage, -3, 4> YEAR;

    // Tests:

    void test_range()
    {
        // test of object creation/initialization
        YEAR y1 = 0;

        // test of operator overloading
        y1++;
        assert(y1 == 1);

        // test of operator overloading & integrity
        y1 = 42;
        assert(y1 == 4);
        y1 -= 100;
        assert(y1 == -3);

        // test of arithmetic
        YEAR y2 = y1 + 2;
        assert(y2 == -1);

        // test of compile-time constant array limits for declarations
        double income[YEAR::size] = { 100, 200, 300, 400, 500, 600, 700, 800 };

        // test of support for ranged-based for
        double sum1 = 0;
        for (int j : YEAR::indices()) {
            sum1 += income[j];
        }
        assert(sum1 == 3600);

        double sum2 = 0;
        for (int y : YEAR::values()) {
            size_t j = YEAR::to_index(y);
            sum2 += income[j];
        }
        assert(sum2 == 3600);
    }
} // namespace mm
