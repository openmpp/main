/**
* @file    EntityTableMeasureAttributeSymbol.h
* Declarations for the EntityTableMeasureAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <string>
#include "Symbol.h"

class AttributeSymbol;
class CodeBlock;

using namespace std;

/**
* EntityTableMeasureAttributeSymbol.
*
* Represents an agentvar used in the analysis dimension
* (also known as the expression dimension) of a table.
* An agentvar can be used multiple times in the analysis dimension of a table.
* If so, it is represented by a single instance of this class.
* This class handles the creation of an associated agentvar which holds the
* 'in' value of the agentvar when a table increment is started.
*/

class EntityTableMeasureAttributeSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityTableMeasureAttributeSymbol(Symbol *table, Symbol *agentvar, int index)
        : Symbol(symbol_name(table, agentvar))
        , table(table->stable_rp())
        , pp_table(nullptr)
        , agentvar(agentvar->stable_rp())
        , pp_agentvar(nullptr)
        , index(index)
        , need_value_in(false)
        , need_value_in_event(false)
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
     * Gets the member name for the 'in' value (for 'interval' table operator)
     * 
     * Stores the "in" value of the attribute at the start of an increment.
     *
     * @return The name as a string.
     */
    string in_member_name() const;

    /**
     * Gets the member name for the 'in' value (for 'event' table operator)
     * 
     * Stores the "in" value of the attribute at the start of an increment.
     *
     * @return The name as a string.
     */
    string in_event_member_name() const;

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
    EntityTableSymbol* pp_table;

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

    AttributeSymbol* pp_agentvar;


    /**
    * Zero-based index of the analysis agentvar
    * in the list of all analysis agentvars used in the table.
    */

    const int index;

    /**
     * true means that generated code needs the member storing the "value_in" of the attribute
     */
    bool need_value_in;

    /**
     * true means that generated code needs the member storing the "value_in" of event(attribute)
     */
    bool need_value_in_event;
};

