/**
* @file    AttributeGroupSymbol.cpp
* Definitions for the AttributeGroupSymbol class.
*/
// Copyright (c) 2013-2024 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "AttributeGroupSymbol.h"
#include "ModuleSymbol.h"
#include "EntitySymbol.h"
#include "LanguageSymbol.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;

void AttributeGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to entity for use post-parse
        pp_entity = dynamic_cast<EntitySymbol*> (pp_symbol(entity));
        if (!pp_entity) {
            pp_fatal(LT("error : '") + entity->name + LT("' is not an entity"));
        }
        break;
    }
    case eResolveDataTypes:
    {
        // Expand module symbols in group
        // This is done in this pass, so that module symbols are expanded
        // before group is used in subsequent pass ePopulateCollections,
        // e.g. to implement build-time retain, suppress, or hide.

        // copy of original list
        auto original = pp_symbol_list;
        pp_symbol_list.clear();
        for (auto sym : original) {
            auto mod = dynamic_cast<ModuleSymbol*>(sym);
            if (mod) {
                // push all attributes (with correct entity) in module
                for (auto mod_sym : mod->pp_symbols_declared) {
                    auto attr_sym = dynamic_cast<AttributeSymbol*>(mod_sym);
                    if (attr_sym && (attr_sym->pp_entity == pp_entity)) {
                        pp_symbol_list.push_back(mod_sym);
                    }
                }
            }
            else {
                pp_symbol_list.push_back(sym);
            }
        }
        original.clear();
        break;
    }
    case ePopulateCollections:
    {
        // Verify validity of attribute group members.
        // Can't be done in eAssignMembers pass, because entity of all attribute groups is needed to check member validity.
        for (auto sym : pp_symbol_list) {
            auto symbol_name = sym->name;
            AttributeGroupSymbol* sym_attribute_group = dynamic_cast<AttributeGroupSymbol*>(sym);
            bool is_attribute_group = sym_attribute_group;
            if (is_attribute_group) {
                // verify that group member has same entity as group
                if (pp_entity != sym_attribute_group->pp_entity) {
                    // entity does not match
                    is_attribute_group = false;
                }
            }
            bool is_attribute = dynamic_cast<AttributeSymbol*>(sym);
            bool is_module = dynamic_cast<ModuleSymbol*>(sym);;
            if (!(is_attribute_group || is_attribute || is_module)) {
                pp_error(LT("error : invalid member '") + symbol_name + LT("' of attribute group '") + name + LT("'"));
            }
        }

        // add this to the complete list of attribute groups
        pp_all_attribute_groups.push_back(this);
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

void AttributeGroupSymbol::populate_metadata(openm::MetaModelHolder& metaRows)
{
    using namespace openm;

    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.
 
    auto theEntityId = pp_entity->pp_entity_id;

    // groups of entity attributes
    EntityGroupLstRow groupRow;

    // basic information about the group
    groupRow.entityId = theEntityId;
    groupRow.groupId = pp_group_id;
    groupRow.name = name;
    groupRow.isHidden = is_hidden;
    metaRows.entityGroupLst.push_back(groupRow);

    // labels and notes for the group
    for (const auto& langSym : Symbol::pp_all_languages) {
        const string& lang = langSym->name; // e.g. "EN" or "FR"
        EntityGroupTxtLangRow groupTxt;
        groupTxt.entityId = theEntityId;
        groupTxt.groupId = pp_group_id;
        groupTxt.langCode = lang;
        groupTxt.descr = label(*langSym);
        groupTxt.note = note(*langSym);
        metaRows.entityGroupTxt.push_back(groupTxt);
    }

    // group children
    int childPos = 1;   // child position in the group, must be unique

    for (auto sym : pp_symbol_list) {
        auto ags = dynamic_cast<AttributeGroupSymbol*>(sym);
        if (ags) {
            EntityGroupPcRow groupPc;
            groupPc.entityId = theEntityId;
            groupPc.groupId = pp_group_id;
            groupPc.childPos = childPos++;
            groupPc.childGroupId = ags->pp_group_id;
            groupPc.attrId = -1;            // negative value treated as db-NULL
            metaRows.entityGroupPc.push_back(groupPc);

            continue;   // done with this child group
        }
        // else symbol is an entity attribute
        auto attr = dynamic_cast<AttributeSymbol*>(sym);
        if (attr) {
            EntityGroupPcRow groupPc;
            groupPc.entityId = theEntityId;
            groupPc.groupId = pp_group_id;
            groupPc.childPos = childPos++;
            groupPc.childGroupId = -1;      // negative value treated as db-NULL
            groupPc.attrId = attr->pp_member_id;
            metaRows.entityGroupPc.push_back(groupPc);

            continue;   // done with this child attribute
        }
        else {
            assert(false);
            // invalid entity attribute group member
            // (previously detected error condition)
        }
    }
}
