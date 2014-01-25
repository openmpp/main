/**
* @file    ClassificationSymbol.cpp
* Definitions for the ClassificationSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "ClassificationSymbol.h"
#include "EnumeratorSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

void ClassificationSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateDependencies:
        {
            // Information to compute storage type is known in this pass
            storage_type = optimized_storage_type(0, pp_size() - 1);
        }
        break;
    default:
        break;
    }
}

CodeBlock ClassificationSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += doxygen(name);
    h += "enum om_enum_" + name + " : " + token_to_string(storage_type) + " {";
    bool first = true;
    for (auto enumerator : pp_enumerators) {
        if (first) {
            h += enumerator->name + " = 0," ;
            first = false;
        }
        else {
            h += enumerator->name + "," ;
        }
    }
    h += "};";
    h += "typedef Classification<om_enum_" + name + ", " + to_string(pp_size()) + "> " + name + ";" ;

    return h;
}

