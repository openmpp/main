/**
* @file    ClassificationSymbol.cpp
* Definitions for the ClassificationSymbol class.
*/
// Copyright (c) 2013 OpenM++
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
    h += "enum " + name + " {";
    for (auto enumerator : pp_enumerators) {
        h += enumerator->name + ",";
    }
    h += "};";

    return h;
}

