/**
* @file    ModelSymbol.cpp
* Definitions for the ModelSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

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

    // Perform operations specific to this level in the Symbol hierarchy.

    c += "// model identification";
    c += "const char * OM_MODEL_NAME =\"" + name + "\";";
    c += "";
    c += "// model time stamp";
    c += "const char * OM_MODEL_TIMESTAMP = \"" + time_stamp + "\";";
    c += "";
    c += "// parameter metadata digest";
    c += "const char * OM_MODEL_DIGEST = \"" + model_digest + "\";";

    return c;
}

string ModelSymbol::metadata_signature() const
{
    // Hook into the hierarchical calling chain
    string sig = super::metadata_signature();

    // Perform operations specific to this level in the Symbol hierarchy.
    // TODO: Consider whether model name should be part of signature.
    {
        sig += "parameters: " + to_string(pp_all_parameters.size()) + "\n";
        int parm_index = 0;
        for (auto param : pp_all_parameters) {
            sig += "parameter " + to_string(parm_index) + ": ";
            sig += param->metadata_signature();
            sig += "\n";
            parm_index++;
        }
    }

    return sig;
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
        modelTxt.descr = label(*lang);
        modelTxt.note = note(*lang);
        metaRows.modelTxt.push_back(modelTxt);
    }
};
