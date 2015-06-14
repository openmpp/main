/**
* @file    StringTypeSymbol.h
* Declarations for the StringTypeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

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

    const string default_initial_value() const {
        return "";
    };

    /**
     * Gets the one and only 'string type' symbol.
     *
     * @return null if it fails, else a StringTypeSymbol*.
     */
    static StringTypeSymbol *find();
};

