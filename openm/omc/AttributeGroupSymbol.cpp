/**
* @file    AttributeGroupSymbol.cpp
* Definitions for the AttributeGroupSymbol class.
*/
// Copyright (c) 2013-2024 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "AttributeGroupSymbol.h"
#include "LanguageSymbol.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;

void AttributeGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
}

void AttributeGroupSymbol::populate_metadata(openm::MetaModelHolder& metaRows)
{
    using namespace openm;

    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

}
