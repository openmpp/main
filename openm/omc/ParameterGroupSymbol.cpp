/**
* @file    ParameterGroupSymbol.cpp
* Definitions for the ParameterGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ParameterGroupSymbol.h"
#include "ParameterSymbol.h"
#include "LanguageSymbol.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;

void ParameterGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // check that group members are allowed types
        // and check if all parameters are fixed
        for (auto sym : pp_symbol_list) {
            bool is_parameter = dynamic_cast<ParameterSymbol*>(sym);
            bool is_parameter_group = dynamic_cast<ParameterGroupSymbol*>(sym);
            if (!(is_parameter || is_parameter_group)) {
                pp_error(LT("error : invalid member '") + sym->name + LT("' of parameter group '") + name + LT("'"));
            }
        }
        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of parameter groups
        pp_all_parameter_groups.push_back(this);
        // assign reverse link from each member of group to this group
        for (auto child : pp_symbol_list) {
            child->pp_parent_groups.insert(this);
        }
        break;
    }
    default:
        break;
    }
}

void ParameterGroupSymbol::populate_metadata(openm::MetaModelHolder& metaRows)
{
    using namespace openm;

    if (!contains_scenario_parameter()) {
        // do not publish a parameter group which contains only non-scenario parameters
        return;
    }

    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    GroupLstRow groupRow;

    // basic information about the group
    groupRow.groupId = pp_group_id;
    groupRow.isParam = true;        // group of parameters
    groupRow.name = name;
    groupRow.isHidden = is_hidden;
    metaRows.groupLst.push_back(groupRow);

    // labels and notes for the group
    for (auto lang : Symbol::pp_all_languages) {
        GroupTxtLangRow groupTxt;
        groupTxt.groupId = pp_group_id;
        groupTxt.langCode = lang->name;
        groupTxt.descr = label(*lang);
        groupTxt.note = note(*lang);
        metaRows.groupTxt.push_back(groupTxt);
    }

    // group children
    int childPos = 1;   // child position in the group, must be unique

    for (auto sym : pp_symbol_list) {
        auto pgs = dynamic_cast<ParameterGroupSymbol *>(sym);
        if (pgs) {
            // element is a parameter group
            if (pgs->contains_scenario_parameter()) {

                GroupPcRow groupPc;
                groupPc.groupId = pp_group_id;
                groupPc.childPos = childPos++;
                groupPc.childGroupId = pgs->pp_group_id;
                groupPc.leafId = -1;            // negative value treated as db-NULL
                metaRows.groupPc.push_back(groupPc);

                continue;   // done with this child group
            }
            // else do not publish a parameter group which contains only non-scenario parameters
        }
        // else symbol is a parameter
        auto param = dynamic_cast<ParameterSymbol *>(sym);
        if (param) {
            if (param->source == ParameterSymbol::scenario_parameter) {

                GroupPcRow groupPc;
                groupPc.groupId = pp_group_id;
                groupPc.childPos = childPos++;
                groupPc.childGroupId = -1;      // negative value treated as db-NULL
                groupPc.leafId = param->pp_parameter_id;
                metaRows.groupPc.push_back(groupPc);

                continue;   // done with this child parameter
            }
            // else do not publish non-scenario parameter
        }
        else {
            // invalid parameter group member
            // (previously detected error condition)
        }
    }
}

bool ParameterGroupSymbol::contains_scenario_parameter() const
{
    for (auto sym : pp_symbol_list) {
        auto pgs = dynamic_cast<ParameterGroupSymbol*>(sym);
        if (pgs) {
            // element is a parameter group
            // The following line contains a recursive call
            if (pgs->contains_scenario_parameter()) {
                return true;
            }
        }
        else {
            // element is a parameter
            auto param = dynamic_cast<ParameterSymbol*>(sym);
            if (param) {
                if (param->source == ParameterSymbol::scenario_parameter) {
                    return true;
                }
            }
            else {
                // invalid parameter group member
                // (previously detected error condition)
                return true;
            }
        }
    }
    return false;
}

