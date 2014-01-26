/**
* @file    ParameterSymbol.h
* Definitions for the ParameterSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ParameterSymbol.h"
#include "LanguageSymbol.h"
#include "TypedefTypeSymbol.h"
#include "ClassificationSymbol.h"
#include "RangeSymbol.h"
#include "PartitionSymbol.h"
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
            // assign direct pointer to type symbol for use post-parse
            pp_type_symbol = dynamic_cast<TypeSymbol *>(pp_symbol(type_symbol));

            if (!pp_type_symbol) {
                pp_error("Invalid type '" + type_symbol->name + "' for parameter '" + name + "'");
            }

            if (dynamic_cast<ClassificationSymbol *>(type_symbol)
                || dynamic_cast<RangeSymbol *>(type_symbol)
                || dynamic_cast<PartitionSymbol *>(type_symbol)
                ) {
                pp_is_enum = true;
            }
            else {
                pp_is_enum = false;
            }

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
    h += "extern " + type_symbol->name + " " + name + ";";
    return h;
}

CodeBlock ParameterSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += type_symbol->name + " " + name + " = " + pp_type_symbol->default_initial_value() + ";";
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
    paramDic.typeId = pp_type_symbol->type_id;

    paramDic.isHidden = false; // TODO: not implemented
    paramDic.isGenerated = false; // TODO: not implemented
    paramDic.numCumulated = 0; //TODO: not implemented
    metaRows.paramDic.push_back(paramDic);

    for (auto lang : Symbol::pp_all_languages) {
        ParamDicTxtLangRow paramTxt;
        paramTxt.paramId = pp_parameter_id;
        paramTxt.langName = lang->name;
        paramTxt.descr = label(*lang);
        paramTxt.note = note(*lang);
        metaRows.paramTxt.push_back(paramTxt);
    }
}

string ParameterSymbol::cxx_read_parameter()
{
    string typ; // storage type
    if (pp_is_enum) {
        // for parameters of type classification, range, or partition
        // get the underlying storage type
        auto ens = dynamic_cast<EnumTypeSymbol *>(pp_type_symbol);
        assert(ens); // grammar guarantee
        typ = Symbol::token_to_string(ens->storage_type);
    }
    else {
        // For fundamental types (and bool), the name of the symbol is the name of the type
        typ = type_symbol->name;
    }

    string result = "readParameter(\"" + name + "\", typeid(" + typ + "),  1, &" + name + ");";
    return result;
}

string ParameterSymbol::cxx_assert_sanity()
{
    string typ; // storage type
    if (pp_is_enum) {
        // for parameters of type classification, range, or partition
        // get the underlying storage type
        auto ens = dynamic_cast<EnumTypeSymbol *>(pp_type_symbol);
        assert(ens); // grammar guarantee
        typ = Symbol::token_to_string(ens->storage_type);
    }
    else {
        // For fundamental types (and bool), the name of the symbol is the name of the type
        typ = type_symbol->name;
    }

    string result = "assert(sizeof(" + name + ") == sizeof(" + typ + "));" ;
    return result;
}
