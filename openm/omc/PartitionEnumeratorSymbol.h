/**
* @file    PartitionEnumeratorSymbol.h
* Declarations for the PartitionEnumeratorSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "EnumeratorSymbol.h"

class EnumTypeSymbol;

using namespace std;

class PartitionEnumeratorSymbol : public EnumeratorSymbol
{
private:
    typedef EnumeratorSymbol super;

public:
    bool is_base_symbol() const { return false; }

    PartitionEnumeratorSymbol(const string unm, const Symbol *enumeration, int ordinal, string upper_split_point, yy::location decl_loc = yy::location())
        : EnumeratorSymbol(unm, enumeration, ordinal, decl_loc)
        , upper_split_point(upper_split_point)
    {
    }


    /**
     * Gets the label for the interval in the partition.
     * 
     * This specialization is language-independent.
     *
     * @param language The language.
     *
     * @return A string.
     */

    string label(const LanguageSymbol & language) const
    {
        return "["
            + lower_split_point
            + ","
            + upper_split_point
            + ( (upper_split_point == "max") ? "]" : ")" );
    }


    /**
    * Gets the (always empty) note for the interval in the partition.
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
    * The lower split point of the partition interval
    *
    * The lower value is supplied in the post-parse phase.
    */

    string lower_split_point;


    /**
     * The upper split point of the partition interval
     * 
     * The upper value is supplied at parse time throught the constructor.
     */

    string upper_split_point;
};


