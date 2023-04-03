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

void ModelSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.

    switch (pass) {
    case ePopulateDependencies:
    {
        {
            // process option memory_popsize_parameter
            // Done in later pass to have parameter datatype nad source
            string param_name = Symbol::option_memory_popsize_parameter;
            if (param_name != "") {
                auto sym = Symbol::get_symbol(param_name);
                auto param = dynamic_cast<ParameterSymbol*>(sym);
                if (   !sym
                    || !param
                    || param->rank() != 0
                    || !param->pp_datatype->is_numeric_integer()
                    || param->source != ParameterSymbol::scenario_parameter
                    ) {
                    pp_error(LT("error : memory_popsize_parameter must be a scalar integer scenario parameter"));
                }
                else {
                    // store the popsize parameter in the ModelSymbol member
                    pp_memory_popsize_parameter = param;
                }
            }
        }
    }
    break;
    default:
        break;
    }
}


CodeBlock ModelSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    c += "namespace omr {";
    c +=     "const char * model_name = \"" + external_name + "\";";
    c +=     "const int memory_MB_constant_per_instance = " + std::to_string(memory_MB_constant_per_instance) + ";";
    c +=     "const int memory_MB_constant_per_sub = " + std::to_string(memory_MB_constant_per_sub) + ";";
    c +=     "const double memory_MB_popsize_coefficient = " + std::to_string(memory_MB_popsize_coefficient) + ";";
    c +=     "const double memory_safety_factor = " + std::to_string(memory_safety_factor) + ";";
    c += "} // namespace omr";
    c += "";

    return c;
}

void ModelSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
    metaRows.modelDic.name = external_name;

    // if memory prediction options defined then create default model profile memory options
    if (pp_memory_popsize_parameter && memory_MB_popsize_coefficient > 0) {
        metaRows.profileRows[MetaModelHolder::popSizeMemoryOption] = pp_memory_popsize_parameter->name;
        metaRows.profileRows[MetaModelHolder::popCoeffMemoryOption] = to_string(memory_MB_popsize_coefficient);
    }
    if (memory_MB_constant_per_instance > 0) {
        metaRows.profileRows[MetaModelHolder::constInstanceMemoryOption] = to_string(memory_MB_constant_per_instance);
    }
    if (memory_MB_constant_per_sub > 0) {
        metaRows.profileRows[MetaModelHolder::constSubMemoryOption] = to_string(memory_MB_constant_per_sub);
    }
    if (memory_safety_factor > 1.0 &&
        (pp_memory_popsize_parameter && memory_MB_popsize_coefficient > 0 ||
            memory_MB_constant_per_instance > 0 ||
            memory_MB_constant_per_sub > 0))
        {
            metaRows.profileRows[MetaModelHolder::safetyFactorMemoryOption] = to_string(memory_safety_factor);
        }

    // set language specific model description and notes
    for (auto lang : Symbol::pp_all_languages) {
        ModelDicTxtLangRow modelTxt;
        modelTxt.langCode = lang->name;
        modelTxt.descr = label(*lang);
        modelTxt.note = note(*lang);
        metaRows.modelTxt.push_back(modelTxt);
    }

    // set default model code: first language in the list of model languages
    if (!metaRows.modelTxt.empty()) metaRows.modelDic.defaultLangCode = metaRows.modelTxt[0].langCode;
};
