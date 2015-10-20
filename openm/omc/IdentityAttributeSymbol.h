/**
* @file    IdentityAttributeSymbol.h
* Declarations for the IdentityAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <set>
#include "AttributeSymbol.h"
#include "LinkToAttributeSymbol.h"
#include "EntityFuncSymbol.h"

using namespace std;

/**
* IdentityAttributeSymbol
*
* Symbol for expression agentvars of the form agentvar = <expression>;
*/

class IdentityAttributeSymbol : public AttributeSymbol
{
private:
    typedef AttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Morphing constructor
     *
     * @param [in,out] sym  If non-null, the symbol.
     * @param agent         The agent.
     * @param type          The type.
     * @param [in,out] root If non-null, the root.
     * @param decl_loc      (Optional) the declaration location.
     */
    IdentityAttributeSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, ExprForAttribute *root, yy::location decl_loc = yy::location())
        : AttributeSymbol(sym, agent, type, decl_loc)
        , root(root)
    {
        create_auxiliary_symbols();

        // This sorting group ensures that side-effect code for the identity agentvar
        // which might be injected into 'time' will execute after side-effect code injected by active_spell_delta (sorting group 2)
        // (side-effect code is injected by sorting group then lexicographic order of the name of the injecting agentvar.)
        // This ensures that logic to maintain active_spell_delta works correctly: the side-effect for active_spell_delta
        // must execute before side-effect code which changes the condition.
        sorting_group = 8;
    }

    /**
     * Constructor from name
     *
     * @param member_name   Name of the member.
     * @param agent         The agent.
     * @param type          The type.
     * @param [in,out] root If non-null, the root.
     * @param decl_loc      (Optional) the declaration location.
     */
    IdentityAttributeSymbol(const string member_name, const Symbol *agent, const Symbol *type, ExprForAttribute *root, yy::location decl_loc = yy::location())
        : AttributeSymbol(member_name, agent, type, decl_loc)
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

    void post_parse_traverse1(ExprForAttribute *node);

    void post_parse_traverse2(ExprForAttribute *node);

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
    string cxx_expression(const ExprForAttribute *node);

    /**
     * Creates an identity symbol for an equality or returns it if already present.
     *
     * @param [in,out] av If non-null, the agentvar on the lhs
     * @param k           The ConstantSymbol on the rhs.
     *
     * @return The new equality symbol.
     */
    static IdentityAttributeSymbol * CreateEqualityIdentitySymbol(Symbol *agent, Symbol * av, const ConstantSymbol *k, yy::location decl_loc);

    /**
     * Root of the expression tree.
     */
    ExprForAttribute *root;

    /**
     * The agentvars used in the expression.
     */
    set<AttributeSymbol *> pp_agentvars_used;

    /**
     * The links used in the expression.
     */
    set<LinkAttributeSymbol *> pp_links_used;

    /**
     * The linked agentvars used in the expression.
     */
    set<LinkToAttributeSymbol *> pp_linked_agentvars_used;

    /**
     * The expression function.
     */
    EntityFuncSymbol *expression_fn;
};

