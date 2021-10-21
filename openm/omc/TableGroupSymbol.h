/**
* @file    TableGroupSymbol.h
* Declarations for the TableGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "GroupSymbol.h"

/**
* TableGroupSymbol.
*
*/
class TableGroupSymbol : public GroupSymbol
{
private:
    typedef GroupSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    TableGroupSymbol(Symbol *sym, omc::location decl_loc = omc::location())
        : GroupSymbol(sym, decl_loc)
    {
    }

    void post_parse(int pass);

    void populate_metadata(openm::MetaModelHolder& metaRows);

    /**
     * Determine if group contains a published table
     *
     * @return true if group contains a published table, otherwise false
     */
    const bool contains_published_table();

};
