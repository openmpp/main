/**
* @file    NumericSymbol.cpp
* Definitions for the NumericSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "NumericSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

void NumericSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // add this to the complete list of fundamental types
            pp_all_types0.push_back(this);
        }
        break;
    default:
        break;
    }
}

CodeBlock NumericSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    string typedef_string;
    if (!keywords.empty()) {
        for (auto tk : keywords) {
            typedef_string += " " + token_to_string(tk);
        }
        h += "typedef" + typedef_string + " " + name + ";";
    }

    return h;
}

void NumericSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    TypeDicRow typeDic;
    typeDic.typeId = type_id;
    typeDic.name = name;
    typeDic.dicId = kind_of_type::simple_type;          // simple types
    typeDic.totalEnumId = 1;
    metaRows.typeDic.push_back(typeDic);

}

NumericSymbol *NumericSymbol::get_typedef_symbol(token_type type)
{
    return dynamic_cast<NumericSymbol *>(get_symbol(token_to_string(type)));
}
