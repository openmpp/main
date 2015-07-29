/**
* @file    HideGroupSymbol.h
* Declarations for the HideGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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
    * Constructor.
    * 
    * A unique name is generated using a counter.
    */
    HideGroupSymbol(yy::location decl_loc = yy::location())
        : GroupSymbol("om_hide_" + to_string(++counter), decl_loc)
    {
    }

    void post_parse(int pass);

    static int counter;
};
