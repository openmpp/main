/**
* @file    SimpleAgentVarSymbol.h
* Declarations for the SimpleAgentVarSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentVarSymbol.h"

using namespace std;


/**
 * SimpleAgentVarSymbol.
 * 
 * A symbol of this class represents an agentvar whose value can be modified by C++ code in an
 * event implement function.
 */

class SimpleAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    bool is_base_symbol() const { return false; }

    SimpleAgentVarSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, Literal *initializer, yy::location decl_loc = yy::location())
        : AgentVarSymbol(sym, agent, type, decl_loc)
        , initializer(initializer)
    {
    }

    ~SimpleAgentVarSymbol()
    {
    }


    /**
     * Gets the initial value for the agentvar.
     *
     * @return The initial value as a string.
     */

    string initialization_value(bool type_default) const;

    CodeBlock cxx_declaration_agent();

    // members


    /**
     * The initializer.
     *
     *  C++ literal to initialize simple agentvar at agent creation.
     */

    Literal *initializer;

};

