/**
* @file    AgentVarSymbol.h
* Declarations for the AgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentDataMemberSymbol.h"

class AgentFuncSymbol;

using namespace std;

class AgentVarSymbol : public AgentDataMemberSymbol
{
private:
    typedef AgentDataMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    AgentVarSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentDataMemberSymbol(sym, agent, type, decl_loc)
        , side_effects_fn(nullptr)
    {
        create_auxiliary_symbols();
    }

    AgentVarSymbol(const string member_name, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : AgentDataMemberSymbol(member_name, agent, type, decl_loc)
        , side_effects_fn(nullptr)
    {
        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this agentvar.
     */
    void create_auxiliary_symbols();

    /**
     * Change the data type in the post-parse phase
     *
     * @param type The new data type.
     */
    void change_data_type(TypeSymbol *new_type);

    /**
     * Creates the lagged version of this agentvar.
     * 
     * The lagged version is an AgentInternalSymbol which holds
     * the value of the agentvar at the end of the previous event.
     * An additional AgentInternaalSymbol holds the value of the
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

    void post_parse(int pass);

    /**
     * The side effects function of the agentvar.
     */
    AgentFuncSymbol *side_effects_fn;

};

