/**
* @file    EntitySetSymbol.h
* Declarations for the EntitySetSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class AgentFuncSymbol;
class AgentInternalSymbol;
class AgentVarSymbol;
class IdentityAgentVarSymbol;
class CodeBlock;

/**
* EntitySetSymbol.
*
*/
class EntitySetSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    EntitySetSymbol(Symbol *sym, const Symbol *agent, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , agent(agent->stable_rp())
        , pp_agent(nullptr)
        , cell(nullptr)
        , update_cell_fn(nullptr)
        , insert_fn(nullptr)
        , erase_fn(nullptr)
        , filter(nullptr)
    {
        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * Builds the function body of the update_cell function.
     */
    void build_body_update_cell();

    /**
     * Builds the function body of the insert function.
     */
    void build_body_insert();

    /**
     * Builds the function body of the erase function.
     */
    void build_body_erase();

    /**
     * The rank of the entity set
     *
     * @return An int.
     */
    int rank();

    /**
     * The total number of cells in the agent set
     *
     * @return An int.
     */
    int cell_count();

    /**
     * Reference to pointer to agent.
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& agent;

    /**
     * Direct pointer to agent.
     * 
     * For use post-parse.
     */

    AgentSymbol *pp_agent;

    /**
     * The agentvar which will hold the active index into the entity set.
     */
    AgentInternalSymbol *cell;

    /**
     * The agent function which updates the active cell index using agentvars.
     */
    AgentFuncSymbol *update_cell_fn;

    /**
     * The agent function which inserts the agent into the entity set.
     */
    AgentFuncSymbol *insert_fn;

    /**
     * The agent function which removes the agent from the entity set.
     */
    AgentFuncSymbol *erase_fn;

    /**
     * The expression agentvar of the entity set filter.
     */
    IdentityAgentVarSymbol *filter;

    /**
     * List of dimensions (parse phase references to pointers)
     */
    list<Symbol **> dimension_list_agentvar;

    /**
     * List of dimensions (post-parse phase pointers)
     */
    list<AgentVarSymbol *> pp_dimension_list_agentvar;

    /**
     * List of dimensions (post-parse phase pointers)
     */
    list<EnumerationSymbol *> pp_dimension_list_enum;

};
