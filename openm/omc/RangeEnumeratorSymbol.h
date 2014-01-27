/**
* @file    RangeEnumeratorSymbol.h
* Declarations for the RangeEnumeratorSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "EnumeratorSymbol.h"

class EnumerationSymbol;

using namespace std;

class RangeEnumeratorSymbol : public EnumeratorSymbol
{
private:
    typedef EnumeratorSymbol super;

public:
    bool is_base_symbol() const { return false; }

    RangeEnumeratorSymbol(const string unm, const Symbol *enumeration, int ordinal, int lower_bound)
        : EnumeratorSymbol(unm, enumeration, ordinal)
        , lower_bound(lower_bound)
    {
    }


    /**
    * Gets the label for the enumerator in the range.
    *
    * This specialization is language-independent.
    *
    * @param language The language.
    *
    * @return A string.
    */

    string label(const LanguageSymbol & language) const
    {
        return to_string(lower_bound + ordinal);
    }


    /**
    * Gets the (always empty) note for an enumerator in the range.
    *
    * This specialization is language-independent.
    *
    * @param language The language.
    *
    * @return A string.
    */

    string note(const LanguageSymbol & language) const
    {
        return "";
    }


    /**
    * The lower bound of the range
    */

    int lower_bound;

};


