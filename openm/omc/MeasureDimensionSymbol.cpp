/**
* @file    MeasureDimensionSymbol.h
* Definitions for the MeasureDimensionSymbol class.
*/
// Copyright (c) 2013-2022 OpenM++ contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "MeasureDimensionSymbol.h"
#include "LanguageSymbol.h"

using namespace openm;

// static
string MeasureDimensionSymbol::symbol_name(const TableSymbol* table)
{
    assert(table);
    return table->name + "::measures";
}

string MeasureDimensionSymbol::default_label(const LanguageSymbol& lang) const
{
    if (lang.name == "EN") {
        return "Measure";
    }
    else if (lang.name == "FR") {
        return "Mesure";
    }
    else {
        return "Measure";
    }
}

void MeasureDimensionSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignLabel:
    {
        break;
    }
    default:
        break;
    }
}

