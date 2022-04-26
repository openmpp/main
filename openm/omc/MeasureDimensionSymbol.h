/**
* @file    MeasureDimensionSymbol.h
* Declarations for the MeasureDimensionSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "TableSymbol.h"

using namespace std;

/**
* MeasureDimensionSymbol.
* 
* Corresponds to the single measure dimension of a table.
*/
class MeasureDimensionSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    MeasureDimensionSymbol(TableSymbol *table, omc::location decl_loc = omc::location())
        : Symbol(symbol_name(table), decl_loc)
        , table(table)
    {
    }

    /**
     * Construct symbol name for the measure dimension symbol. Example: BasicDemography.measures.
     *
     * @param table The table containing this measure dimension.
     *
     * @return A string.
     */
    static string symbol_name(const TableSymbol* table);

    void post_parse(int pass);

    /**
     * The enumeration for the dimension
     * 
     * Only valid after post-parse.
     */
    TableSymbol* table;
};

