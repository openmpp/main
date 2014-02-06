/**
* @file    AgentVarSymbol.h
* Declarations for the AgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentCallbackMemberSymbol.h"

class AgentFuncSymbol;

using namespace std;

class AgentVarSymbol : public AgentCallbackMemberSymbol
{
private:
    typedef AgentCallbackMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    AgentVarSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentCallbackMemberSymbol(sym, agent, type, decl_loc)
        , side_effects_fn(nullptr)
    {
        create_auxiliary_symbols();
    }

    AgentVarSymbol(const string member_name, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentCallbackMemberSymbol(member_name, agent, type, decl_loc)
        , side_effects_fn(nullptr)
    {
        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this agentvar.
     */

    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();

    /**
    * Get name of member function which implements side-effects
    *
    * Example:  Person::time_side_effects
    *
    * @return  The qualified function name as a string
    */

    const string side_effects_func();

    /**
    * Get declaration of member function which implements side-effects
    *
    * Example:  void time_side_effects(Time old_value, Time new_value)
    * Note that there is no terminating semicolon.
    */

    const string side_effects_decl();

    /**
    * Get qualified declaration of member function which implements side-effects
    *
    * Example:  void Person::time_side_effects(Time old_value, Time new_value)
    * Note that there is no terminating semicolon.
    */

    const string side_effects_decl_qualified();

    /**
    * The lines of C++ code which implement side-effects.
    */

    list<string> pp_side_effects;


    /**
     * The side effects function of the agentvar
     *
     */

    AgentFuncSymbol *side_effects_fn;

};

