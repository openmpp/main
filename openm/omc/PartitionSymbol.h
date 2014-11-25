/**
* @file    PartitionSymbol.h
* Declarations for the PartitionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include <cfloat>
#include "EnumerationWithEnumeratorsSymbol.h"
#include "Constant.h"

using namespace std;
using namespace openm;


/**
* PartitionSymbol.
*/
class PartitionSymbol : public EnumerationWithEnumeratorsSymbol
{
private:
    typedef EnumerationWithEnumeratorsSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    *
    * @param [in,out] sym The symbol to be morphed.
    */

    PartitionSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : EnumerationWithEnumeratorsSymbol(sym, token::TK_int, kind_of_type::partition_type, decl_loc)
    {
    }

    const string default_initial_value() const;

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    bool is_valid_constant(const Constant &k) const;

    /**
     * Format a number to a valid literal for a real
     * 
     * A double literal requires no transformation, but if real_type if float, a trailing 'f' needs
     * to be appended to avoid compiler warnings. Also, if a trailing f is appended, the number must
     * contain a '.'.
     *
     * @param num           Number to be converted (string)
     * @param real_is_float true if real is float.
     *
     * @return The formatted float.
     */
    static string real_literal(string num, bool real_is_float);

};

