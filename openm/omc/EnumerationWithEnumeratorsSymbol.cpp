/**
* @file    EnumerationWithEnumeratorsSymbol.cpp
* Definitions for the EnumerationWithEnumeratorsSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EnumerationWithEnumeratorsSymbol.h"
#include "EnumeratorSymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

const string EnumerationWithEnumeratorsSymbol::default_initial_value() const
{
    // enumerators have been populated
    assert(pp_enumerators.size() > 0); // grammar guarnatee

    // initial value is first enumerator
    return pp_enumerators.front()->name;
};

void EnumerationWithEnumeratorsSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // Add this enumeration to the complete list of enumerations with enumerators.
        pp_all_enumerations_with_enumerators.push_back(this);
        break;
    }
    default:
        break;
    }
}

void EnumerationWithEnumeratorsSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    // Only generate metadata if the enumeration has been marked
    // as being used in a table or external parameter
    if (!metadata_needed) return;

    // Generate metadata for enumerators in this enumeration
    for (auto enumerator : pp_enumerators) {
        {
            TypeEnumLstRow typeEnum;
            typeEnum.typeId = type_id;
            typeEnum.enumId = enumerator->ordinal;
            typeEnum.name = enumerator->name;
            metaRows.typeEnum.push_back(typeEnum);
        }

        for (auto lang : Symbol::pp_all_languages) {
            TypeEnumTxtLangRow typeEnumTxt;
            typeEnumTxt.typeId = type_id;
            typeEnumTxt.enumId = enumerator->ordinal;
            typeEnumTxt.langName = lang->name;
            typeEnumTxt.descr = enumerator->label(*lang);
            typeEnumTxt.note = enumerator->note(*lang);
            metaRows.typeEnumTxt.push_back(typeEnumTxt);
        }
    }
}

int EnumerationWithEnumeratorsSymbol::pp_size()
{
    return pp_enumerators.size();
}


