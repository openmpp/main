/**
* @file    EnumTypeSymbol.cpp
* Definitions for the EnumTypeSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "EnumTypeSymbol.h"
#include "EnumeratorSymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

const string EnumTypeSymbol::get_initial_value() const
{
    // initial value is first enumerator
    return pp_enumerators.front()->name;
};

void EnumTypeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // Add this enumeration to the complete list of enumerations.
            pp_all_enumerations.push_back(this);
        }
        break;
    default:
        break;
    }
}

void EnumTypeSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    {
        TypeDicRow typeDic;
        typeDic.typeId = type_id;
        typeDic.name = name;
        typeDic.dicId = dicId;          // see enum kind_of_type for possible values
        typeDic.totalEnumId = pp_size();
        metaRows.typeDic.push_back(typeDic);
    }

    for (auto lang : Symbol::pp_all_languages) {
        TypeDicTxtLangRow typeTxt;
        typeTxt.typeId = type_id;
        typeTxt.langName = lang->name;
        typeTxt.descr = label(*lang);
        typeTxt.note = note(*lang);
        metaRows.typeTxt.push_back(typeTxt);
    }

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

