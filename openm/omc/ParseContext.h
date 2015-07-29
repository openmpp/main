/**
 * @file    ParseContext.h
 * Declares the parse context class.
 * 
 * A single instance of the ParseContext class manages
 * context-dependent information during parsing.  This instance
 * is accessible to the parser, the scanner, and the driver.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "location.hh"
#include "Symbol.h"

class ParseContext
{
public:
    /// Construct a ParseContext
    ParseContext()
        : comment_body ("")
        , literal_length (0)
        , literal_specification ("")
        , brace_level (0)
        , parenthesis_level (0)
        , bracket_level (0)
        , cxx_suppress (false)
        , counter1 (0)
        , counter2 (0)
        , counter3 (0)
        , counter4 (0)
        , cxx_memfunc_gather (false)
        , cxx_memfunc_name ("")
        , parse_errors(0)
        , parse_warnings(0)
        , next_word_is_string(false)
        , redeclaration(false)
        , agent_context (nullptr)
        , table_context (nullptr)
        , derived_table_context (nullptr)
        , entity_set_context (nullptr)
        , classification_context(nullptr)
        , partition_context(nullptr)
        , parameter_context(nullptr)
        , is_fixed_parameter_value(false)
        , is_scenario_parameter_value(false)
    {
        // The default location constructor for comment_location is fine.
    }

    void set_agent_context( Symbol *agent )
    {
        if ( agent != nullptr ) {
            agent_context = &agent->stable_rp();
        }
        else {
            agent_context = nullptr;
        }
    }

    Symbol * get_agent_context( )
    {
        if ( agent_context != nullptr ) {
            return *agent_context;
        }
        else {
            return nullptr;
        }
    }

    void set_table_context( EntityTableSymbol *table )
    {
        table_context = table;
    }

    EntityTableSymbol * get_table_context( )
    {
        assert(table_context);  // grammar/logic guarantee that requests only occur in valid table context
        return table_context;
    }

    void set_derived_table_context( DerivedTableSymbol *derived_table )
    {
        derived_table_context = derived_table;
    }

    DerivedTableSymbol * get_derived_table_context( )
    {
        assert(derived_table_context);  // grammar/logic guarantee that requests only occur in valid table context
        return derived_table_context;
    }

    void set_entity_set_context( EntitySetSymbol *entity_set )
    {
        entity_set_context = entity_set;
    }

    EntitySetSymbol * get_entity_set_context( )
    {
        assert(entity_set_context);  // grammar/logic guarantee that requests only occur in valid entity set context
        return entity_set_context;
    }

    void set_classification_context(Symbol *classification)
    {
        if (classification != nullptr) {
            classification_context = &classification->stable_rp();
        }
        else {
            classification_context = nullptr;
        }
    }

    Symbol * get_classification_context()
    {
        if (classification_context != nullptr) {
            return *classification_context;
        }
        else {
            return nullptr;
        }
    }

    void set_partition_context(Symbol *partition)
    {
        if (partition != nullptr) {
            partition_context = &partition->stable_rp();
        }
        else {
            partition_context = nullptr;
        }
    }

    Symbol * get_partition_context()
    {
        if (partition_context != nullptr) {
            return *partition_context;
        }
        else {
            return nullptr;
        }
    }

    void set_parameter_context(ParameterSymbol *parameter)
    {
        parameter_context = parameter;
    }

    ParameterSymbol * get_parameter_context()
    {
        assert(parameter_context);  // grammar/logic guarantee that requests only occur in valid parameter context
        return parameter_context;
    }

    /**
     * Initialize parse context for outermost code level, which is C++.
     */
    void InitializeForCxx()
    {
        brace_level = 0;
        parenthesis_level = 0;
        bracket_level = 0;
        counter1 = 0;
        counter2 = 0;
        counter3 = 0;
        counter4 = 0;
        redeclaration = false;
        next_word_is_string = false;
        agent_context = nullptr;
        table_context = nullptr;
        derived_table_context = nullptr;
        entity_set_context = nullptr;
        classification_context = nullptr;
        partition_context = nullptr;
        parameter_context = nullptr;
    }

    /**
     * Initialize parse contaxt for a new module.
     */
    void InitializeForModule()
    {
        comment_location.begin.initialize();
        comment_location.end.initialize();
        comment_body = "";
        literal_length = 0;
        literal_specification = "";
        cxx_suppress = false;
        cxx_memfunc_gather = false;
        cxx_memfunc_name = "";

        InitializeForCxx();
    }

    /**
     * comment location.
     */
    yy::location comment_location;

    /**
     * comment body.
     */
    string comment_body;

    /**
     * true length of string or character literal.
     */
    int literal_length;

    /**
     * string or character literal as specified in source code.
     */
    string literal_specification;

    /**
     * brace nesting level.
     */
	int brace_level;

    /**
     * parenthesis nesting level.
     */
	int parenthesis_level;

    /**
     * bracket nesting level.
     */
	int bracket_level;

    /**
     * Suppress C++ code found in model source until next outermost brace.
     */
   	bool cxx_suppress;

    /**
     * working counter #1 for parsing.
     */
	int counter1;

    /**
     * working counter #2 for parsing.
     */
	int counter2;

    /**
     * working counter #3 for parsing.
     */
	int counter3;

    /**
     * working counter #4 for parsing.
     */
	int counter4;

    /**
     * true to gather identifiers from C++ member function body.
     */
    bool cxx_memfunc_gather;

    /**
     * Qualified name of the C++ member function. Example: "Person::MortalityEvent".
     */
    string cxx_memfunc_name;

    /**
     * A count of the number of omc parse errors in the model source files.
     */
	int parse_errors;

    /**
     * A count of the number of omc parse warnings in the model source files.
     */
	int parse_warnings;

    /**
     * Tells the scanner to treat the immediately following word as a string rather than a symbol.
     * 
     * Used by the parser to suppress symbol resolution by the scanner, for the next word only.
     * Used for parsing pointer (link).
     * Used in expressions, because the lhs of "->" is not known in general.  In this case, the
     * nature of the rhs is resolved in a post-parse phase.
     * Used in derived tables, to avoid creating symbols for placeholder strings
     * 
     */
    bool next_word_is_string;

    /**
     * Indicates if context is a redeclaration of an enclosing symbol.
     * 
     * The om langauge permits redeclaration isncertain contexts, in particular agents can be freely
     * redeclared, and the agentvar specifications are gathered together.  Parameters can be
     * redeclared, once to give their properties and (optionally) a second time to provide values.
     */
    bool redeclaration;

    /**
     * True if parsing fixed parameter values.
     */
    bool is_fixed_parameter_value;

    /**
     * True if parsing parameter values specified in a scenario.
     */
    bool is_scenario_parameter_value;
