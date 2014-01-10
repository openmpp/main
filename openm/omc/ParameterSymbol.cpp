/**
* @file    ParameterSymbol.h
* Definitions for the ParameterSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ParameterSymbol.h"
#include "LanguageSymbol.h"
#include "TypedefTypeSymbol.h"
#include "CodeBlock.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;
using namespace openm;

void ParameterSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // add this parameter to the complete list of parameters
            pp_all_parameters.push_back(this);
        }
        break;
    default:
        break;
    }
}

CodeBlock ParameterSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    string parm_type = Symbol::token_to_string(type);
    h += "extern " + parm_type + " " + name + ";";
    return h;
}

CodeBlock ParameterSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    string parm_type = Symbol::token_to_string(type);
    c += parm_type + " " + name + " = 0.0;";
    return c;
}

void ParameterSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata( metaRows );

    // Perform operations specific to this level in the Symbol hierarchy.
 
    ParamDicRow paramDic;

    paramDic.paramId = pp_parameter_id;
    paramDic.paramName = name;  // must be valid database view name, if we want to use compatibility views
    paramDic.rank = 0; // TODO: currently hard-coded to scalar parmaeters for alpha

    // get the type_id (e.g. 12) of the parameter data type (e.g. TK_double)
    auto *sym = TypedefTypeSymbol::get_typedef_symbol(type);
    assert(sym); // Initialization guarantee
    paramDic.typeId = sym->type_id;

    paramDic.isHidden = false; // TODO: not implemented
    paramDic.isGenerated = false; // TODO: not implemented
    paramDic.numCumulated = 0; //TODO: not implemented
    metaRows.paramDic.push_back(paramDic);

    for (auto lang : Symbol::pp_all_languages) {
        ParamDicTxtLangRow paramTxt;
        paramTxt.paramId = pp_parameter_id;
        paramTxt.langName = lang->name;
        paramTxt.langId = lang->language_id;
        paramTxt.descr = label(*lang);
        paramTxt.note = note(*lang);
        metaRows.paramTxt.push_back(paramTxt);
    }
}

