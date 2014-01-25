/**
* @file    PartitionSymbol.h
* Declarations for the PartitionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "EnumTypeSymbol.h"

using namespace std;
using namespace openm;


/**
* PartitionSymbol.
*/
class PartitionSymbol : public EnumTypeSymbol
{
private:
    typedef EnumTypeSymbol super;

public:

    /**
    * Constructor.
    *
    * @param [in,out] sym The symbol to be morphed.
    */

    PartitionSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : EnumTypeSymbol(sym, token::TK_int, kind_of_type::partition_type, decl_loc)
    {
    }

    const string get_initial_value() const;

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

};

