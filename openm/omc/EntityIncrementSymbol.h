/**
* @file    EntityIncrementSymbol.h
* Declarations for the EntityIncrementSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "AgentMemberSymbol.h"
#include "EntityTableSymbol.h"


class CodeBlock;

using namespace std;

/**
 * Functionality associated with the entity member Increment<> associated with a table
 */
class EntityIncrementSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.  Create a specialized symbol of this kind with a given name.
     *
     * @param member_name Name of the member.
     * @param agent       The agent.
     * @param data_type   The type.
     * @param decl_loc    (Optional) the declaration location.
     */
    EntityIncrementSymbol(const string member_name, const Symbol *agent, const EntityTableSymbol *table, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(member_name, agent, decl_loc)
        , table(table)
    {
    }

    string pretty_name() const
    {
        return "Increment for table " + table->name;
    }


    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();

    void post_parse(int pass);

    /**
     * The table for this increment.
     */
    const EntityTableSymbol *table;
};
