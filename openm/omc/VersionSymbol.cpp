/**
* @file    VersionSymbol.cpp
* Definitions for the VersionSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "VersionSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/modelBuilder.h"

using namespace std;
using namespace openm;


void VersionSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    metaRows.modelDic.version = formatted_value();
};
