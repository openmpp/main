/**
* @file    DependencyGroupSymbol.h
* Declarations for the DependencyGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "GroupSymbol.h"

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
    DependencyGroupSymbol(yy::location decl_loc = yy::location())
        : GroupSymbol("om_dependency_" + to_string(++counter), decl_loc)
    {
    }

    void post_parse(int pass);

    static int counter;
};
