/**
* @file    EntityTableSymbol.h
* Declares the EntityTableSymbol derived class of the Symbol class.
* Also declares related classes.
*/
// Copyright (c) 2013-2022 OpenM++ Contributors
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
class GlobalFuncSymbol;
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

    /**
     * Enumeration indicating the kind of entity table.
     */
    enum class table_kind {
        ///< general entity table.
        general,
        ///< snapshot-style entity table.
        snapshot,
    };

    EntityTableSymbol(Symbol *sym, const Symbol * ent, omc::location decl_loc = omc::location())
        : TableSymbol(sym, decl_loc)
        , entity(ent->stable_rp())
        , pp_entity(nullptr)
        , increment(nullptr)
        , current_cell_fn(nullptr)
        , init_increment_fn(nullptr)
        , push_increment_fn(nullptr)
        , filter(nullptr)
        , n_collections(0)
        , kind(table_kind::general)
        , default_statistic(token::TK_sum)
        , default_increment(token::TK_delta)
        , default_tabop(token::TK_interval)
        , is_untransformed(false)
        , is_screened1(false)
        , resource_use_gfn(nullptr)
        , resource_use_reset_gfn(nullptr)
    {
        cxx_class = "cls_" + name;
        increments_gvn = name + "_increments";

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
     * Reference to pointer to entity.
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& entity;

    /**
     * Direct pointer to entity.
     * 
     * For use post-parse.
     */
    EntitySymbol *pp_entity;

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
     * The expression attribute of the table filter.
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

    /** The kind of table */
    table_kind kind;

    /** The default statistic */
    token_type default_statistic;

    /** The default increment */
    token_type default_increment;

    /** The default tabop */
    token_type default_tabop;

    /**
     * Query if this EntityTableSymbol is general
     *
     * @returns True if classic, false if not.
     */
    bool is_general(void) const
    {
        return kind == table_kind::general;
    }

    /**
     * Query if this EntityTableSymbol is snapshot
     *
     * @returns True if snapshot, false if not.
     */
    bool is_snapshot(void) const
    {
        return kind == table_kind::snapshot;
    }

    /**
     * The default statistic for this table
     *
     * @returns A token_type.
     */
    token_type get_default_statistic(void) const
    {
        return default_statistic;
    }

    /**
     * Set the default statistic for this table
     */
    void set_default_statistic(token_type val)
    {
        default_statistic = val;
    }

    /**
     * The default increment for this table
     *
     * @returns A token_type.
     */
    token_type get_default_increment(void) const
    {
        return default_increment;
    }

    /**
     * Set the default increment for this table
     */
    void set_default_increment(token_type val)
    {
        default_increment = val;
    }

    /**
     * The default tabop for this table
     *
     * @returns A token_type.
     */
    token_type get_default_tabop(void) const
    {
        return default_tabop;
    }

    /**
     * Set the default tabop for this table
     */
    void set_default_tabop(token_type val)
    {
        default_tabop = val;
    }

    const char * kind_as_string(void) const
    {
        switch (kind) {
        case table_kind::general: return "general";
        case table_kind::snapshot: return "snapshot";
        default: return ""; // not reached
        }
    }

    /**
     * True if the table is unweighted, unscaled, and aggregated across subs.
     */
    bool is_untransformed;

    /**
     * True if the table is confidentiality screened, using method 1
     */
    bool is_screened1;

    /**
     * Class name used to declare the entity table.
     */
    string cxx_class;

    /**
     * The name of the global variable for the number of increments pushed to the entity table.
     */
    std::string increments_gvn;

    /**
     * The global function which reports resource use for the entity table.
     */
    GlobalFuncSymbol* resource_use_gfn;

    /**
     * The global function which resets resource use for the entity table.
     */
    GlobalFuncSymbol* resource_use_reset_gfn;
};
