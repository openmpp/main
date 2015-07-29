/**
* @file    TableGroupSymbol.cpp
* Definitions for the TableGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "TableGroupSymbol.h"

using namespace std;

void TableGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // add this to the complete list of table groups
        pp_all_table_groups.push_back(this);
        break;
    }
    default:
        break;
    }
}
