/**
* @file    MultilinkAttributeSymbol.cpp
* Definitions for the MultilinkAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "MultilinkAttributeSymbol.h"
#include "EntityFuncSymbol.h"
#include "EntityMultilinkSymbol.h"
#include "LinkAttributeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void MultilinkAttributeSymbol::create_auxiliary_symbols()
{
    // Create an EntityFuncSymbol for the evaluation function
    evaluate_fn = new EntityFuncSymbol(name + "_evaluate", agent);
    evaluate_fn->doc_block = doxygen_short("Evaluate the multilink attribute " + name + ".");
}

void MultilinkAttributeSymbol::build_body_evaluate()
{
    CodeBlock& c = evaluate_fn->func_body;

    assert(multilink); // logic guarantee
    if (func == token::TK_count) {
        // ex. om_mlTestActors_count.set((counter)mlTestActors.size());
        c += name + ".set((" + pp_data_type->name + ")" + multilink->name + ".size());";
    }
    else if (func == token::TK_sum_over) {
        assert(attribute.size());  // grammar guarantee
        c += pp_data_type->name + " val_result = 0;";
        c += "for (auto &item : " + multilink->name + ".storage) {";
        c += "if (item.get() != nullptr) {";
        c += pp_data_type->name + " val_item = item->" + attribute + "; ";
        c += "val_result += val_item;";
        c += "}";
        c += "}";
        c += name + ".set(val_result);";
    }
    else if (func == token::TK_min_over || func == token::TK_max_over ) {
        assert(attribute.size());  // grammar guarantee
        string op = (func == token::TK_min_over) ? " < " : " > ";
        string initializer = " = " + pp_attribute->initialization_value(true);
        c += pp_data_type->name + " val_result" + initializer + ";  // default value for the type";
        c += "bool found = false;";
        c += "for (auto &item : " + multilink->name + ".storage) {";
        c += "if (item.get() != nullptr) {";
        c += pp_data_type->name + " val_item = item->" + attribute + "; ";
        c += "if (!found) {";
        c += "val_result = val_item;";
        c += "found = true;";
        c += "}";
        c += "else {";
        c += "if (val_item" + op + "val_result) val_result = val_item;";
        c += "}";
        c += "}";
        c += "}";
        c += name + ".set(val_result);";
    }
    else {
        assert(false); // logic guarantee
    }
    
}


/**
 * Post-parse operations for the symbol.
 *
 * @param pass The pass.
 */
void MultilinkAttributeSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointers for post-parse use
        pp_multilink = dynamic_cast<EntityMultilinkSymbol *> (pp_symbol(multilink));
        assert(pp_multilink); // syntax error

        // assign direct pointer to attribute used in multilink
        if (attribute.size()) {
            assert(func == token::TK_sum_over || func == token::TK_min_over || func == token::TK_max_over);
            Symbol *as = nullptr;
            if (pp_multilink->reciprocal_link) {
                as = pp_multilink->reciprocal_link->agent;
            }
            else {
                assert(pp_multilink->reciprocal_multilink); // logic guarantee
                as = pp_multilink->reciprocal_multilink->agent;
            }
            assert(as); // logic guarantee
            auto sym = Symbol::get_symbol(attribute, as);
            assert(sym); // parser / scanner guarantee
            auto av = dynamic_cast<AttributeSymbol *> (sym);
            assert(av); // TODO possible model source code error
            pp_attribute = av;
        }
    }
    break;

    case eResolveDataTypes:
    {
        if (func == token::TK_count) {
            // for count(), data type is always counter
            change_data_type(NumericSymbol::find(token::TK_counter));
        }
        else { // sum_over, min_over, max_over
            assert(pp_attribute); // logic guarantee
            assert(pp_attribute->pp_data_type);  // TODO recursive logic for derived av's
            auto dts = pp_attribute->pp_data_type;

            // set data_type of this multilink attribute
            if (func == token::TK_sum_over) {
                // for sum_over, data type is either real or integer
                if (dts == NumericSymbol::find(token::TK_float)
                    || dts == NumericSymbol::find(token::TK_double)
                    || dts == NumericSymbol::find(token::TK_real)) {
                    change_data_type(NumericSymbol::find(token::TK_real));
                }
                else {
                    change_data_type(NumericSymbol::find(token::TK_integer));
                }
            }
            else {
                // for min_over and max_over, data type is type of attribute
                assert(func == token::TK_min_over || func == token::TK_max_over);
                change_data_type(dts);
            }
        }
    }
    break;

    case ePopulateCollections:
    {
        if (pp_attribute) {
            // is sum_over, min_over, max_over

            // The multilink function attribute depends on the attribute
            pp_dependent_attributes.emplace(pp_attribute);

            // The time-like status of the attribute propagate to the mutlilink function attribute
            pp_dependent_attributes_timelike_propagating.emplace(pp_attribute);
        }
        break;
    }
    case ePopulateDependencies:
    {
        // Construct function body
        build_body_evaluate();

        // Dependency on multilink
        {
            CodeBlock& c = pp_multilink->side_effects_fn->func_body;
            c += injection_description();
            c += "// Re-evaluate multilink attribute " + name;
            c += evaluate_fn->name + "();";
            c += "";
        }

        // Dependency on attribute
        if (pp_attribute) {
            if (pp_multilink->reciprocal_link) {
                string rlink = pp_multilink->reciprocal_link->name;
                CodeBlock& c = pp_attribute->side_effects_fn->func_body;
                c += injection_description();
                c += "// Incremental update of multilink attribute " + name;
                c += "{";
                c += "auto lnk = " + rlink + ".get();";
                c += "if (lnk.get() != nullptr) {";
                string typ = pp_data_type->name; // C++ type of the multilink attribute, e.g. 'int', 'double'
                if (func == token::TK_sum_over) {
                    c += "lnk->" + name + ".set(lnk->" + name + " + om_new - om_old);";
                }
                else if (func == token::TK_min_over) {
                    c += typ + " current = lnk->" + name + ";";
                    c += "if (om_new < current) lnk->" + name +".set(om_new);";
                    c += "else if (om_old == current && om_new > current) lnk->" + evaluate_fn->name + "();";
                }
                else if (func == token::TK_max_over) {
                    c += typ + " current = lnk->" + name + ";";
                    c += "if (om_new > current) lnk->" + name +".set(om_new);";
                    c += "else if (om_old == current && om_new < current) lnk->" + evaluate_fn->name + "();";
                }
                else {
                    assert(false); // logic guarantee
                }
                c += "}";
                c += "}";
                //c += "if (!" + rlink + ".is_nullptr()) " + rlink + "->" + evaluate_fn->name + "();";
                c += "";
            }
            else {
                assert(pp_multilink->reciprocal_multilink);
                string rlink = pp_multilink->reciprocal_multilink->name;
                CodeBlock& c = pp_attribute->side_effects_fn->func_body;
                c += injection_description();
                c += "// Incremental update of multilink attribute " + name + " for each agent of multilink";
                c += "for (auto &lnk : " + rlink + ".storage) {";
                c += "if (lnk.get() != nullptr) {";
                string typ = pp_data_type->name; // C++ type of the multilink attribute, e.g. 'int', 'double'
                if (func == token::TK_sum_over) {
                    c += "lnk->" + name + ".set(lnk->" + name + " + om_new - om_old);";
                }
                else if (func == token::TK_min_over) {
                    c += typ + " current = lnk->" + name + ";";
                    c += "if (om_new < current) lnk->" + name +".set(om_new);";
                    c += "else if (om_old == current && om_new > current) lnk->" + evaluate_fn->name + "();";
                }
                else if (func == token::TK_max_over) {
                    c += typ + " current = lnk->" + name + ";";
                    c += "if (om_new > current) lnk->" + name +".set(om_new);";
                    c += "else if (om_old == current && om_new < current) lnk->" + evaluate_fn->name + "();";
                }
                else {
                    assert(false); // logic guarantee
                }
                c += "}";
                c += "}";
                c += "";
            }
        }
    }
    break;

    default:
    break;
    }
}

CodeBlock MultilinkAttributeSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "Attribute<" 
        + pp_data_type->name + ", "
        + pp_data_type->exposed_type() + ", "
        + agent->name + ", "
        + "&om_name_" + name + ", "
        + (is_time_like ? "true" : "false") + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + (!side_effects_fn->empty() ? "true" : "false") + ", "
        + "&" + notify_fn->unique_name + ", "
        + (!notify_fn->empty() ? "true" : "false")
        + ">";
    h += name + ";";
    h += "";

    return h;
}


// static
string MultilinkAttributeSymbol::member_name(token_type func, const Symbol *multilink, const string attribute)
{
    string result = "om_" + multilink->name + "_" + token_to_string(func);
    if (attribute.size()) {
        result += "_" + attribute;
    }
    return result;
}

// static
string MultilinkAttributeSymbol::symbol_name(const Symbol *agent, token_type func, const Symbol *multilink, const string attribute)
{
    string member = MultilinkAttributeSymbol::member_name(func, multilink, attribute);
    string result = Symbol::symbol_name(member, agent);
    return result;
}

//static
Symbol * MultilinkAttributeSymbol::create_symbol(const Symbol *agent, token_type func, const Symbol *multilink, const string attribute)
{
    Symbol *sym = nullptr;
    string unm = MultilinkAttributeSymbol::symbol_name(agent, func, multilink, attribute);
    auto it = symbols.find(unm);
    if (it != symbols.end())
        sym = it->second;
    else {
        string nm = MultilinkAttributeSymbol::member_name(func, multilink, attribute);
        sym = new MultilinkAttributeSymbol(agent, func, multilink, attribute);
    }

    return sym;

}
