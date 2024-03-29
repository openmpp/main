/**
* @file    EnumerationWithEnumeratorsSymbol.cpp
* Definitions for the EnumerationWithEnumeratorsSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EnumerationWithEnumeratorsSymbol.h"
#include "EnumeratorSymbol.h"
#include "LanguageSymbol.h"
#include "ClassificationSymbol.h"
#include "PartitionSymbol.h"
#include "BoolSymbol.h"
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
            typeEnum.name = enumerator->db_name();
            metaRows.typeEnum.push_back(typeEnum);
        }

        for (const auto& langSym : Symbol::pp_all_languages) {
            const string& lang = langSym->name; // e.g. "EN" or "FR"
            TypeEnumTxtLangRow typeEnumTxt;
            typeEnumTxt.typeId = type_id;
            typeEnumTxt.enumId = enumerator->ordinal;
            typeEnumTxt.langCode = lang;
            typeEnumTxt.descr = enumerator->label(*langSym);
            typeEnumTxt.note = enumerator->note(*langSym);
            metaRows.typeEnumTxt.push_back(typeEnumTxt);
        }
    }
}

size_t EnumerationWithEnumeratorsSymbol::pp_size() const
{
    return pp_enumerators.size();
}

bool EnumerationWithEnumeratorsSymbol::is_classification(void) const
{
    return nullptr != dynamic_cast<const ClassificationSymbol*>(this);
}

bool EnumerationWithEnumeratorsSymbol::is_partition(void) const
{
    return nullptr != dynamic_cast<const PartitionSymbol*>(this);
}

bool EnumerationWithEnumeratorsSymbol::is_bool(void) const
{
    return nullptr != dynamic_cast<const BoolSymbol*>(this);
}

bool EnumerationWithEnumeratorsSymbol::is_valid_enum_name(const char * i_value) const
{
    return i_value != nullptr && pp_name_to_int.count(i_value) != 0;
}

