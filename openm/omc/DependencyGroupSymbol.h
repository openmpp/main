/**
* @file    DependencyGroupSymbol.h
* Declarations for the DependencyGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "GroupSymbol.h"
#include "TableSymbol.h"

/**
* DependencyGroupSymbol.
*
*/
class DependencyGroupSymbol : public GroupSymbol
{
private:
    typedef GroupSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * constructor
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    DependencyGroupSymbol(omc::location decl_loc = omc::location())
        : GroupSymbol("om_dependency_" + to_string(++counter), decl_loc)
        , pp_table_requiring(nullptr)
    {
    }

    void post_parse(int pass);

    /**
    * The requiring table.
    * First symbol in the dependency statement.
    */
    TableSymbol *pp_table_requiring;

    static int counter;
};
