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
* Represents an attribute used in the analysis dimension
* (also known as the expression dimension) of a table.
* An attribute can be used multiple times in the analysis dimension of a table.
* If so, it is represented by a single instance of this class.
* This class handles the creation of an associated attribute which holds the
* 'in' value of the attribute when a table increment is started.
*/

class EntityTableMeasureAttributeSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityTableMeasureAttributeSymbol(Symbol *table, Symbol *attribute, int index)
        : Symbol(symbol_name(table, attribute))
        , table(table->stable_rp())
        , pp_table(nullptr)
        , attribute(attribute->stable_rp())
        , pp_attribute(nullptr)
        , index(index)
        , need_value_in(false)
        , need_value_in_event(false)
    {
    }

    /**
    * Get the unique name for a specified symbol of this kind.
    *
    * @param   table       The table.
    * @param   attribute    The attribute used in the analysis dimension
    *
    * @return  The symbol name as a string.
    *          Example: om_taav_DurationOfLife_alive.
    */

    static string symbol_name(const Symbol *table, const Symbol *attribute);

    /**
    * Check for existence of a specific table analysis attribute.
    *
    * @param   table       The table.
    * @param   attribute    The attribute.
    *
    * @return  true if found, else false.
    */

    static bool exists(const Symbol *table, const Symbol *attribute);

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
    * The table using the attribute
    *
    * Stored as a reference to a pointer, which is stable to symbol morphing during the parse phase.
    */

    Symbol*& table;

    /**
    * The table using the attribute
    *
    * Stored as a pointer, which is only valid after post-parse phase 1.
    */
    EntityTableSymbol* pp_table;

    /**
    * The attribute used in the analysis dimension of the table
    *
    * Stored as a reference to a pointer, which is stable to symbol morphing during the parse phase.
    */

    Symbol*& attribute;

    /**
    * The attribute used in the analysis dimension of the table.
    *
    * Stored as a pointer, which is only valid after post-parse phase 1.
    */

    AttributeSymbol* pp_attribute;


    /**
    * Zero-based index of the analysis attribute
    * in the list of all analysis attributes used in the table.
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

