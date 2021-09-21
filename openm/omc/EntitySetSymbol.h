/**
* @file    EntitySetSymbol.h
* Declarations for the EntitySetSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class EntityFuncSymbol;
class EntityInternalSymbol;
class AttributeSymbol;
class IdentityAttributeSymbol;
class DimensionSymbol;
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
     * The number of dimensions in the entity set (rank).
     *
     * @return A size_t.
     */
    size_t dimension_count() const
    {
        return dimension_list.size();
    }
    /**
     * The total number of cells in the agent set
     *
     * @return An int.
     */
    size_t cell_count() const;

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

    EntitySymbol *pp_agent;

    /**
     * The attribute which will hold the active index into the entity set.
     */
    EntityInternalSymbol *cell;

    /**
     * The agent function which updates the active cell index using attributes.
     */
    EntityFuncSymbol *update_cell_fn;

    /**
     * The agent function which inserts the agent into the entity set.
     */
    EntityFuncSymbol *insert_fn;

    /**
     * The agent function which removes the agent from the entity set.
     */
    EntityFuncSymbol *erase_fn;

    /**
     * The expression attribute of the entity set filter.
     */
    IdentityAttributeSymbol *filter;

    /**
     * List of dimensions
     */
    list<DimensionSymbol *> dimension_list;
};
