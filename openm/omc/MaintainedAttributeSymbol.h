/**
* @file    MaintainedAttributeSymbol.h
* Declarations for the MaintainedAttributeSymbol class.
*/
// Copyright (c) 2013-2017 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "AttributeSymbol.h"

using namespace std;

/**
* MaintainedAttributeSymbol
*
* Symbol for attributes whose values are automatically maintained
* based on changes in other attributes.
*/
class MaintainedAttributeSymbol : public AttributeSymbol
{
private:
    typedef AttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Morphing constructor
     *
     * @param [in,out] sym If non-null, the symbol.
     * @param agent        The agent.
     * @param type         The type.
     * @param decl_loc     (Optional) the declaration location.
     */
    MaintainedAttributeSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, omc::location decl_loc = omc::location())
        : AttributeSymbol(sym, agent, type, decl_loc)
    {
    }

    /**
     * Constructor by name
     *
     * @param member_name Name of the member.
     * @param agent       The agent.
     * @param type        The type.
     * @param decl_loc    (Optional) the declaration location.
     */
    MaintainedAttributeSymbol(const string member_name, const Symbol *agent, const Symbol *type, omc::location decl_loc = omc::location())
        : AttributeSymbol(member_name, agent, type, decl_loc)
    {
    }

    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();

    void post_parse(int pass);

    /**
     * The attributes on which this maintained attribute depends.
     */
    unordered_set<AttributeSymbol *> pp_dependent_attributes;

    /**
     * The attributes whose time-like status propagates to this attribute.
     */
    unordered_set<AttributeSymbol*> pp_dependent_attributes_timelike_propagating;
};

