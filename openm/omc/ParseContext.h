/**
 * @file    ParseContext.h
 * Declares the parse context class.
 * 
 * A single instance of the ParseContext class manages
 * context-dependent information during parsing.  This instance
 * is accessible to the parser, the scanner, and the driver.
 */
// Copyright (c) 2013-2023 OpenM++ Contributors (see AUTHORS.txt)
// This code is licensed under the MIT license (see LICENSE.txt)

#pragma once
#include "libopenm/omLog.h"
#include "libopenm/common/omHelper.h"
#include "omc_location.hh"
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
        , all_line_count(0)
        , island_line_count(0)
        , counter1 (0)
        , counter2 (0)
        , counter3 (0)
        , counter4 (0)
        , cxx_function_gather (false)
        , cxx_function_name ("")
        , parse_errors(0)
        , parse_warnings(0)
        , next_word_is_string(false)
        , redeclaration(false)
        , is_fixed_parameter_value(false)
        , is_scenario_parameter_value(false)
        , entity_context (nullptr)
        , table_context (nullptr)
        , derived_table_context (nullptr)
        , entity_set_context (nullptr)
        , classification_context(nullptr)
        , partition_context(nullptr)
        , parameter_context(nullptr)
    {
        // The default location constructor for *_location members is fine.
    }

    void reset_working_counters()
    {
        counter1 = 0;
        counter2 = 0;
        counter3 = 0;
        counter4 = 0;
    }

    void set_entity_context( Symbol * ent)
    {
        if (ent != nullptr ) {
            entity_context = &ent->stable_rp();
        }
        else {
            entity_context = nullptr;
        }
    }

    Symbol * get_entity_context( )
    {
        if (entity_context != nullptr ) {
            return *entity_context;
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
        entity_context = nullptr;
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
        syntactic_island_location.begin.initialize();
        syntactic_island_location.end.initialize();
        innermost_opening_brace_location.begin.initialize();
        innermost_opening_brace_location.end.initialize();
        innermost_opening_parenthesis_location.begin.initialize();
        innermost_opening_parenthesis_location.end.initialize();
        innermost_opening_bracket_location.begin.initialize();
        innermost_opening_bracket_location.end.initialize();
        all_line_count = 0;
        island_line_count = 0;
        comment_body = "";
        literal_length = 0;
        literal_specification = "";
        cxx_function_gather = false;
        cxx_function_name = "";
        cxx_tokens.clear();

        InitializeForCxx();
    }

    /**
     * Handle parse error with no code location
     *
     * @param m The error message
     */
    void error(const string & m)
    {
        parse_errors++;
        theLog->logFormatted("%s", m.c_str());
    }

    /**
     * Handle parse error with code location.
     *
     * @param l The location
     * @param m The error message
     */
    void error(const omc::location & l, const string & m)
    {
        parse_errors++;
        theLog->logFormatted("%s(%d) : %s", l.begin.filename->c_str(), l.begin.line, m.c_str());
    }

    /**
     * Handle parse warning with code location.
     *
     * @param l The location
     * @param m The warning message
     */
    void warning(const omc::location & l, const string & m)
    {
        parse_warnings++;
        theLog->logFormatted("%s(%d) : %s", l.begin.filename->c_str(), l.begin.line, m.c_str());
    }

    /**
     * Handle parse note with code location.
     *
     * @param l The location
     * @param m The note message
     */
    void note(const omc::location& l, const string& m)
    {
        theLog->logFormatted("%s(%d) : %s", l.begin.filename->c_str(), l.begin.line, m.c_str());
    }

    /**
     * Process a token in C++ code.
     *
     * Return possibly modified version of tok_str
     */
    string cxx_process_token(token_type tok, const string yytext, omc::location * loc);

    /**
     * Process a C++ single-line comment.
     *
     * @param cmt The comment.
     * @param loc The source code location.
     */
    void process_cxx_comment(const string& cmt, const omc::location& loc);

    /**
     * Process a C-style comment.
     *
     * @param cmt The comment.
     * @param loc The source code location.
     */
    void process_c_comment(const string& cmt, const omc::location& loc);

    /**
     * comment location.
     */
    omc::location comment_location;

    /**
     * syntactic island start location.
     */
    omc::location syntactic_island_location;

    /**
     * innermost opening brace location.
     *
     * For error reporting.
     */
    omc::location innermost_opening_brace_location;

    /**
     * innermost opening parenthesis location.
     *
     * For error reporting.
     */
    omc::location innermost_opening_parenthesis_location;

    /**
     * innermost opening bracket location.
     *
     * For error reporting.
     */
    omc::location innermost_opening_bracket_location;

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
     * Line count of all input.
     */
    int all_line_count;

    /**
     * Line count of syntactic islands.
     */
    int island_line_count;

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
    bool cxx_function_gather;

    /**
     * Qualified name of the C++ member function. Example: "Person::MortalityEvent".
     */
    string cxx_function_name;

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
     * The ompp language permits redeclaration isncertain contexts, in particular entities can be freely
     * redeclared, and the attribute specifications are gathered together.  Parameters can be
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
     * entity context for contained symbols
     * 
     * Note that this cannot be Symbol &amp;*, since the value can be nullptr.
     */
	Symbol **entity_context;

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

    /**
     * list of tokens in C++ code
     */
    list<pair<token_type,string>> cxx_tokens;
};
