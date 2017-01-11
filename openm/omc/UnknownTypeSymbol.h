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
    * Constructor for the unknown type symbol
    */
    UnknownTypeSymbol()
        : TypeSymbol("om_unknown")
    {
    }

    const string default_initial_value() const {
        // no types of this kind should be present in code generation phase
        return "error_unknown_type";
    };

    /**
     * Gets the one and only 'unknown type' symbol.
     *
     * @return the UnknownTypeSymbol*.
     */
    static UnknownTypeSymbol *find();
};

