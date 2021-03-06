/**
* @file    UnknownTypeSymbol.h
* Declarations for the UnknownTypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "TypeSymbol.h"

using namespace std;
using namespace openm;

/**
* BoolSymbol.
*/
class UnknownTypeSymbol : public TypeSymbol
{
private:
    typedef TypeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor for an unknown type symbol
    */
    UnknownTypeSymbol(const string nm)
        : TypeSymbol(nm)
    {
    }

    const string default_initial_value() const {
        return "0";
    };

    /**
     * Gets the built-in 'unknown type' symbol with name om_unknown
     *
     * @return the UnknownTypeSymbol*.
     */
    static UnknownTypeSymbol *find();
};

