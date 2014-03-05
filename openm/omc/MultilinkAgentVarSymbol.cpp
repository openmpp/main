/**
* @file    MultilinkAgentVarSymbol.cpp
* Definitions for the MultilinkAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "MultilinkAgentVarSymbol.h"
#include"AgentFuncSymbol.h"
#include "AgentMultilinkSymbol.h"
#include "CodeBlock.h"

using namespace std;

void MultilinkAgentVarSymbol::create_auxiliary_symbols()
{
    // Create an AgentFuncSymbol for the evaluation function
    evaluate_fn = new AgentFuncSymbol(name + "_evaluate", agent);
    evaluate_fn->doc_block = doxygen_short("Evaluate the multilink agentvar " + name + ".");
    CodeBlock &c = evaluate_fn->func_body;
    assert(multilink); // logic guarantee
    c += name + ".set(" + multilink->name + ".size());";
}

/**
 * Post-parse operations for the symbol.
 *
 * @param pass The pass.
 */
void MultilinkAgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // assign direct pointers for post-parse use
            pp_multilink = dynamic_cast<AgentMultilinkSymbol *> (pp_symbol(multilink));
            assert(pp_multilink); // syntax error
            // assign direct pointers for post-parse use
            if (agentvar) {
                pp_agentvar = dynamic_cast<AgentVarSymbol *> (pp_symbol(agentvar));
                assert(pp_agentvar); // syntax error
            }
        }
        break;
    case ePopulateDependencies:
        {
            // Dependency on multilink
            {
                CodeBlock& c = pp_multilink->side_effects_fn->func_body;
                c += "// Re-evaluate multilink agentvar " + name;
                c += evaluate_fn->name + "();";
                c += "";
            }
        }
        break;
    default:
        break;
    }
}

CodeBlock MultilinkAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "AgentVar<" + pp_data_type->name + ", "
        + agent->name + ", "
        + "&" + side_effects_fn->unique_name + ">";
    h += name + ";";
    h += "";

    return h;
}


// static
string MultilinkAgentVarSymbol::member_name(token_type func, const Symbol *multilink, const Symbol *agentvar)
{
    string result = "om_" + multilink->name + "_" + token_to_string(func);
    if (agentvar) {
        result += "_" + agentvar->name;
    }
    return result;
}

// static
string MultilinkAgentVarSymbol::symbol_name(const Symbol *agent, token_type func, const Symbol *multilink, const Symbol *agentvar)
{
    string member = MultilinkAgentVarSymbol::member_name(func, multilink, agentvar);
    string result = Symbol::symbol_name(member, agent);
    return result;
}

//static
Symbol * MultilinkAgentVarSymbol::create_symbol(const Symbol *agent, token_type func, const Symbol *multilink, const Symbol *agentvar)
{
    Symbol *sym = nullptr;
    string unm = MultilinkAgentVarSymbol::symbol_name(agent, func, multilink, agentvar);
    auto it = symbols.find(unm);
    if (it != symbols.end())
        sym = it->second;
    else {
        string nm = MultilinkAgentVarSymbol::member_name(func, multilink, agentvar);
        sym = new MultilinkAgentVarSymbol(agent, func, multilink, agentvar);
    }

    return sym;

}
