/**
* @file    TableAnalysisAgentVarSymbol.h
* Declarations for the TableAnalysisAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <string>
#include "Symbol.h"

class AgentVarSymbol;
class CodeBlock;

using namespace std;

/**
* TableAnalysisAgentVarSymbol.
*
* Represents an agentvar used in the analysis dimension
* (also known as the expression dimension) of a table.
* An agentvar can be used multiple times in the analysis dimension of a table.
* If so, it is represented by a single instance of this class.
* This class handles the creation of an associated agentvar which holds the
* 'in' value of the agentvar when a table increment is started.
*/

class TableAnalysisAgentVarSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    TableAnalysisAgentVarSymbol(Symbol *table, Symbol *agentvar, int index)
        : Symbol(symbol_name(table, agentvar))
        , table(table->get_rpSymbol())
        , agentvar(agentvar->get_rpSymbol())
        , index(index)
        , need_value_in(false)
        , pp_table(nullptr)
        , pp_agentvar(nullptr)
    {
    }

    /**
    * Get the unique name for a specified symbol of this kind.
    *
    * @param   table       The table.
    * @param   agentvar    The agentvar used in the analysis dimension
    *
    * @return  The symbol name as a string.
    *          Example: om_taav_DurationOfLife_alive.
    */

    static string symbol_name(const Symbol *table, const Symbol *agentvar);

    /**
    * Check for existence of a specific table analysis agentvar.
    *
    * @param   table       The table.
    * @param   agentvar    The agentvar.
    *
    * @return  true if found, else false.
    */

    static bool exists(const Symbol *table, const Symbol *agentvar);

    void post_parse(int pass);

    /**
    * Gets the agentvar name for the 'in' value.
    *
    * This is the name of the data member which holds the
    * "in" value of the agentvar.  It holds the value of the agentvar
    * at the start of an increment, and is used to compute 'delta', etc.
    * when the increment is finalized and passed to an accumulator.
    *
    * @return  The name as a string.
    */

    string in_agentvar_name() const;

    CodeBlock cxx_prepare_increment() const;

    /**
    * The table using the agentvar
    *
    * Stored as a reference to a pointer, which is stable to symbol morphing during the parse phase.
    */

    Symbol*& table;

    /**
    * The table using the agentvar
    *
    * Stored as a pointer, which is only valid after post-parse phase 1.
    */
    TableSymbol* pp_table;

    /**
    * The agentvar used in the analysis dimension of the table
    *
    * Stored as a reference to a pointer, which is stable to symbol morphing during the parse phase.
    */

    Symbol*& agentvar;

    /**
    * The agentvar used in the analysis dimension of the table.
    *
    * Stored as a pointer, which is only valid after post-parse phase 1.
    */

    AgentVarSymbol* pp_agentvar;


    /**
    * Zero-based index of the analysis agentvar
    * in the list of all analysis agentvars used in the table.
    */

    const int index;

    /**
    * true means that generated code needs the member storing the "value_in" of the agentvar
    *
    * Only valid after post-parse phase 1.
    */

    bool need_value_in;



};

