/**
* @file    EntityEventSymbol.cpp
* Definitions for the EntityEventSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

//#include <cassert>
//#include <algorithm>
//#include <typeinfo>
#include <cassert>
#include <set>
#include "EntityEventSymbol.h"
#include "EntitySymbol.h"
#include "AttributeSymbol.h"
#include "BuiltinAttributeSymbol.h"
#include "LinkAttributeSymbol.h"
#include "EntityMultilinkSymbol.h"
#include "EntityFuncSymbol.h"
#include "LanguageSymbol.h"
#include "CodeBlock.h"
#include "omc_file.h" // for LTA support

using namespace std;
using namespace omc; // for LTA support

void EntityEventSymbol::create_auxiliary_symbols(Symbol *tfs, Symbol *ifs, bool is_developer_supplied)
{
    if (is_developer_supplied) {
        // Create an EntityFuncSymbol for the time function ('true' means the definition is developer-supplied, so suppress definition)
        time_func = new EntityFuncSymbol(tfs, entity, "Time", "", true);
        time_func->doc_block = doxygen_short("Return the time to the event " + event_name + " in the " + entity->name + " entity (model code).");
        time_func->associated_event = this;

        // Create an EntityFuncSymbol for the implement function ('true' means the definition is developer-supplied, so suppress definition)
        implement_func = new EntityFuncSymbol(ifs, entity, "void", "", true);
        implement_func->doc_block = doxygen_short("Implement the event " + event_name + " when it occurs in the " + entity->name + " entity (model code).");
        implement_func->associated_event = this;
    }
    else {
        // The functions are created internally (for the internally-generated self-scheduling event)
        time_func = dynamic_cast<EntityFuncSymbol *>(tfs);
        assert(time_func);
        implement_func = dynamic_cast<EntityFuncSymbol *>(ifs);
        assert(implement_func);
    }
    time_func_original = time_func;
    implement_func_original = implement_func;
}

void EntityEventSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        // First determine if this is an event with memory or not,
        // since if so the members of the event trace cover function symbols 
        // possibly created below will depend on that.
        bool time_func_defined = false;
        bool time_func_memory = false;
        {
            assert(time_func); // logic guarantee
            auto search = function_parmlist.find(time_func->unique_name);
            if (search == function_parmlist.end()) {
                time_func_defined = false; // this is an error
            }
            else {
                time_func_defined = true;
                auto parmlist = search->second;
                // see if time function parmlist starts with '( int *'
                if (parmlist.size() >= 2 && parmlist[0] == "int" && parmlist[1] == "*") {
                    time_func_memory = true;
                }
            }
        }

        bool implement_func_defined = false;
        bool implement_func_memory = false;
        {
            assert(implement_func); // logic guarantee
            auto search = function_parmlist.find(implement_func->unique_name);
            if (search == function_parmlist.end()) {
                implement_func_defined = false; // this is an error
            }
            else {
                implement_func_defined = true;
                auto parmlist = search->second;
                // see if implement function parmlist starts with '( int '
                if (parmlist.size() >= 2 && parmlist[0] == "int" && parmlist[1] != "*") {
                    implement_func_memory = true;
                }
            }
        }

        if (!time_func_defined) {
            pp_error(LT("error : missing definition for event time function '") + time_func->unique_name + LT("'"));
        }
        if (!implement_func_defined) {
            pp_error(LT("error : missing definition for event implement function '") + implement_func->unique_name + LT("'"));
        }
        if (time_func_memory != implement_func_memory) {
            pp_error(LT("error : mismatching arguments in event time function '") + time_func->unique_name + LT("' and event implement function '") + implement_func->unique_name + LT("'"));
        }

        // Now record the conclusion of the preceding chunk of code.
        event_memory = time_func_memory;

        if (event_memory) {
            // Change the declaration argument lists (were empty strings)
            time_func->arg_list_decl = "int *";
            implement_func->arg_list_decl = "int";
        }

        if (Symbol::option_event_trace) {
            // The event_trace option is on, so create cover functions
            // for the time and implement functions of the event,
            // and use them instead of the original functions.
            // The cover functions perform logging and then call
            // the original functions.

            // Create an EntityFuncSymbol for the time 'cover' function
            string cover_time_func_name = "om_cover_" + time_func->name;
            auto cover_time_func = new EntityFuncSymbol(
                cover_time_func_name,
                entity,
                "Time",
                event_memory ? "int * p_event_mem" : "",
                false
            );
            cover_time_func->doc_block = doxygen_short("Logging cover function: Return the time to the event " + event_name + " in the " + entity->name + " entity.");
            // Note that the body of the cover time function is supplied in a subsequent pass below
            // Plug in the cover time function to replace the original function
            time_func = cover_time_func;

            // Create an EntityFuncSymbol for the implement 'cover' function
            string cover_implement_func_name = "om_cover_" + implement_func->name;
            auto cover_implement_func = new EntityFuncSymbol(
                cover_implement_func_name,
                entity,
                "void",
                event_memory ? "int event_mem" : "",
                false
            );
            cover_implement_func->doc_block = doxygen_short("Logging cover function: Implement the event " + event_name + " when it occurs in the " + entity->name + " entity.");
            // Note that the body of the cover implement function is supplied in a subsequent pass below
            // Plug in the cover implement function to replace the original function
            implement_func = cover_implement_func;
			// Push the names into the post parse ignore hash for the current pass.
			pp_symbols_ignore.insert(cover_time_func->unique_name);
            pp_symbols_ignore.insert(cover_implement_func->unique_name);

        }
        break;
    }
    case eResolveDataTypes:
    {
        // Any explicit Labels and Notes for an event are bound to the implement function of the event,
        // instead of to the EntityEventSymbol (this), because the name of the implement function is the event name.
        // If there are any such, propagate them to the EntityEventSymbol (this).
        {
            for (const auto& langSym : Symbol::pp_all_languages) {
                int lang_index = langSym->language_id; // 0-based
                if (implement_func->pp_labels_explicit[lang_index]) {
                    pp_labels[lang_index] = implement_func->pp_labels[lang_index];
                    pp_labels_explicit[lang_index] = true;
                }
                pp_notes[lang_index] = implement_func->pp_notes[lang_index];
            }
        }
        break;
    }
    case ePopulateCollections:
    {
        // Propagate event labels to the event time and event implement entity functions.
        for (const auto& langSym : Symbol::pp_all_languages) {
            int lang_index = langSym->language_id; // 0-based
            const string& lang = langSym->name; // e.g. "EN" or "FR"
            const string& label = pp_labels[lang_index];
            time_func->pp_labels[lang_index] = LTA(lang, "Time") + " - " + label;
            implement_func->pp_labels[lang_index] = LTA(lang, "Implement") + " - " + label;
        }

        // Add this entity event time symbol to the entity's list of all such symbols
        pp_entity->pp_events.push_back(this);

        // Add this entity event time symbol to the entity's list of all callback members
        pp_entity->pp_callback_members.push_back(this);
        break;
    }
    case ePopulateDependencies:
    {
        // Dependencies are generated directly if not developer-supplied.
        // The only event which is not developer-supplied is the special omc-generated event
        // which maintains self-scheduling attributes for the entity.
        if (!is_developer_supplied) break;
        // Iterate through list of identifiers in the body of the time function
        // whose name matches an attribute.
        for (auto& identifier : time_func_original->body_identifiers) {
            if (exists(identifier, pp_entity)) {
                // identifier is a member of this entity
                auto sym = get_symbol(identifier, pp_entity);
                auto av = dynamic_cast<AttributeSymbol *>(sym);
                if (av) {
                    // Identifier is an attribute of this entity.
                    // Dependency of time function on av detected.
                    // Note it.
                    pp_attribute_dependencies.insert(av);
                    // Implement event dependency on change in attribute value.
                    CodeBlock& c = av->side_effects_fn->func_body;
                    c += injection_description();
                    c += "// Recalculate time to event " + event_name;
                    c += "if (om_active) " + name + ".make_dirty();";
                }
            }
        }
        // Iterate through list of pointers A->B in the body of the time function
        // where A matches a link attribute and B matches an attribute in the other entity.
        for (auto& item : time_func_original->body_pointers) {
            auto& A = item.first;
            auto& B = item.second;
            if (exists(A, pp_entity)) {
                // A is an attribute of this entity
                auto sym = get_symbol(A, pp_entity);
                auto A_lnk = dynamic_cast<LinkAttributeSymbol*>(sym);
                if (A_lnk) {
                    // A is a link attribute of this entity.
                    // The reciprocal link is either a one-to-one link
                    // or a one-to-many multilink.
                    if (A_lnk->reciprocal_link) {
                        // The reciprocal link is a one-to-one link.
                        // Get the other entity.
                        auto& B_entity = A_lnk->reciprocal_link->pp_entity;
                        if (exists(B, B_entity)) {
                            // B is a member of the other entity.
                            auto sym = get_symbol(B, B_entity);
                            auto B_attribute = dynamic_cast<AttributeSymbol*>(sym);
                            if (B_attribute) {
                                // B is an attribute of the other entity.
                                // Dependency of time function on B detected.
                                // Note it.
                                pp_linked_attribute_dependencies.insert({ A_lnk, B_attribute });
                                // Implement event dependency on change in B attribute value.
                                CodeBlock& c = B_attribute->side_effects_fn->func_body;
                                c += injection_description();
                                c += "// Recalculate time to event " + event_name + " in linked entity " + pp_entity->name;
                                c += "if (om_active) {";
                                c +=   "if (" + A_lnk->reciprocal_link->name + ") {";
                                c +=     A_lnk->reciprocal_link->name + "->" + name + ".make_dirty();";
                                c +=   "}";
                                c += "}";
                            }
                        }
                    }
                    else {
                        // The reciprocal link is a one-to-many multilink
                        assert(A_lnk->reciprocal_multilink);
                        // Get the other entity
                        auto& B_entity = A_lnk->reciprocal_multilink->pp_entity;
                        if (exists(B, B_entity)) {
                            // B is a member of the other entity.
                            auto sym = get_symbol(B, B_entity);
                            auto B_attribute = dynamic_cast<AttributeSymbol*>(sym);
                            if (B_attribute) {
                                // B is an attribute of the other entity.
                                // Dependency of time function on B detected.
                                // Note it.
                                pp_linked_attribute_dependencies.insert({ A_lnk, B_attribute });
                                // Implement event dependency on change in B attribute value.
                                CodeBlock& c = B_attribute->side_effects_fn->func_body;
                                c += injection_description();
                                c += "// Recalculate time to event " + event_name + " in all linked " + pp_entity->name + "s";
                                c += "if (om_active) {";
                                c +=   "for (auto &item : " + A_lnk->reciprocal_multilink->name + ".storage) {";
                                c +=     "if (item.get() != nullptr) {";
                                c +=        "item->" + name + ".make_dirty();";
                                c +=     "}";
                                c +=   "}";
                                c += "}";
                            }
                        }
                    }
                }
            }
        }

        if (Symbol::option_event_trace) {
            // Provide the function body for each cover event trace function.
            // Done in this pass rather than earlier to provide pp_event_id to event_trace_msg.

            CodeBlock& ct = time_func->func_body;
            ct += "Time event_time = " + time_func_original->name + (event_memory ? "(p_event_mem);" : "();");
            ct += "if (event_trace_on) "
                "event_trace_msg("
                "\"" + entity->name + "\", "
                "(int)entity_id, "
                "(double)age.direct_get(), "
                "GetCaseSeed(), "
                "\"" + event_name + "\", "
                + std::to_string(pp_event_id) + ","
                "\"" + time_func_original->name + "\", "
                "(double)event_time, "              // old_time
                "(double)BaseEvent::stashed_time, " // new_time
                "(double)time.direct_get(), "
                "BaseEntity::et_msg_type::eQueuedEvent);"
                ;
            ct += "return event_time;";

            CodeBlock& ci = implement_func->func_body;
            ci += "if (event_trace_on) "
                "event_trace_msg("
                "\"" + entity->name + "\", "
                "(int)entity_id, "
                "(double)age.direct_get(), "
                "GetCaseSeed(), "
                "\"" + event_name + "\", "
                + std::to_string(pp_event_id) + ","
                "\"" + entity->name + "." + event_name + "\", "
                "(double)time, " // current_time
                "(double)age, "  // current_age
                "(double)BaseEvent::get_global_time(), "
                "BaseEntity::et_msg_type::eEventOccurrence);"
                ;
            ci += implement_func_original->name + (event_memory ? "(event_mem);" : "();");
        }
        break;
    }
    default:
    break;
    }
}

CodeBlock EntityEventSymbol::cxx_declaration_entity()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_entity();

    // Perform operations specific to this level in the Symbol hierarchy.

    int event_id = pp_event_id;
    int modgen_event_num = pp_modgen_event_num;
    h += (event_memory ? "MemoryEvent<" : "Event<")
        + entity->name + ", "
        + to_string(event_id) + ", "
        + to_string(event_priority) + ", "
        + to_string(modgen_event_num) + ", "
        + "&" + implement_func->unique_name + ", "
        + "&" + time_func->unique_name + ">";
    h += name + ";";

    return h;
}

