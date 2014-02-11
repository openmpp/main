/**
* @file    PartitionSymbol.h
* Declarations for the PartitionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include <cfloat>
#include "EnumerationSymbol.h"

using namespace std;
using namespace openm;


/**
* PartitionSymbol.
*/
class PartitionSymbol : public EnumerationSymbol
{
private:
    typedef EnumerationSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    *
    * @param [in,out] sym The symbol to be morphed.
    */

    PartitionSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : EnumerationSymbol(sym, token::TK_int, kind_of_type::partition_type, decl_loc)
    {
    }

    const string default_initial_value() const;

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

};

