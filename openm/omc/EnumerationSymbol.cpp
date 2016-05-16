/**
* @file    EnumerationSymbol.cpp
* Definitions for the EnumerationSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EnumerationSymbol.h"
#include "EnumeratorSymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

void EnumerationSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // add this to the complete list of templated types
        pp_all_types1.push_back(this);

        // Add this enumeration to the complete list of enumerations.
        pp_all_enumerations.push_back(this);
        break;
    }
    default:
        break;
    }
}

void EnumerationSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    // Only generate metadata if the enumeration has been marked
    // as being used in a table or external parameter
    if (!metadata_needed) return;

    {
        TypeDicRow typeDic;
        typeDic.typeId = type_id;
        typeDic.name = name;
        typeDic.dicId = dicId;          // see enum kind_of_type for possible values
        typeDic.totalEnumId = pp_size();
        metaRows.typeDic.push_back(typeDic);
    }

    for (auto lang : Symbol::pp_all_languages) {
        TypeDicTxtLangRow typeTxt;
        typeTxt.typeId = type_id;
        typeTxt.langName = lang->name;
        typeTxt.descr = label(*lang);
        typeTxt.note = note(*lang);
        metaRows.typeTxt.push_back(typeTxt);
    }
}

