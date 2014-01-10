/**
* @file    ClassificationLevelSymbol.cpp
* Definitions for the ClassificationLevelSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ClassificationLevelSymbol.h"
#include "ClassificationSymbol.h"

using namespace std;

void ClassificationLevelSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // assign direct pointer to classification for use post-parse
            pp_classification = dynamic_cast<ClassificationSymbol *> (classification);
            assert(pp_classification); // parser guarantee

            // Add this classification level to the classification's list of classification levels
            pp_classification->pp_classification_levels.push_back(this);
        }
        break;
    default:
        break;
    }
}



