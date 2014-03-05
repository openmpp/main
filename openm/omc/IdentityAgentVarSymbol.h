/**
* @file    IdentityAgentVarSymbol.h
* Declarations for the IdentityAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <set>
#include "AgentVarSymbol.h"
#include "LinkToAgentVarSymbol.h"
#include "AgentFuncSymbol.h"

using namespace std;

/**
* IdentityAgentVarSymbol
*
* Symbol for expression agentvars of the form agentvar = <expression>;
*/

class IdentityAgentVarSymbol : public AgentVarSymbol
{
private:
    typedef AgentVarSymbol super;

public:
    bool is_base_symbol() const { return false; }

    IdentityAgentVarSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, ExprForAgentVar *root, yy::location decl_loc = yy::location())
        : AgentVarSymbol(sym, agent, type, decl_loc)
        , root(root)
    {
        create_auxiliary_symbols();
    }

    IdentityAgentVarSymbol(const string member_name, const Symbol *agent, const Symbol *type, ExprForAgentVar *root, yy::location decl_loc = yy::location())
        : AgentVarSymbol(member_name, agent, type, decl_loc)
        , root(root)
    {
        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

    void post_parse_traverse(ExprForAgentVar *node);

    /**
     * Builds the function body of the expression function.
     */
    void build_body_expression();

    /**
     * Constructs the C++ expression from the parse tree.
     *
     * @param node The node.
     *
     * @return The expression.
     */
    string cxx_expression(const ExprForAgentVar *node);

    /**
     * Root of the expression tree.
     */
    ExprForAgentVar *root;

    /**
     * The agentvars used in the expression.
     */
    set<AgentVarSymbol *> pp_agentvars_used;

    /**
     * The links used in the expression.
     */
    set<LinkAgentVarSymbol *> pp_links_used;

    /**
     * The linked agentvars used in the expression.
     */
    set<LinkToAgentVarSymbol *> pp_linked_agentvars_used;

    /**
     * The expression function.
     */
    AgentFuncSymbol *expression_fn;
};

