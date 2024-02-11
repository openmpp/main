/**
* @file    ClassificationEnumeratorSymbol.h
* Declarations for the ClassificationEnumeratorSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "EnumeratorSymbol.h"

class EnumerationSymbol;

using namespace std;

class ClassificationEnumeratorSymbol : public EnumeratorSymbol
{
private:
    typedef EnumeratorSymbol super;

public:
    bool is_base_symbol() const override { return false; }

    explicit ClassificationEnumeratorSymbol(Symbol *sym, string* pname, const Symbol *enumeration, int ordinal, omc::location decl_loc = omc::location())
        : EnumeratorSymbol(sym, enumeration, ordinal, decl_loc)
        , short_name(pname ? *pname : name)
        , short_name_default(name)
        , short_name_explicit(pname ? *pname : "")
    {
    }

    /**
    * Heuristically-generated short name for the enumerator
    * 
    * Adapted from DimensionSymbol::heuristic_short_name
    */
    string heuristic_short_name(void) const
    {
        string hn; // heuristic name

        // Get the dimension's label for the model's default language.
        string lbl = pp_labels[0];
        string unm = unique_name;
        if (lbl != unm && lbl.length() <= short_name_max_length) {
            // Use label from model source code, if not too long.
            hn = lbl;
        }
        else {
            hn = unm;
        }

        assert(hn.length() > 0); // logic guarantee

        // trim off leading "om_" prefix if present
        //if (hn.starts_with("om_")) { // c++20
        if (hn.length() >= 3 && hn.substr(0, 3) == "om_") {
            hn.erase(0, 3);
        }

        if (!std::isalpha(hn[0], locale::classic())) {
            // First character is not alphabetic.
            // Prepend X_ to make valid name
            hn = "X_" + hn;
        }

        // trim off trailing "_" if present
        //if (hn.ends_with("_")) { // c++20
        if (hn[hn.length() - 1] == '_') {
            hn.erase(hn.length() - 1, 1);
        }

        // if name is subject to truncation, remove characters from middle rather than truncating from end,
        // because long descriptions often disambiguate at both beginning and end.
        if (hn.length() > short_name_max_length) {
            // replacement string for snipped section
            string r = "_x_";
            // number of characters to snip from middle (excess plus length of replacement)
            size_t n = hn.length() - short_name_max_length + r.length();
            // start of snipped section (middle section of original string)
            size_t p = (hn.length() - n) / 2;
            // replace snipped section with _
            if (p > 0 && p < hn.length()) {
                hn.replace(p, n, r);
            }
        }

        // Make name alphanumeric and truncate it to maximum length.
        hn = openm::toAlphaNumeric(hn, (int)short_name_max_length);

        // trim off trailing "_" if present
        //if (hn.ends_with("_")) { // c++20
        if (hn[hn.length() - 1] == '_') {
            hn.erase(hn.length() - 1, 1);
        }

        return hn;
    }

    void post_parse(int pass) override
    {
        // Hook into the post_parse hierarchical calling chain
        super::post_parse(pass);

        // Perform post-parse operations specific to this level in the Symbol hierarchy.
        switch (pass) {
        case eAssignLabel:
        {
            // If an explicit short name was given by //NAME, use it
            auto search = explicit_names.find(unique_name);
            if (search != explicit_names.end()) {
                auto text = (search->second).first;
                auto loc = (search->second).second;
                short_name_explicit = text;
                short_name = short_name_explicit;
            }
            // If an explicit short name was provided, use it to look for and process if found
            // a LABEL or NOTE which uses it as key, e.g. //LABEL(MyTable.Prov,FR)
            if (short_name_explicit != "") {
                // note that pp_enumeration is not yet available in this pass
                string key = (*enumeration).name + "::" + short_name;
                associate_explicit_label_or_note(key);
            }
            break;
        }
        default:
            break;
        }
    }

    /**
    * Short name of the enumerator
    */
    string short_name;

    /**
    * Short name of the enumerator (generated default name)
    */
    string short_name_default;

    /**
    * Short name of the enumerator (provided in model source)
    */
    string short_name_explicit;
    
    string db_name() const override
	{
		// The 'name' in the data store is the same as the short name for the level
		return short_name;
	};

};


