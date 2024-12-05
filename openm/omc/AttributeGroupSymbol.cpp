/**
* @file    AttributeGroupSymbol.cpp
* Definitions for the AttributeGroupSymbol class.
*/
// Copyright (c) 2013-2024 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "AttributeGroupSymbol.h"
#include "EntitySymbol.h"
#include "LanguageSymbol.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;

void AttributeGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to entity for use post-parse
        pp_entity = dynamic_cast<EntitySymbol*> (pp_symbol(entity));
        if (!pp_entity) {
            pp_error(LT("error : '") + entity->name + LT("' is not an entity"));
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

}

/*
* Prototype of
*   populate_metadata()
* for entity attribute groups
*
* Entity attribute groups are similar to parameters and output tables groups
*
void EntityAttributeGroupSymbol::populate_metadata(openm::MetaModelHolder& metaRows)
{
    using namespace openm;

    // Hook into the hierarchical calling chain
    super::populate_metadata(metaRows);

    // Perform operations specific to this level in the Symbol hierarchy.

    // groups of entity attributes
    EntityGroupLstRow groupRow;

    // basic information about the group
    groupRow.entityId = pp_entity_id;
    groupRow.groupId = pp_group_id;
    groupRow.name = name;
    groupRow.isHidden = is_hidden;
    metaRows.entityGroupLst.push_back(groupRow);

    // labels and notes for the group
    for (const auto & langSym : Symbol::pp_all_languages) {
        const string & lang = langSym->name; // e.g. "EN" or "FR"
        EntityGroupTxtLangRow groupTxt;
        groupTxt.entityId = pp_entity_id;
        groupTxt.groupId = pp_group_id;
        groupTxt.langCode = lang;
        groupTxt.descr = label(*langSym);
        groupTxt.note = note(*langSym);
        metaRows.entityGroupTxt.push_back(groupTxt);
    }

    // group children
    int childPos = 1;   // child position in the group, must be unique

    for (auto sym : pp_symbol_list) {
        auto ags = dynamic_cast<EntityAttributeGroupSymbol *>(sym);
        if (ags) {
            EntityGroupPcRow groupPc;
            groupPc.entityId = pp_entity_id;
            groupPc.groupId = pp_group_id;
            groupPc.childPos = childPos++;
            groupPc.childGroupId = ags->pp_group_id;
            groupPc.attrId = -1;            // negative value treated as db-NULL
            metaRows.entityGroupPc.push_back(groupPc);

            continue;   // done with this child group
        }
        // else symbol is an entity attribute
        auto attr = dynamic_cast<AttributeSymbol *>(sym);
        if (attr) {
            EntityGroupPcRow groupPc;
            groupPc.entityId = pp_entity_id;
            groupPc.groupId = pp_group_id;
            groupPc.childPos = childPos++;
            groupPc.childGroupId = -1;      // negative value treated as db-NULL
            groupPc.attrId = attr->pp_attribute_id;
            metaRows.entityGroupPc.push_back(groupPc);

            continue;   // done with this child attribute
        }
        else {
            // invalid entity attribute group member
            // (previously detected error condition)
        }
    }
}

*/

