/**
* @file    MeasureDimensionSymbol.h
* Definitions for the MeasureDimensionSymbol class.
*/
// Copyright (c) 2013-2022 OpenM++ contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "MeasureDimensionSymbol.h"
#include "LanguageSymbol.h"
#include "omc_file.h" // for LTA support

using namespace openm;
using namespace omc; // for LTA support

// static
string MeasureDimensionSymbol::symbol_name(const TableSymbol* table)
{
    assert(table);
    return table->name + "::expression_dimension";
}

string MeasureDimensionSymbol::default_label(const LanguageSymbol& langSym) const
{
    const string& lang = langSym.name; // e.g. "EN" or "FR"
    return LTA(lang, "Quantity");
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

