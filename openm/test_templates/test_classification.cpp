/**
* @file    test_classification.cpp
* Used for development testing of the run-time support classes
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <array>
#include <cmath>
#include "omc/Classification.h"

namespace mm {

    //classification SEX //EN Sex
    //{
    //    //EN Female
    //    FEMALE,
    //
    //    //EN Male
    //    MALE
    //};

    //classification REGION //EN Region
    //{
    //    //EN East
    //    EAST,
    //
    //    //EN West
    //    WEST
    //};


    // omc-generated code fragments:
    
    enum om_enum_SEX {
        FEMALE = 0,
        MALE,
    };
    typedef Classification<om_enum_SEX, 2> SEX;

    enum om_enum_REGION {
        EAST = 0,
        WEST,
    };
    typedef Classification<om_enum_REGION, 2> REGION;

    // Tests:

    void test_classification()
    {
        // test of object creation/initialization
        SEX s1 = MALE;
        REGION r1(WEST);
        r1 = EAST;

        // Following gives compiler error as desired.
        //r1 = MALE;

        // test of operator overloading
        s1++;

        // test of storage integrity & unsigned underflow protection
        s1--;
        s1--;
        s1--;
        s1--;

        // test of compile-time constant array limits for declarations
        double income[REGION::size] = { 100, 200 };

        // test of support for ranged-based for
        double sum = 0;
        for (int j : REGION::indices()) {
            sum += income[j];
        }
    }
} // namespace mm
