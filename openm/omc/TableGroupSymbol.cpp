/**
* @file    TableGroupSymbol.cpp
* Definitions for the TableGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "TableGroupSymbol.h"
#include "TableSymbol.h"
#include "ParameterSymbol.h"
#include "LanguageSymbol.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;

void TableGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // check that group members are allowed types
        for (auto sym : pp_symbol_list) {
            auto symbol_name = sym->name;
            bool is_table = dynamic_cast<TableSymbol*>(sym);
            bool is_table_group = dynamic_cast<TableGroupSymbol*>(sym);
            auto ps = dynamic_cast<ParameterSymbol*>(sym);
            bool is_derived_parameter = (ps && ps->is_derived());
            if (!(is_table || is_table_group || is_derived_parameter)) {
                pp_error(LT("error : invalid member '") + symbol_name + LT("' of table group '") + name + LT("'"));
            }
        }
        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of table groups
        pp_all_table_groups.push_back(this);
        // assign reverse link from each member of group to this group
        for (auto child : pp_symbol_list) {
            child->pp_parent_group = this;
        }
        break;
    }
    default:
        break;
    }
}

void TableGroupSymbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
    using namespace openm;

    if (!contains_published_table()) {
        // do not publish a table group which contains only non-published tables
        return;
    }

    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    GroupLstRow groupRow;

    // basic information about the group
    groupRow.groupId = pp_group_id;
    groupRow.isParam = false;       // group of output tables
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
        auto tgs = dynamic_cast<TableGroupSymbol *>(sym);
        if (tgs) {
            // element is a table group
            if (tgs->contains_published_table()) {
                GroupPcRow groupPc;
                groupPc.groupId = pp_group_id;
                groupPc.childPos = childPos++;
                groupPc.childGroupId = tgs->pp_group_id;
                groupPc.leafId = -1;            // negative value treated as db-NULL
                metaRows.groupPc.push_back(groupPc);

                continue;   // done with this child group
            }
            // else do not publish a table group which contains only non-published tables
        }
        // else symbol must be a table or a derived parameter marked metadata_as_table
        auto tbl = dynamic_cast<TableSymbol*>(sym);
        auto param = dynamic_cast<ParameterSymbol *>(sym);
        if (tbl) {
            if (!tbl->is_internal && !tbl->is_suppressed_table) {
                GroupPcRow groupPc;
                groupPc.groupId = pp_group_id;
                groupPc.childPos = childPos++;
                groupPc.childGroupId = -1;      // negative value treated as db-NULL
                groupPc.leafId = tbl->pp_table_id;
                metaRows.groupPc.push_back(groupPc);
                continue;   // done with this child table
            }
            // else do not publish this table
        }
        else if (param && param->metadata_as_table)  {
            GroupPcRow groupPc;
            groupPc.groupId = pp_group_id;
            groupPc.childPos = childPos++;
            groupPc.childGroupId = -1;      // negative value treated as db-NULL
            groupPc.leafId = param->pp_parameter_to_table_id;
            metaRows.groupPc.push_back(groupPc);
            continue;   // done with this child table
        }
        else {
            // invalid table group member
            // (previously detected error condition)
        }
    }
}

bool TableGroupSymbol::contains_published_table() const
{
    for (auto sym : pp_symbol_list) {
        auto tgs = dynamic_cast<TableGroupSymbol*>(sym);
        if (tgs) {
            // element is a table group
            // The following line contains a recursive call
            if (tgs->contains_published_table()) {
                return true;
            }
        }
        else {
            // element is a table
            auto tbl = dynamic_cast<TableSymbol*>(sym);
            if (tbl) {
                if (!tbl->is_internal && !tbl->is_suppressed_table) {
                    return true;
                }
            }
            else {
                // invalid table group member
                // (previously detected error condition)
                return true;
            }
        }
    }
    return false;
}

