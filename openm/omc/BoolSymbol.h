/**
* @file    BoolSymbol.h
* Declarations for the BoolSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "EnumerationWithEnumeratorsSymbol.h"
#include "TypeSymbol.h"
#include "Constant.h"

using namespace std;
using namespace openm;

/**
* BoolSymbol.
*/
class BoolSymbol : public EnumerationWithEnumeratorsSymbol
{
private:
    typedef EnumerationWithEnumeratorsSymbol super;

public:
    bool is_base_symbol() const override { return false; }

    /**
     * Constructor for the bool enumeration symbol.
     */
    BoolSymbol()
        : EnumerationWithEnumeratorsSymbol(token_to_string(token::TK_bool), token::TK_bool, kind_of_type::logical_type)
    {
    }

    void post_parse(int pass) override;

    bool is_valid_constant(const Constant &k) const override;

    /**
    * Create new boolean Constant from 'i_value' enum literal.
    *
    * @return pointer to a new Constant or nullptr on error.
    */
    Constant * make_constant(const string & i_value) const override;

    string format_for_storage(const Constant &k) const override;

    /**
     * Gets the one and only bool type symbol.
     *
     * @return null if it fails, else a BoolSymbol*.
     */
    static BoolSymbol *find();
};

