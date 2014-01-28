/**
 * @file    ParseContext.h
 * Declares the parse context class.
 * 
 * A single instance of the ParseContext class manages
 * context-dependent information during parsing.  This instance
 * is accessible to the parser, the scanner, and the driver.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

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
        , cxx_memfunc_gather (false)
        , cxx_memfunc_name ("")
        , agent_context (nullptr)
        , table_context (nullptr)
        , classification_context(nullptr)
        , partition_context(nullptr)
        , parameter_context(nullptr)
        , parse_errors(0)
    {
        // The default location constructor for comment_location is fine.
    }

    void set_agent_context( Symbol *agent )
    {
        if ( agent != nullptr ) {
            agent_context = &agent->get_rpSymbol();
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

    void set_table_context( Symbol *table )
    {
        if ( table != nullptr ) {
            table_context = &table->get_rpSymbol();
        }
        else {
            table_context = nullptr;
        }
    }

    Symbol * get_table_context( )
    {
        if ( table_context != nullptr ) {
            return *table_context;
        }
        else {
            return nullptr;
        }
    }

    void set_classification_context(Symbol *classification)
    {
        if (classification != nullptr) {
            classification_context = &classification->get_rpSymbol();
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
            partition_context = &partition->get_rpSymbol();
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

    void set_parameter_context(Symbol *parameter)
    {
        if (parameter != nullptr) {
            parameter_context = &parameter->get_rpSymbol();
        }
        else {
            parameter_context = nullptr;
        }
    }

    Symbol * get_parameter_context()
    {
        if (parameter_context != nullptr) {
            return *parameter_context;
        }
        else {
            return nullptr;
        }
    }

    /**
     * Initialize parse context for outermost code level.
     */

    void InitializeForCxxOutside()
    {
        brace_level = 0;
        parenthesis_level = 0;
        bracket_level = 0;
        counter1 = 0;
        counter2 = 0;
        counter3 = 0;
        agent_context = nullptr;
        table_context = nullptr;
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

        InitializeForCxxOutside();
    }

    /**
     * comment location.
     */

    yy::location comment_location;

    /**
     * comment body
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
     * true to gather identifiers from C++ member function body
     */

    bool cxx_memfunc_gather;

    /**
     * Qualified name of the C++ member function.
     * Example: "Person::MortalityEvent"
     */

    string cxx_memfunc_name;

	/**
	 * A count of the number of omc parse errors in the model source files.
	 */

	int parse_errors;

private:

    /**
     * agent context for contained symbols
     * 
     * Note that this cannot be Symbol &*, since
     * the value can be nullptr.
     */

	Symbol **agent_context;

    /**
     * table context for contained symbols
     * 
     * Note that this cannot be Symbol &*, since
     * the value can be nullptr.
     */

	Symbol **table_context;

    /**
    * classification context for contained symbols
    *
    * Note that this cannot be Symbol &*, since
    * the value can be nullptr.
    */

    Symbol **classification_context;


    /**
    * partition context for contained literals
    *
    * Note that this cannot be Symbol &*, since
    * the value can be nullptr.
    */

    Symbol **partition_context;


    /**
     * parameter context for contained symbols
     */

    Symbol **parameter_context;
};
