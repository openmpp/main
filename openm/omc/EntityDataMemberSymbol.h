/**
* @file    EntityDataMemberSymbol.h
* Declarations for the EntityDataMemberSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
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
     * @param agent        The agent.
     * @param data_type    The type.
     * @param decl_loc     (Optional) the declaration location.
     */

    EntityDataMemberSymbol(Symbol *sym, const Symbol *agent, const Symbol *data_type, yy::location decl_loc = yy::location())
        : EntityMemberSymbol(sym, agent, decl_loc)
        , data_type(data_type->stable_rp())
        , pp_data_type(nullptr)
        , parent(nullptr)
        , pp_parent(nullptr)
    {
    }


    /**
     * Constructor.  Create a specialized symbol of this kind with a given name.
     *
     * @param member_name Name of the member.
     * @param agent       The agent.
     * @param data_type   The type.
     * @param decl_loc    (Optional) the declaration location.
     */

    EntityDataMemberSymbol(const string member_name, const Symbol *agent, const Symbol *data_type, yy::location decl_loc = yy::location())
        : EntityMemberSymbol(member_name, agent, decl_loc)
        , data_type(data_type->stable_rp())
        , pp_data_type(nullptr)
        , parent(nullptr)
        , pp_parent(nullptr)
    {
        assert(data_type);  // grammar/initialization guarantee
    }

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


    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();

    /**
     * Change the data type in the post-parse phase.
     *
     * @param [in,out] new_type The new data type.
     */
    virtual void change_data_type(TypeSymbol *new_type);

    void post_parse(int pass);

    /**
     * The datatype of the agent member (pointer to pointer)
     */
    Symbol*& data_type;

    /**
     * The datatype of the agent member (pointer)
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
};
