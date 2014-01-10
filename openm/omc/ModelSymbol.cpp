/**
* @file    ModelSymbol.cpp
* Definitions for the ModelSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ModelSymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/modelBuilder.h"

using namespace std;
using namespace openm;


CodeBlock ModelSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    c += "// model identification";
    c += "const char * OM_MODEL_NAME =\"" + name + "\";";
    c += "";
    c += "// model time stamp";
    c += "const char * OM_MODEL_TIMESTAMP = \"" + time_stamp + "\";";

    return c;
}

void ModelSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    metaRows.modelDic.name = name;
    metaRows.modelDic.timestamp = time_stamp;

    for (auto lang : Symbol::pp_all_languages) {
        ModelDicTxtLangRow modelTxt;
        modelTxt.langName = lang->name;
        modelTxt.langId = lang->language_id;
        modelTxt.descr = label(*lang);
        modelTxt.note = note(*lang);
        metaRows.modelTxt.push_back(modelTxt);
    }
};
