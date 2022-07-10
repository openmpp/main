/**
* @file    AttributeSymbol.h
* Declarations for the AttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "EntityDataMemberSymbol.h"

class EntityInternalSymbol;
class EntityFuncSymbol;

using namespace std;

class AttributeSymbol : public EntityDataMemberSymbol
{
private:
    typedef EntityDataMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Morphing constructor
     *
     * @param [in,out] sym If non-null, the symbol.
     * @param agent        The agent.
     * @param type         The type.
     * @param decl_loc     (Optional) the declaration location.
     */
    AttributeSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, omc::location decl_loc = omc::location())
        : EntityDataMemberSymbol(sym, agent, type, decl_loc)
        , lagged(nullptr)
        , lagged_event_counter(nullptr)
        , side_effects_fn(nullptr)
        , notify_fn(nullptr)
        , is_time_like(false)
    {
        create_auxiliary_symbols();
    }

    /**
     * Constructor by name
     *
     * @param member_name Name of the member.
     * @param agent       The agent.
     * @param type        The type.
     * @param decl_loc    (Optional) the declaration location.
     */
    AttributeSymbol(const string member_name, const Symbol *agent, const Symbol *type, omc::location decl_loc = omc::location())
        : EntityDataMemberSymbol(member_name, agent, type, decl_loc)
        , lagged(nullptr)
        , lagged_event_counter(nullptr)
        , side_effects_fn(nullptr)
        , notify_fn(nullptr)
        , is_time_like(false)
        , pp_attribute_id(-1)
    {
        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this attribute.
     */
    void create_auxiliary_symbols();

    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();

    /**
     * Change the data type in the post-parse phase.
     *
     * @param [in,out] new_type The new data type.
     */
    void change_data_type(TypeSymbol *new_type);

    /**
     * Creates the lagged version of this attribute.
     * 
     * The lagged version is an EntityInternalSymbol which holds
     * the value of the attribute at the end of the previous event.
     * An additional EntityInternalSymbol holds the value of the
     * event counter at last update.
     */
    void create_lagged();

    /**
     * Gets the name of the lagged version of this attribute.
     *
     * @return The symbol name.
     */
    string get_lagged_name();

    /**
     * Gets the name of the lagged event counter for this attribute.
     *
     * Contains the value of the events counter when the lagged value
     * was last updated.
     * 
     * @return The symbol name.
     */
    string get_lagged_event_counter_name();

    /**
     * Determines if attribute can be validly compared to the specifiec constant.
     *
     * @param constant     The constant.
     * @param [in,out] msg Generated error message
     *
     * @return true if valid comparison, false if not.
     */
    bool is_valid_comparison(const ConstantSymbol * constant, string &err_msg);

    /**
     * Determines if attribute is a maintained attribute.
     *
     * @return true if valid comparison, false if not.
     */
    bool is_maintained() const;

    void post_parse(int pass);

    /**
     * The lagged version of this attribute.
     * 
     * Created by calling create_lagged.
     */
    EntityInternalSymbol *lagged;

    /**
     * The value of the global event counter when the lagged version was last updated.
     * 
     * Created by calling create_lagged.
     */
    EntityInternalSymbol *lagged_event_counter;

    /**
     * The side effects function of the attribute.
     */
    EntityFuncSymbol *side_effects_fn;

    /**
     * The change notification function of the attribute.
     */
    EntityFuncSymbol *notify_fn;

    /**
     * The attribute can change between events, like time.
     */
    bool is_time_like;

    /**
     * The numeric identifier for the attribute.
     */
    int pp_attribute_id;
};

