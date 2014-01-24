/**
* @file    RangeSymbol.h
* Declarations for the RangeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "EnumTypeSymbol.h"
#include "EnumeratorSymbol.h"

using namespace std;
using namespace openm;


/**
* RangeSymbol.
*/
class RangeSymbol : public EnumTypeSymbol
{
private:
    typedef EnumTypeSymbol super;

public:

    /**
    * Constructor.
    *
    * @param [in,out] sym The symbol to be morphed.
    */

    RangeSymbol(Symbol *sym, int lower_bound, int upper_bound, yy::location decl_loc = yy::location())
        : EnumTypeSymbol(sym, kind_of_type::range_type, decl_loc)
        , lower_bound(lower_bound)
        , upper_bound(upper_bound)
    {
    }

    const string get_initial_value() const;

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();


    /**
     * The lower bound of the range.
     */

    int lower_bound;


    /**
     * The upper bound of the range.
     */

    int upper_bound;

};

