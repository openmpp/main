/**
* @file    AgentDataMemberSymbol.h
* Declarations for the AgentDataMemberSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "AgentMemberSymbol.h"


class CodeBlock;

using namespace std;

class AgentDataMemberSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }


    /**
     * Constructor.  Morph a generic symbol to a specialized symbol of this kind.
     *
     * @param [in,out] sym If non-null, the symbol.
     * @param agent        The agent.
     * @param data_type    The type.
     * @param decl_loc     (Optional) the declaration location.
     */

    AgentDataMemberSymbol(Symbol *sym, const Symbol *agent, const Symbol *data_type, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(sym, agent, decl_loc)
        , data_type(data_type->stable_rp())
        , pp_data_type(nullptr)
    {
    }


    /**
     * Constructor.  Create a specialized symbol of this kind with a given name.
     *
     * @param member_name Name of the member.
     * @param agent       The agent.
     * @param data_type   The type.
     * @param decl_loc    (Optional) the declaration location.
     */

    AgentDataMemberSymbol(const string member_name, const Symbol *agent, const Symbol *data_type, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(member_name, agent, decl_loc)
        , data_type(data_type->stable_rp())
        , pp_data_type(nullptr)
    {
        assert(data_type);  // grammar/initialization guarantee
    }


    /**
    * Gets the initial value for the data member.
    *
    * @return The initial value as a string.
    */

    virtual string initial_value() const;

    virtual CodeBlock cxx_initialize_expression() const;


    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();


    void post_parse(int pass);


    /**
    * The datatype of the agent member (reference to pointer)
    *
    */

    Symbol*& data_type;

    /**
    * The datatype of the agent member (pointer)
    *
    */

    TypeSymbol *pp_data_type;

};
