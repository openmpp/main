/**
* @file    TableSymbol.h
* Declares the TableSymbol derived class of the Symbol class.
* Also declares related classes.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "Symbol.h"

using namespace std;


/**
* TableExpressionSymbol.
*/
class TableExpressionSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    TableExpressionSymbol(Symbol *table, ExprForTable *root, int index)
        : Symbol(symbol_name(table, index))
        , root(root)
        , index(index)
        , table(table->get_rpSymbol())
        , pp_table(nullptr)
    {
    }

    // Construct symbol name for the table expression symbol.
    // Example: BasicDemography.Expr0
    static string symbol_name(const Symbol* table, int index)
    {
        assert(table);
        return table->name + ".Expr" + to_string(index);
    }


    void post_parse(int pass);

    void post_parse_traverse(ExprForTable *node);

    enum expression_style {
        cxx,
        sql
    };
    string get_expression(const ExprForTable *node, expression_style style);

    /**
    * Root of the expression tree
    */

    ExprForTable *root;

    /**
    * Zero-based index of the expression in the expression table dimension
    */

    int index;

    /**
    * The table containing this expression (reference to pointer)
    *
    * Stable to symbol morphing during parse phase.
    */

    Symbol*& table;

    /**
    * The table containing this expression (pointer)
    *
    * Only valid after post-parse phase 1.
    */

    TableSymbol* pp_table;
};

/**
* TableAccumulatorSymbol.
*/
class TableAccumulatorSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    TableAccumulatorSymbol(Symbol *table, token_type accumulator, token_type increment, Symbol *agentvar, Symbol *analysis_agentvar, int index)
        : Symbol(symbol_name(table, accumulator, increment, agentvar))
        , table(table->get_rpSymbol())
        , accumulator(accumulator)
        , increment(increment)
        , agentvar(agentvar->get_rpSymbol())
        , analysis_agentvar(analysis_agentvar->get_rpSymbol())
        , index(index)
        , pp_table(nullptr)
        , pp_agentvar(nullptr)
        , pp_analysis_agentvar(nullptr)
    {
        // grammar guarantee
        assert(accumulator == token::TK_sum
            || accumulator == token::TK_min
            || accumulator == token::TK_max
            );

        // grammar guarantee
        assert(increment == token::TK_delta
            || increment == token::TK_delta2
            || increment == token::TK_nz_delta
            || increment == token::TK_value_in
            || increment == token::TK_value_in2
            || increment == token::TK_nz_value_in
            || increment == token::TK_value_out
            || increment == token::TK_value_out2
            || increment == token::TK_nz_value_out
            );
    }

    /**
    * Get the unique name for this TableAccumulatorSymbol.
    *
    * @param   table       The table.
    * @param   accumulator The accumulator, e.g. token::TK_sum
    * @param   increment   The increment, e.g. token::TK_delta
    * @param   agentvar    The agentvar.
    *
    * @return  The name, e.g. om_ta_DurationOfLife_sum_delta_om_duration.
    */

    static string symbol_name(const Symbol *table, token_type accumulator, token_type increment, const Symbol *agentvar)
    {
        string result;
        result = "om_ta_" + table->name + "_" + token_to_string(accumulator) + "_" + token_to_string(increment) + "_" + agentvar->name;
        return result;
    }

    /**
    * Check for existence of symbol with this unique name.
    *
    * @param   table       The table.
    * @param   accumulator The accumulator, e.g. token::TK_sum.
    * @param   increment   The increment, e.g. token::TK_delta.
    * @param   agentvar    The agentvar.
    *
    * @return  true if found, else false.
    */

    static bool exists(const Symbol *table, token_type accumulator, token_type increment, const Symbol *agentvar)
    {
        string unm = symbol_name(table, accumulator, increment, agentvar);
        return symbols.count(unm) == 0 ? false : true;
    }

    void post_parse(int pass);

    virtual string pretty_name()
    {
        // example:     accumulator 0: sum(delta(duration))
        string result = " accumulator " + to_string(index) + ": " + token_to_string(accumulator) + "(" + token_to_string(increment) + "(" + agentvar->pretty_name() + "))";
        return result;
    }

    /**
    * The table containing this accumulator (reference to pointer)
    *
    * Stable to symbol morphing during parse phase.
    */

    Symbol*& table;

    token_type accumulator;

    token_type increment;

    /**
    * The agentvar being accumulated (reference to pointer)
    *
    * Stable to symbol morphing during parse phase.
    */

    Symbol*& agentvar;

    /**
    * The analysis agentvar being accumulated (reference to pointer)
    *
    * Stable to symbol morphing during parse phase.
    */

    Symbol*& analysis_agentvar;

    /**
    * Zero-based index of the accumulator within this table.
    */

    int index;

    /**
    * The table containing this accumulator (pointer)
    *
    * Only valid after post-parse phase 1.
    */

    TableSymbol* pp_table;

    /**
    * The agentvar being accumulated (pointer)
    *
    * Only valid after post-parse phase 1.
    */

    AgentVarSymbol* pp_agentvar;

    /**
    * The analysis agentvar being accumulated (pointer)
    *
    * Only valid after post-parse phase 1.
    */

    TableAnalysisAgentVarSymbol* pp_analysis_agentvar;
};

