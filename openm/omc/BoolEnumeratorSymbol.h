/**
* @file    BoolEnumeratorSymbol.h
* Declarations for the BoolEnumeratorSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "EnumeratorSymbol.h"
#include "LanguageSymbol.h"

class EnumerationSymbol;

using namespace std;

class BoolEnumeratorSymbol : public EnumeratorSymbol
{
private:
    typedef EnumeratorSymbol super;

public:
    bool is_base_symbol() const override { return false; }

    explicit BoolEnumeratorSymbol(const string unm, const Symbol *enumeration, int ordinal)
        : EnumeratorSymbol(unm, enumeration, ordinal)
    {
    }

	string db_name() const
	{
		// The 'name' in the data store is the ordinal, which is 0 for false and 1 for true
		return to_string(ordinal);
	};

    /**
     * Gets the fixed label for the enumerator in bool.
	 *
	 * Hard-coded support for EN and FR, for now.
     *
     * @param language The language.
     *
     * @return A string.
     */
    string label(const LanguageSymbol & language) const
    {
        string result("");
        if (ordinal == 0) {
            if      (language.name == "EN") result = "false";
            else if (language.name == "FR") result = "faux";
            else result = "false";
        }
        else {
            assert(ordinal == 1);
            if      (language.name == "EN") result = "true";
            else if (language.name == "FR") result = "vrai";
            else result = "true";
        }
        return result;
    }

    /**
     * Gets the (always empty) note.
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
};


