/**
* @file    EntityForeignMemberSymbol.cpp
* Definitions for the EntityForeignMemberSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "EntityForeignMemberSymbol.h"
#include "TypeSymbol.h"
#include "EnumerationSymbol.h"
#include "CodeBlock.h"

using namespace std;

CodeBlock EntityForeignMemberSymbol::cxx_initialization_expression(bool type_default) const
{
    CodeBlock c;
    // no initialization of attributes with foreign types
    return c;
}

CodeBlock EntityForeignMemberSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += pp_data_type->name + " " + name + ";";
    return h;
}

void EntityForeignMemberSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
}