/**
* TableAnalysisAgentVarSymbol.
* Represents an agentvar used in the analysis dimension
* (also known as the expression dimension) of a table.
* An agentvar can be used multiple times in the analysis dimension of a table.
* If so, it is represented by a single instance of this class.
* This class handles the creation of an associated agentvar which holds the
* 'in' value of the agentvar when a table increment is started.
*/

class TableAnalysisAgentVarSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    TableAnalysisAgentVarSymbol(Symbol *table, Symbol *agentvar, int index)
        : Symbol(symbol_name(table, agentvar))
        , table(table->get_rpSymbol())
        , agentvar(agentvar->get_rpSymbol())
        , index(index)
        , need_value_in(false)
        , pp_table(nullptr)
        , pp_agentvar(nullptr)
    {
    }

    /**
    * Get the unique name for a specified symbol of this kind.
    *
    * @param   table       The table.
    * @param   agentvar    The agentvar used in the analysis dimension
    *
    * @return  The symbol name as a string.
    *          Example: om_taav_DurationOfLife_alive.
    */

    static string symbol_name(const Symbol *table, const Symbol *agentvar)
    {
        string result;
        result = "om_taav_" + table->name + "_" + agentvar->name;
        return result;
    }

    /**
    * Check for existence of a specific table analysis agentvar.
    *
    * @param   table       The table.
    * @param   agentvar    The agentvar.
    *
    * @return  true if found, else false.
    */

    static bool exists(const Symbol *table, const Symbol *agentvar)
    {
        string unm = symbol_name(table, agentvar);
        return symbols.count(unm) == 0 ? false : true;
    }

    void post_parse(int pass);

    /**
    * Gets the agentvar name for the 'in' value.
    * This is the name of the data member which holds the
    * "in" value of the agentvar.  It holds the value of the agentvar
    * at the start of an increment, and is used to compute 'delta', etc.
    * when the increment is finalized and passed to an accumulator.
    *
    * @return  The name as a string.
    */

    string in_agentvar_name() const
    {
        string result;
        result = "om_in_" + table->name + "_" + agentvar->name;
        return result;
    }

    CodeBlock cxx_prepare_increment() const;

    /**
    * The table using the agentvar
    *
    * Stored as a reference to a pointer, which is stable to symbol morphing during the parse phase.
    */

    Symbol*& table;

    /**
    * The agentvar used in the analysis dimension of the table
    *
    * Stored as a reference to a pointer, which is stable to symbol morphing during the parse phase.
    */

    Symbol*& agentvar;

    /**
    * Zero-based index of the analysis agentvar
    * in the list of all analysis agentvars used in the table.
    */

    const int index;

    /**
    * true means that generated code needs the member storing the "value_in" of the agentvar
    *
    * Only valid after post-parse phase 1.
    */

    bool need_value_in;

    /**
    * The table using the agentvar
    *
    * Stored as a pointer, which is only valid after post-parse phase 1.
    */

    TableSymbol* pp_table;

    /**
    * The agentvar used in the analysis dimension of the table.
    *
    * Stored as a pointer, which is only valid after post-parse phase 1.
    */

    AgentVarSymbol* pp_agentvar;

};

