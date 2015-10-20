/**
* @file    EntityMultilinkSymbol.h
* Declarations for the EntityMultilinkSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "EntityDataMemberSymbol.h"

class LinkAttributeSymbol;
class EntityFuncSymbol;

using namespace std;

/**
* Symbol for a multilink member of an agent.
*/
class EntityMultilinkSymbol : public EntityDataMemberSymbol
{
private:
    typedef EntityDataMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param [in,out] sym Symbol to be morphed.
     * @param agent        The containing agent.
     * @param type         The type of the link in the set, e.g. link<Thing>
     * @param decl_loc     (Optional) the declaration location.
     */
    EntityMultilinkSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : EntityDataMemberSymbol(sym, agent, type, decl_loc)
        , reciprocal_link(nullptr)
        , reciprocal_multilink(nullptr)
        , side_effects_fn(nullptr)
        , insert_fn(nullptr)
        , erase_fn(nullptr)
    {
        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this agentvar.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    /**
     * The reciprocal link if many-to-one
     */
    LinkAttributeSymbol *reciprocal_link;

    /**
     * The reciprocal multilink if many-to-many
     */
    EntityMultilinkSymbol *reciprocal_multilink;

    /**
     * The side effects function of the multilink.
     */
    EntityFuncSymbol *side_effects_fn;

    /**
     * Function which maintains reciprocal link on insert.
     */
    EntityFuncSymbol *insert_fn;

    /**
     * Function which maintains reciprocal link on erase.
     */
    EntityFuncSymbol *erase_fn;
};

