/**
* @file    IdentityAttributeSymbol.cpp
* Definitions for the IdentityAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "IdentityAttributeSymbol.h"
#include "EntitySymbol.h"
#include "ExprForAttribute.h"
#include "LinkAttributeSymbol.h"
#include "EntityMultilinkSymbol.h"
#include "LinkToAttributeSymbol.h"
#include "TypeSymbol.h"
#include "BoolSymbol.h"
#include "GlobalFuncSymbol.h"
#include "ConstantSymbol.h"
#include "CodeBlock.h"
#include "Literal.h"

using namespace std;

void IdentityAttributeSymbol::create_auxiliary_symbols()
{
    // Create an EntityFuncSymbol for the expression function
    expression_fn = new EntityFuncSymbol(name + "_update_identity", agent);
    assert(expression_fn); // out of memory check
    expression_fn->doc_block = doxygen_short("Evaluate and assign expression for " + name + ".");
    expression_fn->has_line_directive = true; // the body will contain a #line directive
}

/**
* Post-parse operations for EntityTableMeasureSymbol
*/

void IdentityAttributeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        // do a post-parse traverse to identify apparent use of global functions, and create a GlobalFuncSymbol for each
        post_parse_traverse1(root);
        break;
    }
    case ePopulateCollections:
    {
        // Add this identity agentvar symbol to the agent's list of all such symbols
        pp_agent->pp_identity_agentvars.push_back(this);
        
        // Perform post-parse operations to each element in the expression tree
        post_parse_traverse2(root);
        break;
    }
    case ePopulateDependencies:
    {
        // construct function body
        build_body_expression();

        // Dependency on agentvars in expression
        for (auto av : pp_agentvars_used) {
            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            c += "// Maintain identity for '" + name + "'";
            c += expression_fn->name + "();";
        }

        // Dependency on linked agentvars in expression
        for (auto ltav : pp_linked_agentvars_used) {
            auto av = ltav->pp_agentvar; // agentvar being referenced across the link (rhs)
            assert(av);

            auto lav = ltav->pp_link; // the link agentvar
            assert(lav);

            CodeBlock& c = av->side_effects_fn->func_body;
            c += injection_description();
            if (lav->reciprocal_link) {
                // is a one-to-one link
                c += "// Maintain identity for '" + unique_name + "' using reciprocal link";
                auto rlav = lav->reciprocal_link;
                c += "if (!" + rlav->name + ".is_nullptr()) " + rlav->name + "->" + expression_fn->name + "();";
            }
            else {
                // is a one-to-many link
                c += "// Maintain identity for '" + unique_name + "' in all entities in reciprocal multilink";
                assert(lav->reciprocal_multilink); // grammar guarantee (must be either link or multilink)
                auto rlav = lav->reciprocal_multilink;
                c += "for (auto &item : " + rlav->name + ".storage) {";
                c += "if (item.get() != nullptr) {";
                c += "item->" + expression_fn->name + "();";
                c += "}";
                c += "}";
            }
        }

        // Dependency of all links used in expression
        for (auto lav : pp_links_used) {
            CodeBlock& c = lav->side_effects_fn->func_body;
            c += injection_description();
            c += "// Maintain identity for '" + unique_name + "' when link changed to different agent";
            c += expression_fn->name + "();";
        }

        // Initialization before entity enters simulation
        {
            auto fn = pp_agent->initialize_identity_attributes_fn;
            assert(fn);
            CodeBlock& c = fn->func_body;
            c += injection_description();
            c += "// Initialize '" + name + "' to correct value based on expression.";
            c += expression_fn->name + "();";
        }
        break;
    }
    default:
        break;
    }
}

