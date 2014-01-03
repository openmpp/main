/**
* @file    ParameterSymbol.h
* Definitions for the ParameterSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "ParameterSymbol.h"
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
    case 1:
        // add this parameter to the complete list of parameters
        pp_parameters.push_back(this);
        break;
    default:
        break;
    }
}

CodeBlock ParameterSymbol::cxx_declaration()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration();

    // Perform operations specific to this level in the Symbol hierarchy.
    string parm_type = Symbol::token_to_string(type);
    h += "extern " + parm_type + " " + name + ";";
    return h;
}

CodeBlock ParameterSymbol::cxx_definition()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_declaration();

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
    // 
    ParamDicRow paramDic;
    ParamDicTxtLangRow paramTxt;

    paramDic.paramId = pp_numeric_id;
    paramDic.paramName = name;  // must be valid database view name, if we want to use compatibility views
    paramDic.rank = 0; // TODO: currently hard-coded to scalar parmaeters for alpha
    paramDic.typeId = 12;           // TODO: hard-coded to double
    paramDic.isHidden = false; // TODO: not implemented
    paramDic.isGenerated = false; // TODO: not implemented
    paramDic.numCumulated = 0; //TODO: not implemented
    metaRows.paramDic.push_back(paramDic);

    // TODO language entries are hard-coded bilingual for alpha
    paramTxt.paramId = pp_numeric_id;
    paramTxt.langName = "EN";
    paramTxt.descr = name + " short name (EN)";
    paramTxt.note = name + " note (EN)";
    metaRows.paramTxt.push_back(paramTxt);

    paramTxt.langName = "FR";
    paramTxt.descr = name + " short name (FR)";
    paramTxt.note = name + " note (FR)";
    metaRows.paramTxt.push_back(paramTxt);

    // TODO: rank 0 parameters in alpha have no ParamDimsRows entries
}

