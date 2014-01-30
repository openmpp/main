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

    AgentDataMemberSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(sym, agent, decl_loc)
        , type_symbol(type->get_rpSymbol())
        , pp_type_symbol(nullptr)
    {
    }

    AgentDataMemberSymbol(const string member_name, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(member_name, agent, decl_loc)
        , type_symbol(type->get_rpSymbol())
        , pp_type_symbol(nullptr)
    {
        assert(type);  // grammar/initialization guarantee
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
    * Reference to pointer to the type symbol
    *
    * Stable to symbol morhing during parse phase.
    */

    Symbol*& type_symbol;

    /**
    * Direct pointer to the type symbol
    *
    * Set post-parse for convenience.
    */

    TypeSymbol *pp_type_symbol;

};
