/**
* @file    StringTypeSymbol.cpp
* Definitions for the StringTypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "StringTypeSymbol.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

bool StringTypeSymbol::is_valid_constant(const Constant &k) const
{
    // only string literals are valid for a string
    if (!k.is_literal) return false;

    return true;
}

void StringTypeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // add this to the complete list of fundamental types
        pp_all_types0.push_back(this);
        break;
    }
    default:
        break;
    }
}

void StringTypeSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    TypeDicRow typeDic;
    typeDic.typeId = type_id;
    typeDic.name = "file";
    typeDic.dicId = kind_of_type::simple_type;          // simple types
    typeDic.totalEnumId = 1;
    metaRows.typeDic.push_back(typeDic);

}

StringTypeSymbol *StringTypeSymbol::find()
{
    auto bs = dynamic_cast<StringTypeSymbol *>(get_symbol("string"));
    assert(bs); // only called when valid
    return bs;
}
