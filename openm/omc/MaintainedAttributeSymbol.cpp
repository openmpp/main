/**
* @file    MaintainedAttributeSymbol.cpp
* Definitions for the MaintainedAttributeSymbol class.
*/
// Copyright (c) 2013-2017 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "MaintainedAttributeSymbol.h"
#include "EntitySymbol.h"
#include "IdentityAttributeSymbol.h"
#include "DerivedAttributeSymbol.h"
#include "MultilinkAttributeSymbol.h"
#include "CodeBlock.h"

using namespace std;

bool MaintainedAttributeSymbol::is_identity(void)
{
    return dynamic_cast<IdentityAttributeSymbol *>(this);
}

bool MaintainedAttributeSymbol::is_derived(void)
{
    return dynamic_cast<DerivedAttributeSymbol*>(this);
}

bool MaintainedAttributeSymbol::is_multilink_aggregate(void)
{
    return dynamic_cast<MultilinkAttributeSymbol*>(this);
}

CodeBlock MaintainedAttributeSymbol::cxx_declaration_entity()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_entity();

    // Perform operations specific to this level in the Symbol hierarchy.

    return h;
}

CodeBlock MaintainedAttributeSymbol::cxx_definition_entity()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_entity();

    // Perform operations specific to this level in the Symbol hierarchy.

    return c;
}

void MaintainedAttributeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // Add this attribute to the entity's list of all maintained aattributes
        pp_entity->pp_maintained_attributes.push_back(this);
        break;
    }
    default:
        break;
    }
}


