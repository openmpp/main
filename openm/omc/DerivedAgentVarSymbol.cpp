/**
* @file    DerivedAgentVarSymbol.cpp
* Definitions for the DerivedAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DerivedAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentVarSymbol.h"
#include "ConstantSymbol.h"
#include "PartitionSymbol.h"
#include "IdentityAgentVarSymbol.h"
#include "BuiltinAgentVarSymbol.h"
#include "TimeSymbol.h"
#include "ExprForAgentVar.h"
#include "Literal.h"
#include "CodeBlock.h"

using namespace std;

// static
string DerivedAgentVarSymbol::member_name(token_type tk1,
                                          token_type tk2,
                                          const Symbol *av1,
                                          const Symbol *av2,
                                          const Symbol *prt,
                                          const ConstantSymbol *k1,
                                          const ConstantSymbol *k2,
                                          const ConstantSymbol *k3)
{
    string result = "om";
    if (tk1 != token::TK_unused) result += "_" + token_to_string(tk1);
    if (tk2 != token::TK_unused) result += "_" + token_to_string(tk2);
    if (av1 != nullptr) result += "_" + av1->name;
    if (k1 != nullptr)  result += "_" + k1->value_as_name();
    if (av2 != nullptr) result += "_" + av2->name;
    if (prt != nullptr) result += "_" + prt->name;
    if (k2 != nullptr)  result += "_" + k2->value_as_name();
    if (k3 != nullptr)  result += "_" + k3->value_as_name();

    return result;
}

// static
Symbol * DerivedAgentVarSymbol::create_symbol(const Symbol* agent,
                                              token_type tk1,
                                              token_type tk2,
                                              const Symbol *av1,
                                              const Symbol *av2,
                                              const Symbol *prt,
                                              const ConstantSymbol *k1,
                                              const ConstantSymbol *k2,
                                              const ConstantSymbol *k3,
                                              yy::location decl_loc)
{
    Symbol *sym = nullptr;
    string mem_name = member_name(tk1, tk2, av1, av2, prt, k1, k2, k3);
    string nm = Symbol::symbol_name(mem_name, agent);
    auto it = symbols.find(nm);
    if (it != symbols.end()) {
        sym = it->second;
    }
    else {
        sym = new DerivedAgentVarSymbol(agent, tk1, tk2, av1, av2, prt, k1, k2, k3, decl_loc);
    }
    return sym;
}

void DerivedAgentVarSymbol::create_auxiliary_symbols()
{
    switch (tk1) {
    case token::TK_duration:
    {
        if (av1 && k1) {
            // Create identity agentvar to maintain av1 == k1
            iav = IdentityAgentVarSymbol::CreateEqualityIdentitySymbol(agent, *av1, k1, decl_loc);
            assert(iav);
        }
        break;
    }
    default:
        break;
    }
}

void DerivedAgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        break;
    }
    case ePopulateCollections:
    {
        // assign direct pointers for post-parse use
        if (av1) pp_av1 = dynamic_cast<AgentVarSymbol *> (pp_symbol(av1));
        if (av1) assert(pp_av1);
        if (av2) pp_av2 = dynamic_cast<AgentVarSymbol *> (pp_symbol(av2));
        if (av2) assert(pp_av2);
        if (prt) pp_prt = dynamic_cast<PartitionSymbol *> (pp_symbol(prt));
        if (prt) assert(pp_prt);
        break;
    }
    case eResolveDataTypes:
    {
        switch (tk1) {
        case token::TK_duration:
        {
            change_data_type(TimeSymbol::find());
            break;
        }
        default:
        break;
        }
    break;
    }
    case ePopulateDependencies:
    {
        switch (tk1) {
        case token::TK_duration:
        {
            if (!av1) {
                // simple duration()
                // add side-effect to time agentvar
                auto *av = pp_agent->pp_time;
                CodeBlock& c = av->side_effects_fn->func_body;
                c += "// Advance time for " + pretty_name();
                c += name + ".set(" + name + ".get() + om_delta);";
                c += "";
            }
            else {
                // conditioned duration(av, value)
                // add side-effect to time agentvar
                auto *av = pp_agent->pp_time;
                CodeBlock& c = av->side_effects_fn->func_body;
                c += "// Advance time for " + pretty_name();
                c += "if (" + iav->name + ") {";
                c += name + ".set(" + name + ".get() + om_delta);";
                c += "}";
                c += "";
            }
            break;
        }
        default:
        break;
        }
        break;
    }
    default:
        break;
    }
}

CodeBlock DerivedAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += "AgentVar<"
        + pp_data_type->name + ", "
        + pp_data_type->wrapped_type() + ", "
        + pp_agent->name + ", "
        + "&" + side_effects_fn->unique_name + ">";
    h += name + ";";

    return h;
}