void IdentityAttributeSymbol::post_parse_traverse1(ExprForAttribute *node)
{
    auto sym = dynamic_cast<ExprForAgentVarSymbol *>(node);
    auto lit = dynamic_cast<ExprForAgentVarLiteral *>(node);
    auto unary_op = dynamic_cast<ExprForAgentVarUnaryOp *>(node);
    auto binary_op = dynamic_cast<ExprForAgentVarBinaryOp *>(node);
    auto ternary_op = dynamic_cast<ExprForAgentVarTernaryOp *>(node);

    if (sym != nullptr) {
        // do nothing with symbol terminals
    }
    else if (lit != nullptr) {
        // do nothing with Literal terminals
    }
    else if (unary_op != nullptr) {
        assert(unary_op->right); // grammar guarantee
        post_parse_traverse1(unary_op->right);
    }
    else if (binary_op != nullptr) {
        assert(binary_op->left); // grammar guarantee
        post_parse_traverse1(binary_op->left);
        assert(binary_op->right); // grammar guarantee
        post_parse_traverse1(binary_op->right);
        // look for signature of a function call
        if (binary_op->op == token::TK_LEFT_PAREN) {
            auto node = binary_op->left;
            if (auto efavs = dynamic_cast<ExprForAgentVarSymbol *>(node)) {
                auto sym = efavs->symbol;
                if (sym->is_base_symbol() && !Symbol::exists(sym->name)) {
                    // an undeclared SYMBOL followed by a '('
                    // create a GlobalFuncSymbol with that name
                    auto gfs = new GlobalFuncSymbol(sym->name, sym->decl_loc);
                    // push the name into the pass #1 ignore hash
                    pp_ignore_pass1.insert(gfs->unique_name);
                }
            }
        }
    }
    else if (ternary_op != nullptr) {
        assert(ternary_op->cond); // grammar guarantee
        post_parse_traverse1(ternary_op->cond);
        assert(ternary_op->first); // grammar guarantee
        post_parse_traverse1(ternary_op->first);
        assert(ternary_op->second); // grammar guarantee
        post_parse_traverse1(ternary_op->second);
    }
    else {
        assert(false); // not reached
    }
}

void IdentityAttributeSymbol::post_parse_traverse2(ExprForAttribute *node)
{
    auto sym = dynamic_cast<ExprForAgentVarSymbol *>(node);
    auto lit = dynamic_cast<ExprForAgentVarLiteral *>(node);
    auto unary_op = dynamic_cast<ExprForAgentVarUnaryOp *>(node);
    auto binary_op = dynamic_cast<ExprForAgentVarBinaryOp *>(node);
    auto ternary_op = dynamic_cast<ExprForAgentVarTernaryOp *>(node);

    if (sym != nullptr) {
        (sym->pp_symbol) = pp_symbol(sym->symbol);
        assert(sym->pp_symbol); // parser guarantee
        auto av = dynamic_cast<AttributeSymbol *>(sym->pp_symbol);
        if (av) {
            // add to the set of all agentvars used in this expression
            pp_agentvars_used.insert(av);
        }
        auto ltav = dynamic_cast<LinkToAttributeSymbol *>(sym->pp_symbol);
        if (ltav) {
            // add to the set of all links to agentvars used in this expression
            pp_linked_agentvars_used.insert(ltav);

            // add the link itself to the set of all links used in this expression
            // ltav->pp_link is not yet valid in this post-parse pass,
            // so go through ltav->link instead
            assert(ltav->link);
            auto temp_sym = pp_symbol(ltav->link);
            assert(temp_sym);
            auto lav = dynamic_cast<LinkAttributeSymbol *>(temp_sym);
            assert(lav);
            pp_links_used.insert(lav);
        }
    }
    else if (lit != nullptr) {
        // Nothing needs to be done with Literals in the post-parse processing phase.
    }
    else if (unary_op != nullptr) {
        assert(unary_op->right); // grammar guarantee
        post_parse_traverse2(unary_op->right);
    }
    else if (binary_op != nullptr) {
        assert(binary_op->left); // grammar guarantee
        post_parse_traverse2(binary_op->left);
        assert(binary_op->right); // grammar guarantee
        post_parse_traverse2(binary_op->right);
    }
    else if (ternary_op != nullptr) {
        assert(ternary_op->cond); // grammar guarantee
        post_parse_traverse2(ternary_op->cond);
        assert(ternary_op->first); // grammar guarantee
        post_parse_traverse2(ternary_op->first);
        assert(ternary_op->second); // grammar guarantee
        post_parse_traverse2(ternary_op->second);
    }
    else {
        assert(false); // not reached
    }
}

