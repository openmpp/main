/**
* @file    TableMeasureSymbol.h
* Declarations for the TableMeasureSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <regex>
#include "Symbol.h"

using namespace std;

/**
* TableMeasureSymbol.
*
*/
class TableMeasureSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    TableMeasureSymbol(Symbol* table, string* pname_default, string* pname_explicit, int index, omc::location decl_loc = omc::location())
        : Symbol(symbol_name(table, index, pname_default), decl_loc)
        , short_name(pname_explicit ? *pname_explicit : pname_default ? *pname_default : "Expr" + to_string(index))
        , short_name_default(pname_default ? *pname_default : "Expr" + to_string(index))
        , short_name_explicit(pname_explicit ? *pname_explicit : "")
        , index(index)
        , table(table->stable_rp())
        , pp_table(nullptr)
        , decimals(-1)
        , scale(0)
    {
    }

    // Construct symbol name for the table measure symbol.
    // Example: BasicDemography.Expr0
    static string symbol_name(const Symbol* table, int index, string* pname_default);

    /**
     * Make measure name suitable for database column name: it must be unique column name consist of 8 alphanumeric characters.
     *
     * @param i_tableName    name of the table.
     * @param i_measureLst  list of measures.
     * @param[in.out] io_me the measure which name must be validated and updated if required.
     */
    static void to_column_name(const string & i_tableName, const list<TableMeasureSymbol *> i_measureLst, TableMeasureSymbol * io_me);

    void post_parse(int pass);

    string pretty_name() const;

    /**
     * Heuristically-generated short name for the dimension
     *
     * @return Result as a string
     */
    string heuristic_short_name(void) const;

    /**
     * Return the measure scale transformed into a decimal number
     */
    string scale_as_factor() const
    {
        return "1.0E" + to_string(scale);
    }

    /**
     * Short name of the measure
     */
    string short_name;

    /**
    * Short name of the measure (generated default name)
    */
    string short_name_default;

    /**
    * Short name of the measure (provided in model source)
    */
    string short_name_explicit;

    /**
     * Zero-based index of the measure in the table measures.
     */
    int index;

    /**
     * The table containing this measure (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& table;

    /**
     * The table containing this measure (pointer)
     * 
     * Only valid after post-parse phase 1.
     */
    TableSymbol* pp_table;

    /**
     * The default display decimals of this measure
     */
    int decimals;

    /**
     * The decimal scaling to applyt o this measure
     */
    int scale;
};
