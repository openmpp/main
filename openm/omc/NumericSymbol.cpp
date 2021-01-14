/**
* @file    NumericSymbol.cpp
* Definitions for the NumericSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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
        break;
    }
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

NumericSymbol *NumericSymbol::find(token_type type)
{
    auto ns = dynamic_cast<NumericSymbol *>(get_symbol(token_to_string(type)));
    assert(ns); // only called when valid
    return ns;
}

bool NumericSymbol::is_valid_constant(const Constant &k) const
{
    // enumerators are invalid for a numeric type
    if (k.is_enumerator) return false;

    // floating point constants are invalid for non-floating point type
    if (!is_floating() && dynamic_cast<const FloatingPointLiteral *>(k.literal)) return false;

    // TODO (some rainy day) check maximum and minimum values for the numeric type.

    return true;
}

Constant * NumericSymbol::make_constant(const string & i_value) const
{
    if (is_numeric_integer()) return IntegerLiteral::is_valid_literal(i_value.c_str()) ? new Constant(new IntegerLiteral(i_value)) : nullptr;
    if (is_numeric_floating()) return FloatingPointLiteral::is_valid_literal(i_value.c_str()) ? new Constant(new FloatingPointLiteral(i_value)) : nullptr;

    throw HelperException(LT("Validator is not defined for this type of NumericSymbol"));
}

string NumericSymbol::format_for_storage(const Constant &k) const
{
    string result = k.value();

    if (is_floating()) {
        // trim optional floating point suffix if present
        auto ch = tolower(result.back());
        if (ch == 'f' || ch == 'l') result.pop_back();
    }

    return result;
}