string IdentityAttributeSymbol::cxx_expression(const ExprForAttribute *node)
{
    string result;

    auto sym = dynamic_cast<const ExprForAgentVarSymbol *>(node);
    auto lit = dynamic_cast<const ExprForAgentVarLiteral *>(node);
    auto unary_op = dynamic_cast<const ExprForAgentVarUnaryOp *>(node);
    auto binary_op = dynamic_cast<const ExprForAgentVarBinaryOp *>(node);
    auto ternary_op = dynamic_cast<const ExprForAgentVarTernaryOp *>(node);

    if (sym != nullptr) {
        result = sym->pp_symbol->name;
    }
    else if (lit != nullptr) {
        result = lit->constant->value();
    }
    else if (unary_op != nullptr) {
        assert(unary_op->right); // grammar guarantee
        result =
            token_to_string(unary_op->op)
            + cxx_expression(unary_op->right);
    }
    else if (binary_op != nullptr) {
        assert(binary_op->left); // grammar guarantee
        assert(binary_op->right); // grammar guarantee
        if (binary_op->op == token::TK_LEFT_PAREN) {
            // function-style cast or call
            result = cxx_expression(binary_op->left)
                + "("
                + cxx_expression(binary_op->right)
                + ")";
        }
        else if (binary_op->op == token::TK_LEFT_BRACKET) {
            // array indexing
            result = cxx_expression(binary_op->left)
                + "["
                + cxx_expression(binary_op->right)
                + "]";
        }
        else if (binary_op->op == token::TK_COMMA) {
            // infix "," binary operator - do not enclose in parentheses 
            result = 
                  cxx_expression(binary_op->left)
                + ", "
                + cxx_expression(binary_op->right)
                ;
        }
        else {
            // infix binary operator
            result = "("
                + cxx_expression(binary_op->left)
                + " " + token_to_string(binary_op->op) + " "
                + cxx_expression(binary_op->right)
                + ")";
        }
    }
    else if (ternary_op != nullptr) {
        assert(ternary_op->cond); // grammar guarantee
        assert(ternary_op->first); // grammar guarantee
        assert(ternary_op->second); // grammar guarantee
        result = "("
            + cxx_expression(ternary_op->cond) + " ? "
            + cxx_expression(ternary_op->first) + " : "
            + cxx_expression(ternary_op->second)
            + ")";
    }
    else {
        assert(false); // not reached
    }

    return result;
}

void IdentityAttributeSymbol::build_body_expression()
{
    CodeBlock& c = expression_fn->func_body;

    if (decl_loc.begin.filename) {
        c += (no_line_directives ? "//#line " : "#line ")
            + to_string(decl_loc.begin.line)
            + " \""
            + *(decl_loc.begin.filename)
            + "\"";
    }
    else {
        c += "//#line This is a generated identity attribute which has no associated model source file";
    }
    c += name + ".set(" + cxx_expression(root) + ");";
}


CodeBlock IdentityAttributeSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += "AgentVar<"
        + pp_data_type->name + ", "
        + pp_data_type->exposed_type() + ", "
        + agent->name + ", "
        + "&om_name_" + name + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + (!side_effects_fn->empty() ? "true" : "false") + ", "
        + "&" + notify_fn->unique_name + ", "
        + (!notify_fn->empty() ? "true" : "false")
        + ">";
    h += name + ";";

    return h;
}

//static
IdentityAttributeSymbol * IdentityAttributeSymbol::CreateEqualityIdentitySymbol(Symbol *agent, Symbol * av, const ConstantSymbol *k, yy::location decl_loc)
{
    assert(agent);
    assert(av);
    assert(k);
    IdentityAttributeSymbol *iav = nullptr;
    string mem_name = "om_equality_" + av->name + "_EQ_" + k->value_as_name();
    string nm = Symbol::symbol_name(mem_name, agent);
    auto it = symbols.find(nm);
    if (it != symbols.end()) {
        // already exists
        auto sym = it->second;
        iav = dynamic_cast<IdentityAttributeSymbol * >(sym);
        assert(iav);
        return iav;
    }
    else {
        // Create expression tree terminal node for lhs
        auto lhs = new ExprForAgentVarSymbol(av);
        // Create expression tree terminal node for rhs
        ExprForAttribute * rhs = nullptr;
        if (k->is_literal) {
            rhs = new ExprForAgentVarLiteral( k->literal );
        }
        else {
            rhs = new ExprForAgentVarSymbol(*(k->enumerator));
        }
        // Create expression tree node for == binary operation
	    auto expr = new ExprForAgentVarBinaryOp(token::TK_EQ, lhs, rhs);
        // Create the identity agentvar to maintain the expression
        iav = new IdentityAttributeSymbol( mem_name, agent, BoolSymbol::find(), expr, decl_loc);
        assert(iav);

        return iav;
    }
}