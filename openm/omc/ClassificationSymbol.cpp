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

CodeBlock ClassificationSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += doxygen(name);
    h += "enum om_enum_" + name + " : int {";
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

