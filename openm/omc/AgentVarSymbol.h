/**
* @file    AgentVarSymbol.h
* Declarations for the AgentVarSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentCallbackMemberSymbol.h"

using namespace std;

class AgentVarSymbol : public AgentCallbackMemberSymbol
{
private:
    typedef AgentCallbackMemberSymbol super;

public:
    AgentVarSymbol(Symbol *sym, const Symbol *agent, token_type type)
        : AgentCallbackMemberSymbol(sym, agent, type)
    {
    }

    AgentVarSymbol(const string member_name, const Symbol *agent, token_type type)
        : AgentCallbackMemberSymbol(member_name, agent, type)
    {
    }

    void post_parse(int pass);


    /**
    * Get the c++ declaration code associated with the symbol.
    *
    * @return A CodeBlock.
    */

    virtual CodeBlock cxx_declaration_agent_scope();

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition();

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

};

