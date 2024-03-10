/**
* @file    IdentityAttributeSymbol.h
* Declarations for the IdentityAttributeSymbol class.
*/
// Copyright (c) 2013-2016 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <set>
#include "MaintainedAttributeSymbol.h"
#include "LinkToAttributeSymbol.h"
#include "EntityFuncSymbol.h"

class EntityTableSymbol;

using namespace std;

/**
* IdentityAttributeSymbol
*
* Symbol for identity attributes of the form attribute = <expression>;
*/

class IdentityAttributeSymbol : public MaintainedAttributeSymbol
{
private:
    typedef MaintainedAttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Morphing constructor
     *
     * @param [in,out] sym  If non-null, the symbol.
     * @param ent           The entity.
     * @param type          The type.
     * @param [in,out] root If non-null, the root.
     * @param decl_loc      (Optional) the declaration location.
     */
    IdentityAttributeSymbol(Symbol *sym, const Symbol * ent, const Symbol *type, ExprForAttribute *root, omc::location decl_loc = omc::location())
        : MaintainedAttributeSymbol(sym, ent, type, decl_loc)
        , root(root)
        , associated_table(nullptr)
    {
        create_auxiliary_symbols();

        // This sorting group ensures that side-effect code for the identity attribute
        // which might be injected into 'time' will execute after side-effect code injected by active_spell_delta (sorting group 2)
        // (side-effect code is injected by sorting group then lexicographic order of the name of the injecting attribute.)
        // This ensures that logic to maintain active_spell_delta works correctly: the side-effect for active_spell_delta
        // must execute before side-effect code which changes the condition.
        sorting_group = 8;
    }

    /**
     * Constructor from name
     *
     * @param member_name   Name of the member.
     * @param ent           The entity.
     * @param type          The type.
     * @param [in,out] root If non-null, the root.
     * @param decl_loc      (Optional) the declaration location.
     */
    IdentityAttributeSymbol(const string member_name, const Symbol * ent, const Symbol *type, ExprForAttribute *root, omc::location decl_loc = omc::location())
        : MaintainedAttributeSymbol(member_name, ent, type, decl_loc)
        , root(root)
        , associated_table(nullptr)
    {
        create_auxiliary_symbols();

        // This sorting group ensures that side-effect code for the identity attribute
        // which might be injected into 'time' will execute after side-effect code injected by active_spell_delta (sorting group 2)
        // (side-effect code is injected by sorting group then lexicographic order of the name of the injecting attribute.)
        // This ensures that logic to maintain active_spell_delta works correctly: the side-effect for active_spell_delta
        // must execute before side-effect code which changes the condition.
        sorting_group = 8;
    }

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_entity();

    void post_parse_traverse1(ExprForAttribute *node);

    void post_parse_traverse2(ExprForAttribute *node);

    /**
     * Builds the function body of the expression function.
     */
    void build_body_expression();

    /**
     * Constructs the C++ expression from a parse tree.
     *
     * @param node The node.
     *
     * @return The expression.
     */
    static string cxx_expression(const ExprForAttribute *node, bool use_pretty_name = false);

    /**
     * Creates an anonymous identity attribute for an expression tree or returns it if already created.
     *
     * @param [in,out] av If non-null, the attribute on the lhs
     * @param k           The ConstantSymbol on the rhs.
     *
     * @return The new identity attribute.
     */
    static IdentityAttributeSymbol * anonymous_identity_attribute(Symbol *entity, const Symbol *type, ExprForAttribute *node, omc::location decl_loc);

    /**
     * Creates an identity attribute for an equality or returns it if already created.
     *
     * @param [in,out] av If non-null, the attribute on the lhs
     * @param k           The ConstantSymbol on the rhs.
     *
     * @return The new identity attribute.
     */
    static IdentityAttributeSymbol * create_equality_identity_attribute(Symbol * ent, Symbol * av, const ConstantSymbol *k, omc::location decl_loc);

    /**
     * Root of the expression tree.
     */
    ExprForAttribute *root;

    /**
     * The attributes used in the expression.
     */
    set<AttributeSymbol *> pp_attributes_used;

    /**
     * The links used in the expression.
     */
    set<LinkAttributeSymbol *> pp_links_used;

    /**
     * The link-to attributes used in the expression.
     */
    set<LinkToAttributeSymbol *> pp_linkto_attributes_used;

    /**
     * The expression function.
     */
    EntityFuncSymbol *expression_fn;

    /**
     * For table filter identity attributes, points to the entity table
     */
    EntityTableSymbol* associated_table;

    /**
     * Map from key to name for anonymous identity attributes.
     *
     * The key is the canonical C++ expression for the expression tree,
     * prefixed by the entity name and data type.
     */
    static unordered_map<string, string> anonymous_key_to_name;
};

