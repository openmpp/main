/**
* @file    ClassificationSymbol.cpp
* Definitions for the ClassificationSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "ClassificationSymbol.h"
#include "ClassificationLevelSymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

void ClassificationSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // Add this classification to the complete list of classifications.
            pp_all_classifications.push_back(this);
        }
        break;
    default:
        break;
    }
}

CodeBlock ClassificationSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    
    h += doxygen(name);
    h += "typedef enum {";
    for ( auto classification_level : pp_classification_levels) {
        h += classification_level->name + ",";
    }
    h += "} " + name + ";" ;

    return h;
}

void ClassificationSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    {
        TypeDicRow typeDic;
        typeDic.typeId = type_id;
        typeDic.name = name;
        typeDic.dicId = 2;          // classification types
        typeDic.totalEnumId = pp_classification_levels.size();
        metaRows.typeDic.push_back(typeDic);
    }

    for (auto lang : Symbol::pp_all_languages) {
        TypeDicTxtLangRow typeTxt;
        typeTxt.typeId = type_id;
        typeTxt.langName = lang->name;
        typeTxt.langId = lang->language_id;
        typeTxt.descr = label(*lang);
        typeTxt.note = note(*lang);
        metaRows.typeTxt.push_back(typeTxt);
    }

    for (auto classification_level : pp_classification_levels) {
        {
            TypeEnumLstRow typeEnum;
            typeEnum.typeId = type_id;
            typeEnum.enumId = classification_level->ordinal;
            typeEnum.name = classification_level->name;
            metaRows.typeEnum.push_back(typeEnum);
        }

        for (auto lang : Symbol::pp_all_languages) {
            TypeEnumTxtLangRow typeEnumTxt;
            typeEnumTxt.typeId = type_id;
            typeEnumTxt.enumId = classification_level->ordinal;
            typeEnumTxt.langName = lang->name;
            typeEnumTxt.langId = lang->language_id;
            typeEnumTxt.descr = classification_level->label(*lang);
            typeEnumTxt.note = classification_level->note(*lang);
            metaRows.typeEnumTxt.push_back(typeEnumTxt);
        }
    }
}

