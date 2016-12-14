/**
* @file    PartitionEnumeratorSymbol.h
* Declarations for the PartitionEnumeratorSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "EnumeratorSymbol.h"

class EnumerationSymbol;

using namespace std;

class PartitionEnumeratorSymbol : public EnumeratorSymbol
{
private:
    typedef EnumeratorSymbol super;

public:
    bool is_base_symbol() const { return false; }

    explicit PartitionEnumeratorSymbol(const string unm, const Symbol *enumeration, int ordinal, string upper_split_point, yy::location decl_loc = yy::location())
        : EnumeratorSymbol(unm, enumeration, ordinal, decl_loc)
        , upper_split_point(upper_split_point)
    {
    }

    /**
     * Gets the fixed label for the interval in the partition.
     * 
     * This specialization is language-independent.
     *
     * @param language The language.
     *
     * @return A string.
     */
    string label(const LanguageSymbol & language) const
    {
        string result = "";
        if (lower_split_point == "min") {
            //result += "(-\u221e"; // unicode for infinity - requires C++ compiler support for UTF-8
            result += "(-\xE2\x88\x9E"; // UTF-8 for infinity in hexadecimal
        }
        else {
            result += "[" + lower_split_point;
        }

        result += ",";

        if (upper_split_point == "max") {
            //result += "\u221e)"; // unicode for infinity - requires C++ compiler support for UTF-8
            result += "\xE2\x88\x9E)"; // UTF-8 for infinity in hexadecimal
        }
        else {
            result += upper_split_point + ")";
        }

        return result;
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


