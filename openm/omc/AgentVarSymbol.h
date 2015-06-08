/**
* @file    AgentVarSymbol.h
* Declarations for the AgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentDataMemberSymbol.h"

class AgentInternalSymbol;
class AgentFuncSymbol;

using namespace std;

class AgentVarSymbol : public AgentDataMemberSymbol
{
private:
    typedef AgentDataMemberSymbol super;

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
    AgentVarSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentDataMemberSymbol(sym, agent, type, decl_loc)
        , lagged(nullptr)
        , lagged_event_counter(nullptr)
        , side_effects_fn(nullptr)
        , notify_fn(nullptr)
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
    AgentVarSymbol(const string member_name, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentDataMemberSymbol(member_name, agent, type, decl_loc)
        , side_effects_fn(nullptr)
        , notify_fn(nullptr)
    {
        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this agentvar.
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
     * Creates the lagged version of this agentvar.
     * 
     * The lagged version is an AgentInternalSymbol which holds
     * the value of the agentvar at the end of the previous event.
     * An additional AgentInternalSymbol holds the value of the
     * event counter at last update.
     */
    void create_lagged();

    /**
     * Gets the name of the lagged version of this agentvar.
     *
     * @return The symbol name.
     */
    string get_lagged_name();

    /**
     * Gets the name of the lagged event counter for this agentvar.
     *
     * Contains the value of the events counter when the lagged value
     * was last updated.
     * 
     * @return The symbol name.
     */
    string get_lagged_event_counter_name();

    /**
     * Determines if agentvar can be validly compared to the specifiec constant.
     *
     * @param constant     The constant.
     * @param [in,out] msg Generated error message
     *
     * @return true if valid comparison, false if not.
     */
    bool is_valid_comparison(const ConstantSymbol * constant, string &err_msg);

    void post_parse(int pass);

    /**
     * The lagged version of this attribute.
     * 
     * Created by calling create_lagged.
     */
    AgentInternalSymbol *lagged;

    /**
     * The value of the global event counter when the lagged version was last updated.
     * 
     * Created by calling create_lagged.
     */
    AgentInternalSymbol *lagged_event_counter;

    /**
     * The side effects function of the attribute.
     */
    AgentFuncSymbol *side_effects_fn;

    /**
     * The change notification function of the attribute.
     */
    AgentFuncSymbol *notify_fn;

};

