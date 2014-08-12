/**
* @file    TableGroupSymbol.h
* Declarations for the TableGroupSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

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
    TableGroupSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : GroupSymbol(sym, decl_loc)
    {
    }

    void post_parse(int pass);
};
