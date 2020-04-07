/**
* @file    ImportSymbol.cpp
* Definitions for the ImportSymbol class.
*/
// Copyright (c) 2013-2019 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ImportSymbol.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;

void ImportSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        pp_target_param = dynamic_cast<ParameterSymbol*>(pp_symbol(target_param));
        if (!pp_target_param) {
            pp_error(LT("error : '") + target_param->name + LT("' is not a parameter"));
        }
        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of imports
        pp_all_imports.push_back(this);
        break;
    }
    default:
        break;
    }
}

void ImportSymbol::populate_metadata(openm::MetaModelHolder& metaRows)
{
    using namespace openm;
    ParamImportRow importRow;

    if (pp_target_param->source != ParameterSymbol::scenario_parameter) {
        // Do not publish import if the target parameter is not a scenario parameter
        pp_warning(LT("warning : target parameter '") + pp_target_param->name + LT("' is not a scenario parameter, import suppressed."));
        return;
    }

    importRow.paramId = pp_target_param->pp_parameter_id;
    importRow.fromName = upstream_table;
    importRow.fromModel = upstream_model;
    importRow.isSampleDim = sample_dimension_opt;

    metaRows.paramImport.push_back(importRow);
}

int ImportSymbol::counter = 0;
