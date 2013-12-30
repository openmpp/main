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

    void post_parse(int pass);

    /**
    * Get name of member function which handles increments used in the table
    *
    * Example:  DurationOfLife_do_increments
    *
    * @return  The qualified function name as a string
    */

    const string do_increments_func();

    /**
    * Get declaration of member function which handles increments used in the table
    *
    * Example:  void Person::DurationOfLife_do_increments(bool prepare = true, bool process = true);
    */

    const string do_increments_decl();

    /**
    * Get definition of member function which handles increments used in the table
    *
    * Example:  void Person::DurationOfLife_do_increments(bool prepare = true, bool process = true)
    */

    const string do_increments_defn();

    /**
    * Get the c++ declaration code (in agent scope) associated with the symbol.
    */

    virtual CodeBlock cxx_declaration_agent_scope();

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition();

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
