/**
* @file    DeveloperTableSymbol.cpp
* Definitions for the DeveloperTableSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DeveloperTableSymbol.h"

using namespace std;

void DeveloperTableSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        break;
    }
    default:
        break;
    }
}
