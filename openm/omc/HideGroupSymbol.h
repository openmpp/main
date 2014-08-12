/**
* @file    HideGroupSymbol.h
* Declarations for the HideGroupSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "GroupSymbol.h"

/**
* HideGroupSymbol.
*
*/
class HideGroupSymbol : public GroupSymbol
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
    HideGroupSymbol(yy::location decl_loc = yy::location())
        : GroupSymbol("om_hide_" + to_string(++counter), decl_loc)
    {
    }

    void post_parse(int pass);

    static int counter;
};
