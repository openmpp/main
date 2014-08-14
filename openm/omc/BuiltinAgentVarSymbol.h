/**
* @file    BuiltinAgentVarSymbol.h
* Declarations for the BuiltinAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"

using namespace std;

/**
* BuiltinAgentVarSymbol.
*/
class BuiltinAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor by name.
     *
     * @param member_name Name of the member.
     * @param agent       The agent.
     * @param type        The type.
     */
    BuiltinAgentVarSymbol(const string member_name, const Symbol *agent, const Symbol *type)
        : AgentVarSymbol(member_name, agent, type)
    {
    }

    /**
     * Morphing constructor.
     *
     * @param [in,out] sym If non-null, the symbol.
     * @param agent        The agent.
     * @param type         The type.
     */
    BuiltinAgentVarSymbol(Symbol *sym, const Symbol *agent, const Symbol *type)
    : AgentVarSymbol(sym, agent, type, decl_loc)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

};

