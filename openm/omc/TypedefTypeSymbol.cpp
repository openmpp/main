/**
* @file    TypedefTypeSymbol.cpp
* Definitions for the TypedefTypeSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TypedefTypeSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

CodeBlock TypedefTypeSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    string typedef_string;
    if (!keywords.empty()) {
        for (auto tk : keywords) {
            typedef_string += " " + token_to_string(tk);
        }
        h += "typedef " + typedef_string + " " + name + ";";
    }

    if ( type == token::TK_Time ) {
        h += "typedef " + typedef_string + " TIME; // for Modgen models";
        // Time 'literals'
        h += "extern const Time time_infinite;";
        h += "extern const Time TIME_INFINITE; // for Modgen models";
        h += "";
    }
    return h;
}

CodeBlock TypedefTypeSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
 
    if (type == token::TK_Time) {
        // Time 'literals'
        c += "const Time time_infinite = 32767;";
        c += "const Time TIME_INFINITE = 32767; // for Modgen models";
        c += "";
    }
    return c;
}

void TypedefTypeSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    TypeDicRow typeDic;
    typeDic.typeId = type_id;
    typeDic.name = name;
    typeDic.dicId = 0;          // simple types
    typeDic.totalEnumId = 1;
    metaRows.typeDic.push_back(typeDic);

}

TypedefTypeSymbol *TypedefTypeSymbol::get_typedef_symbol(token_type type)
{
    return dynamic_cast<TypedefTypeSymbol *>(get_symbol(token_to_string(type)));
}