/**
* TableSymbol.
*/
class TableSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    TableSymbol(Symbol *sym, const Symbol *agent)
        : Symbol(sym)
        , agent(agent->get_rpSymbol())
    {
    }

    void post_parse(int pass)
    {
        // First perform post-parse operations at next level up in Symbol hierarchy
        super::post_parse(pass);

        switch (pass) {
        case 1:
            // assign direct pointer to agent for use post-parse
            pp_agent = dynamic_cast<AgentSymbol *> (agent);
            // add this table to the complete list of tables
            pp_tables.push_back(this);
            // Add this table to the agent's list of tables
            pp_agent->pp_agent_tables.push_back(this);
            break;
        default:
            break;
        }
    }

    /**
    * Get name of member function which handles increments used in the table
    *
    * Example:  DurationOfLife_do_increments
    *
    * @return  The qualified function name as a string
    */

    const string do_increments_func()
    {
        // Eg. DurationOfLife_do_increments
        return name + "_do_increments";
    }

    /**
    * Get declaration of member function which handles increments used in the table
    *
    * Example:  void Person::DurationOfLife_do_increments(bool prepare = true, bool process = true);
    */

    const string do_increments_decl()
    {
        // E.g. void DurationOfLife_do_increments(bool prepare = true, bool process = true)
        return "void " + do_increments_func() + "( bool prepare = true, bool process = true );";
    }

    /**
    * Get definition of member function which handles increments used in the table
    *
    * Example:  void Person::DurationOfLife_do_increments(bool prepare = true, bool process = true)
    */

    const string do_increments_defn()
    {
        // E.g. void Person::DurationOfLife_do_increments(bool prepare = true, bool process = true)
        return "void " + agent->name + "::" + do_increments_func() + "( bool prepare, bool process )";
    }

    /**
    * Get the c++ declaration code (in agent scope) associated with the symbol.
    */

    virtual CodeBlock cxx_declaration_agent_scope()
    {
        CodeBlock h;
        h += "";
        // example:         // DurationOfLife
        h += "// " + name;
        // example:        void DurationOfLife_do_increments( bool prepare = 0, bool process = 0 );
        h += do_increments_decl();
        return h;
    }

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition()
    {
        CodeBlock c;

        c += "";

        // example:         // DurationOfLife
        c += "// " + name;

        // example:        void DurationOfLife_do_increments( bool prepare = 0, bool process = 0 )
        c += do_increments_defn();
        c += "{";
        c += "if ( process ) {";
        for (auto acc : pp_accumulators) {
            // name of agentvar
            string agentvar_name = acc->agentvar->name;
            // name of 'in' for agentvar
            string in_agentvar_name = acc->pp_analysis_agentvar->in_agentvar_name();
            // index of accumulator as string
            string accumulator_index = to_string(acc->index);
            // expression for the accumulator as string
            string accumulator_expr = "the" + name + ".accumulators[" + accumulator_index + "][0]";

            // expression evaluating to value of increment
            string increment_expr;
            switch (acc->increment) {
            case token::TK_value_in:
                increment_expr = in_agentvar_name;
                break;
            case token::TK_value_out:
                increment_expr = agentvar_name;
                break;
            case token::TK_delta:
                increment_expr = "( " + agentvar_name + " - " + in_agentvar_name + " )";
                break;
                // TODO - all other increment operators
            default:
                assert(0); // grammar guarantee
            }

            c += "{";
            c += "// " + acc->pretty_name();
            c += "double dIncrement = " + increment_expr + ";";
            switch (acc->accumulator) {
            case token::TK_sum:
                c += accumulator_expr + " += dIncrement;";
                break;
            case token::TK_min:
                c += "double dAccumulator = " + accumulator_expr + ";";
                c += "if ( dIncrement < dAccumulator ) " + accumulator_expr + " = dIncrement;";
                break;
            case token::TK_max:
                c += "double dAccumulator = " + accumulator_expr + ";";
                c += "if ( dIncrement > dAccumulator ) " + accumulator_expr + " = dIncrement;";
                break;
            default:
                assert(0); // grammar guarantee
            }
            c += "}";
        }
        c += "}";
        c += "if ( prepare ) {";
        for (auto table_agentvar : pp_table_agentvars) {
            if (table_agentvar->need_value_in)
                c += table_agentvar->cxx_prepare_increment();
        }
        c += "}";
        c += "}";

        return c;
    }

    /**
    * Reference to pointer to agent.
    *
    * Stable to symbol morhing during parse phase.
    */

    Symbol*& agent;

    /**
    * Direct pointer to agent.
    *
    * Set post-parse for convenience.
    */

    AgentSymbol *pp_agent;

    /**
    * The expressions in the table
    */

    list<TableExpressionSymbol *> pp_expressions;

    /**
    * The agentvars used in all expressions in the table
    */

    list<TableAnalysisAgentVarSymbol *> pp_table_agentvars;

    /**
    * The accumulators used in all expressions in the table
    */

    list<TableAccumulatorSymbol *> pp_accumulators;

    /**
    * Numeric identifier.
    * Used for communicating with metadata API.
    */

    int pp_numeric_id;
};
