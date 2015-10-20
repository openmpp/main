/**
* @file    EntityTableSymbol.h
* Declares the EntityTableSymbol derived class of the Symbol class.
* Also declares related classes.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "TableSymbol.h"

class DimensionSymbol;
class EntityIncrementSymbol;
class EntityTableMeasureSymbol;
class EntityTableAccumulatorSymbol;
class EntityTableMeasureAttributeSymbol;
class EntityFuncSymbol;
class EntityInternalSymbol;
class AttributeSymbol;
class IdentityAttributeSymbol;
class BuiltinAttributeSymbol;
class CodeBlock;

using namespace std;



/**
* EntityTableSymbol.
*/
class EntityTableSymbol : public TableSymbol
{
private:
    typedef TableSymbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityTableSymbol(Symbol *sym, const Symbol *agent, yy::location decl_loc = yy::location())
        : TableSymbol(sym, decl_loc)
        , agent(agent->stable_rp())
        , pp_agent(nullptr)
        , increment(nullptr)
        , current_cell_fn(nullptr)
        , init_increment_fn(nullptr)
        , push_increment_fn(nullptr)
        , filter(nullptr)
        , n_collections(0)
    {
        cxx_class = "cls_" + name;

        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    string cxx_initializer() const;

    /**
     * Builds the function body of the current cell function.
     */
    void build_body_current_cell();

    /**
     * Builds the function body of the init increment function.
     */
    void build_body_init_increment();

    /**
     * Builds the function body of the push increment function.
     */
    void build_body_push_increment();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * The number of accumulators in the table.
     *
     * @return A size_t.
     */
    size_t accumulator_count() const
    {
        return pp_accumulators.size();
    }

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
     * The increment symbol for the table
     * 
     * For use post-parse.
     */
    EntityIncrementSymbol *increment;

    /**
     * The entity function which computes the active cell index using entity attributes.
     */
    EntityFuncSymbol *current_cell_fn;

    /**
     * The entity function which initializes the increment.
     */
    EntityFuncSymbol *init_increment_fn;

    /**
     * The entity function which finalizes the increment and pushes it to the accumulator.
     */
    EntityFuncSymbol *push_increment_fn;

    /**
     * The expression agentvar of the table filter.
     */
    IdentityAttributeSymbol *filter;

    /**
     * The attributes used in table measures.
     */
    list<EntityTableMeasureAttributeSymbol *> pp_measure_attributes;

    /**
     * The accumulators used in table measures.
     */
    list<EntityTableAccumulatorSymbol *> pp_accumulators;

    /**
     * The number of observation collections in the table.
     * 
     * Some accumulators such as P50 require an associated onservation collection
     * for each cell of the table.  This is the count of the number of such collections in each table cell.
     */
    int n_collections;

    /**
     * Class name used to declare the entity table.
     */
    string cxx_class;
};
