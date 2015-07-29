/**
* @file    SimpleAgentVarEnumSymbol.h
* Declarations for the SimpleAgentVarEnumSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "SimpleAgentVarSymbol.h"
#include "EnumeratorSymbol.h"

using namespace std;


/**
 * SimpleAgentVarEnumSymbol.
 * 
 * A specialization of SimpleAgentVarSymbol with an enumerator initializer.
 */

class SimpleAgentVarEnumSymbol : public SimpleAgentVarSymbol
{
private:
    typedef SimpleAgentVarSymbol super;

public:
    bool is_base_symbol() const { return false; }

    SimpleAgentVarEnumSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, Symbol *enumerator, yy::location decl_loc = yy::location())
        : SimpleAgentVarSymbol(sym, agent, type, nullptr, decl_loc)
        , enumerator(enumerator->stable_rp())
    {
    }

    ~SimpleAgentVarEnumSymbol()
    {
    }

    void post_parse(int pass);

    /**
     * Gets the initial value for the agentvar.
     *
     * @param type_default true to return the default value for the type.
     *
     * @return The initial value as a string.
     */
    string initialization_value(bool type_default) const;

    // members

    /**
     * Reference to pointer to enumerator
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& enumerator;

    /**
     * Direct pointer to enumerator.
     * 
     * Valid post-parse.
     */
    EnumeratorSymbol *pp_enumerator;
};

