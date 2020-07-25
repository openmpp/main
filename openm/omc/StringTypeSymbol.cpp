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

Constant * StringTypeSymbol::make_constant(const string & i_value) const
{
    return StringLiteral::is_valid_literal(i_value.c_str()) ? new Constant(new StringLiteral(i_value)) : nullptr;
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


string StringTypeSymbol::format_for_storage(const Constant &k) const
{
    string lit = k.value(); // The constant as a C++ string literal

    // string constants may be a C++ string literals
    // e.g. "C:\\MyFolder\\TheFile.dat"
    // including surrounding double quotes and internal C++ escape sequences,
    // such as \\ for \.
    // For DB storage, the quotes need to be removed, and the escapes expanded.

    assert(is_string()); // logic guarantee

    if (lit.length() >= 2 && lit[0] == '"' && lit[lit.length() - 1] == '"') {   // remove "quotes"
        lit = lit.substr(1, lit.length() - 2);
    }

    string result;
    bool in_escape = false;
    for (auto ch : lit) {
        if (in_escape) {
            switch (ch) {
            case '"':
            case '\'':
            case '\\':
                break;
            case 'n':
                ch = '\n';
                break;
            case 't':
                ch = '\t';
                break;
            case 'r':
                ch = '\r';
                break;
            case 'b':
                ch = '\b';
                break;
            case 'f':
                ch = '\f';
                break;
            default:
                break;
            }
            in_escape = false;
        }
        else {
            if (ch == '\\') {
                // start an escape sequence
                in_escape = true;
                continue;
            }
        }
        result += ch;
    }

    return result;
}