private:

    /**
     * agent context for contained symbols
     * 
     * Note that this cannot be Symbol &amp;*, since the value can be nullptr.
     */
	Symbol **agent_context;

    /**
     * table context for symbols in table declaration
     * 
     * Safe to use direct pointers, since context guarantess that we are in a table declaration, so
     * the table has already been morphed to a EntityTableSymbol if not already done.
     */
	EntityTableSymbol *table_context;

    /**
     * derived table context for symbols in derived table declaration
     * 
     * Safe to use direct pointers, since context guarantess that we are in a table declaration, so
     * the table has already been morphed to a EntityTableSymbol if not already done.
     */
	DerivedTableSymbol *derived_table_context;

    /**
     * entity set context for symbols in entity set declaration
     * 
     * Safe to use direct pointers, since context guarantess that we are in an entity set declaration, so
     * the entity set has already been morphed to a EntitySetSymbol if not already done.
     */
	EntitySetSymbol *entity_set_context;

    /**
     * classification context for contained symbols
     * 
     * Note that this cannot be Symbol &amp;*, since the value can be nullptr.
     */
    Symbol **classification_context;

    /**
     * partition context for contained literals
     * 
     * Note that this cannot be Symbol &amp;*, since the value can be nullptr.
     */
    Symbol **partition_context;

    /**
     * parameter context for symbols in parameter declaration
     * 
     * Safe to use direct pointers, since context guarantess that we are in a parameter declaration,
     * so the parameter has already been morphed to a ParameterSymbol if not already done.
     */
    ParameterSymbol *parameter_context;
};
