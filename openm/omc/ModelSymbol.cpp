/**
* @file    ModelSymbol.cpp
* Definitions for the ModelSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ModelSymbol.h"
#include "LanguageSymbol.h"
#include "ParameterSymbol.h"
#include "TableSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/modelBuilder.h"

using namespace std;
using namespace openm;


CodeBlock ModelSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();
    return c;
}

void ModelSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
    metaRows.modelDic.name = external_name;

    for (auto lang : Symbol::pp_all_languages) {
        ModelDicTxtLangRow modelTxt;
        modelTxt.langName = lang->name;
        modelTxt.descr = label(*lang);
        modelTxt.note = note(*lang);
        metaRows.modelTxt.push_back(modelTxt);
    }
};
