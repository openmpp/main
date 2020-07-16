/**
* @file    StringTypeSymbol.h
* Declarations for the StringTypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "TypeSymbol.h"

using namespace std;
using namespace openm;

/**
* StringTypeSymbol.
* 
* Type of a parameter declared as 'file'.
*/
class StringTypeSymbol : public TypeSymbol
{
private:
    typedef TypeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor for the string type symbol
    */
    StringTypeSymbol()
        : TypeSymbol("string")
    {
    }

    void post_parse(int pass);

    void populate_metadata(openm::MetaModelHolder & metaRows);

    bool is_valid_constant(const Constant &k) const;

    /**
    * Create new string Constant from 'i_value' literal.
    *
    * @return pointer to a new Constant or nullptr on error.
    */
    Constant * make_constant(const string & i_value) const override;

    const string default_initial_value() const {
        return "";
    };

    /**
     * Gets the one and only 'string type' symbol.
     *
     * @return null if it fails, else a StringTypeSymbol*.
     */
    static StringTypeSymbol *find();

    /**
     * Formats the constant for the data store.
     *
     * @param k The constant to be formatted.
     *
     * @return The formatted for storage.
     */
    string format_for_storage(const Constant &k) const override;
};

