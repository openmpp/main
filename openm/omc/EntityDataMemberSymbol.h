/**
* @file    EntityDataMemberSymbol.h
* Declarations for the EntityDataMemberSymbol class.
*/
// Copyright (c) 2013-2022 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "EntityMemberSymbol.h"

class CodeBlock;

using namespace std;

class EntityDataMemberSymbol : public EntityMemberSymbol
{
private:
    typedef EntityMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }


    /**
     * Constructor.  Morph a generic symbol to a specialized symbol of this kind.
     *
     * @param [in,out] sym If non-null, the symbol.
     * @param ent          The entity.
     * @param data_type    The type.
     * @param decl_loc     (Optional) the declaration location.
     */

    EntityDataMemberSymbol(Symbol *sym, const Symbol * ent, const Symbol *data_type, omc::location decl_loc = omc::location())
        : EntityMemberSymbol(sym, ent, decl_loc)
        , data_type(data_type->stable_rp())
        , pp_data_type(nullptr)
        , parent(nullptr)
        , pp_parent(nullptr)
        , pp_eligible_microdata(false)
        , pp_member_id(-1)
    {
    }


    /**
     * Constructor.  Create a specialized symbol of this kind with a given name.
     *
     * @param member_name Name of the member.
     * @param ent         The entity.
     * @param data_type   The type.
     * @param decl_loc    (Optional) the declaration location.
     */

    EntityDataMemberSymbol(const string member_name, const Symbol * ent, const Symbol *data_type, omc::location decl_loc = omc::location())
        : EntityMemberSymbol(member_name, ent, decl_loc)
        , data_type(data_type->stable_rp())
        , pp_data_type(nullptr)
        , parent(nullptr)
        , pp_parent(nullptr)
        , pp_eligible_microdata(false)
        , pp_member_id(-1)
    {
        assert(data_type);  // grammar/initialization guarantee
    }

    /**
     * Determines if this is an AttributeSymbol
     *
     * @return true or false.
     */
    bool is_attribute(void) const;

    /**
     * Determines if this is a visible AttributeSymbol
     *
     * @return true or false.
     */
    bool is_visible_attribute(void) const;

    /**
     * Determines if this is eligible microdata
     *
     * @return true or false.
     */
    bool is_eligible_microdata(void) const;

    /**
     * Determines if this is published
     *
     * @return true or false.
     */
    bool is_published(void) const
    {
        return is_eligible_microdata();
    }

    /**
     * Determines if this is a BuiltinAttributeSymbol
     *
     * @return true or false.
     */
    bool is_builtin_attribute(void) const;

    /**
     * Determines if this is a LinkAttributeSymbol
     *
     * @return true or false.
     */
    bool is_link_attribute(void) const;

    /**
     * Determines if this is a MaintainedAttributeSymbol
     *
     * @return true or false.
     */
    bool is_maintained_attribute(void) const;

    /**
     * Determines if this is a IdentityAttributeSymbol
     *
     * @return true or false.
     */
    bool is_identity_attribute(void) const;

    /**
     * Determines if this is a DerivedAttributeSymbol
     *
     * @return true or false.
     */
    bool is_derived_attribute(void) const;

    /**
     * Determines if this is a SimpleAttributeSymbol
     *
     * @return true or false.
     */
    bool is_simple_attribute(void) const;

    /**
     * Determines if this is a MultilinkAttributeSymbol
     *
     * @return true or false.
     */
    bool is_multilink_aggregate_attribute(void) const;

    /**
     * Determines if this is a EntityArrayMemberSymbol
     *
     * @return true or false.
     */
    bool is_array(void) const;

    /**
     * Determines if this is a EntityEventSymbol
     *
     * @return true or false.
     */
    bool is_event(void) const;

    /**
     * Determines if this is a EntityIncrementSymbol
     *
     * @return true or false.
     */
    bool is_increment(void) const;

    /**
     * Determines if this is a EntityForeignMemberSymbol
     *
     * @return true or false.
     */
    bool is_foreign(void) const;

    /**
     * Determines if this is a EntityInternalSymbol
     *
     * @return true or false.
     */
    bool is_internal(void) const;

    /**
     * Determines if this is a EntityMultilinkSymbol
     *
     * @return true or false.
     */
    bool is_multilink(void) const;

    /**
    * Gets the layout group of the data member.
    * 
    * Data members are by default laid out in memory by layout group
    * to facilitate access in a visual debugger.
    *
    * @return A size_t.
    */
    virtual size_t layout_group() const;

    /**
    * Gets the alignment size of the data member.
    *
    * @return A size_t.
    */
    virtual size_t alignment_size() const;

    /**
     * Gets the initial value for the data member.
     *
     * @param type_default True to force use of the default value of the type.
     *
     * @return The initial value as a string.
     */
    virtual string initialization_value(bool type_default) const;

    /**
     * C++ initialization expression.
     *
     * @param type_default True to force use of the default value of the type.
     *
     * @return A CodeBlock.
     */
    virtual CodeBlock cxx_initialization_expression(bool type_default) const;


    CodeBlock cxx_declaration_entity();

    CodeBlock cxx_definition_entity();

    /** return C++ type name, raise an exception if type is a string type. */
    const string cxx_type_of(void) const;

    /**
     * Change the data type in the post-parse phase.
     *
     * @param [in,out] new_type The new data type.
     */
    virtual void change_data_type(TypeSymbol *new_type);

    void post_parse(int pass);

    /**
     * The datatype of the entity member (pointer to pointer)
     */
    Symbol*& data_type;

    /**
     * The datatype of the entity member (pointer)
     */
    TypeSymbol *pp_data_type;

    /**
     * The parent data member (optional, pointer to pointer)
     *
     * Refers to a member from which this member was derived
     */
    Symbol** parent;

    /**
     * The parent data member (optional)
     *
     * Refers to a member from which this member was derived
     */
    EntityDataMemberSymbol *pp_parent;

    /**
     * Is eligible as microdata
     */
    bool pp_eligible_microdata;

    /**
     * The numeric identifier for the member.
     */
    int pp_member_id;
};
