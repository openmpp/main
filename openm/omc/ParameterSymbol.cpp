/**
* @file    ParameterSymbol.h
* Definitions for the ParameterSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "ParameterSymbol.h"

using namespace std;

void ParameterSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // add this parameter to the complete list of parameters
        pp_parameters.push_back(this);
        break;
    default:
        break;
    }
}
