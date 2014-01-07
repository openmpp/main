/**
 * @file    Symbol.cpp
 * Definitions for the Symbol class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
#include <typeinfo>
#include <unordered_map>
#include <set>
#include "CodeBlock.h"
#include "Symbol.h"
#include "VersionSymbol.h"
#include "TypeDeclSymbol.h"
#include "ParameterSymbol.h"
#include "AgentSymbol.h"
#include "AgentDataMemberSymbol.h"
#include "AgentDataMemberSymbol.h"
#include "AgentEventSymbol.h"
#include "AgentEventTimeSymbol.h"
#include "AgentFuncSymbol.h"
#include "AgentVarSymbol.h"
#include "TableExpressionSymbol.h"
#include "TableAccumulatorSymbol.h"
#include "TableAnalysisAgentVarSymbol.h"
#include "TableSymbol.h"

symbol_map_type Symbol::symbols;

list<TypeDeclSymbol *> Symbol::pp_all_types;

list<AgentSymbol *> Symbol::pp_all_agents;

list<TableSymbol *> Symbol::pp_all_tables;

list<ParameterSymbol *> Symbol::pp_all_parameters;

multimap<string, string> Symbol::memfunc_bodyids;

/**
* Map from a token to the preferred string representation of that token.
*
* This map has a unique key and maps the symbol enum to the preferred term. There is an exact
* one-to-one correspondence with code in @a parser.y. Maintain this correspondence in all
* changes or additions. Unfortunately, bison 2.7 with C++ does not expose yytname so we need to
* create hard-coded equivalent.
*/

unordered_map<token_type, string, std::hash<int> > Symbol::token_string =
{
    // top level om keywords, in alphabetic order
    { token::TK_agent, "agent" },
    { token::TK_agent_set, "agent_set" },
    { token::TK_aggregation, "aggregation" },
    { token::TK_classification, "classification" },
    { token::TK_counter_type, "counter_type" },
    { token::TK_dependency, "dependency" },
    { token::TK_extend_parameter, "extend_parameter" },
    { token::TK_hide, "hide" },
    { token::TK_import, "import" },
    { token::TK_index_type, "index_type" },
    { token::TK_integer_type, "integer_type" },
    { token::TK_languages, "languages" },
    { token::TK_link, "link" },
    { token::TK_model_generated_parameter_group, "model_generated_parameter_group" },
    { token::TK_model_type, "model_type" },
    { token::TK_options, "options" },
    { token::TK_parameter_group, "parameter_group" },
    { token::TK_parameters, "parameters" },
    { token::TK_partition, "partition" },
    { token::TK_range, "range" },
    { token::TK_real_type, "real_type" },
    { token::TK_string, "string" },
    { token::TK_table, "table" },
    { token::TK_table_group, "table_group" },
    { token::TK_time_type, "time_type" },
    { token::TK_track, "track" },
    { token::TK_user_table, "user_table" },
    { token::TK_version, "version" },

    // body level om keywords, in alphabetic order
    { token::TK_active_spell_delta, "active_spell_delta" },
    { token::TK_active_spell_duration, "active_spell_duration" },
    { token::TK_active_spell_weighted_duration, "active_spell_weighted_duration" },
    { token::TK_agent_id, "agent_id" },
    { token::TK_aggregate, "aggregate" },
    { token::TK_all_base_states, "all_base_states" },
    { token::TK_all_derived_states, "all_derived_states" },
    { token::TK_all_internal_states, "all_internal_states" },
    { token::TK_all_links, "all_links" },
    { token::TK_bounds_errors, "bounds_errors" },
    { token::TK_case_based, "case_based" },
    { token::TK_case_checksum, "case_checksum" },
    { token::TK_cell_based, "cell_based" },
    { token::TK_changes, "changes" },
    { token::TK_completed_spell_delta, "completed_spell_delta" },
    { token::TK_completed_spell_duration, "completed_spell_duration" },
    { token::TK_completed_spell_weighted_duration, "completed_spell_weighted_duration" },
    { token::TK_count, "count" },
    { token::TK_counter, "counter" },
    { token::TK_cumrate, "cumrate" },
    { token::TK_delta, "delta" },
    { token::TK_delta2, "delta2" },
    { token::TK_duration, "duration" },
    { token::TK_duration_counter, "duration_counter" },
    { token::TK_duration_trigger, "duration_trigger" },
    { token::TK_entrances, "entrances" },
    { token::TK_event, "event" },
    { token::TK_event_trace, "event_trace" },
    { token::TK_exits, "exits" },
    { token::TK_file, "file" },
    { token::TK_filter, "filter" },
    { token::TK_fp_consistency, "fp_consistency" },
    { token::TK_haz1rate, "haz1rate" },
    { token::TK_haz2rate, "haz2rate" },
    { token::TK_hook, "hook" },
    { token::TK_IMPLEMENT_HOOK, "IMPLEMENT_HOOK" },
    { token::TK_index, "index" },
    { token::TK_index_errors, "index_errors" },
    { token::TK_integer, "integer" },
    { token::TK_interval, "interval" },
    { token::TK_just_in_time, "just_in_time" },
    { token::TK_max, "max" },
    { token::TK_max_delta, "max_delta" },
    { token::TK_max_over, "max_over" },
    { token::TK_max_value_in, "max_value_in" },
    { token::TK_max_value_out, "max_value_out" },
    { token::TK_min, "min" },
    { token::TK_min_delta, "min_delta" },
    { token::TK_min_over, "min_over" },
    { token::TK_min_value_in, "min_value_in" },
    { token::TK_min_value_out, "min_value_out" },
    { token::TK_model_generated, "model_generated" },
    { token::TK_nz_delta, "nz_delta" },
    { token::TK_nz_value_in, "nz_value_in" },
    { token::TK_nz_value_out, "nz_value_out" },
    { token::TK_off, "off" },
    { token::TK_on, "on" },
    { token::TK_order, "order" },
    { token::TK_packing_level, "packing_level" },
    { token::TK_permit_all_cus, "permit_all_cus" },
    { token::TK_piece_linear, "piece_linear" },
    { token::TK_RandomStream, "RandomStream" },
    { token::TK_rate, "rate" },
    { token::TK_real, "real" },
    { token::TK_self_scheduling_int, "self_scheduling_int" },
    { token::TK_self_scheduling_split, "self_scheduling_split" },
    { token::TK_sparse, "sparse" },
    { token::TK_split, "split" },
    { token::TK_sum, "sum" },
    { token::TK_sum_over, "sum_over" },
    { token::TK_Time, "Time" },
    { token::TK_time_based, "time_based" },
    { token::TK_time_infinite, "time_infinite" },
    { token::TK_time_keeping, "time_keeping" },
    { token::TK_time_undef, "time_undef" },
    { token::TK_transitions, "transitions" },
    { token::TK_trigger_changes, "trigger_changes" },
    { token::TK_trigger_entrances, "trigger_entrances" },
    { token::TK_trigger_exits, "trigger_exits" },
    { token::TK_trigger_transitions, "trigger_transitions" },
    { token::TK_uchar, "uchar" },
    { token::TK_uint, "uint" },
    { token::TK_ulong, "ulong" },
    { token::TK_undergone_change, "undergone_change" },
    { token::TK_undergone_entrance, "undergone_entrance" },
    { token::TK_undergone_exit, "undergone_exit" },
    { token::TK_undergone_transition, "undergone_transition" },
    { token::TK_unit, "unit" },
    { token::TK_ushort, "ushort" },
    { token::TK_value_at_changes, "value_at_changes" },
    { token::TK_value_at_entrances, "value_at_entrances" },
    { token::TK_value_at_exits, "value_at_exits" },
    { token::TK_value_at_first_change, "value_at_first_change" },
    { token::TK_value_at_first_entrance, "value_at_first_entrance" },
    { token::TK_value_at_first_exit, "value_at_first_exit" },
    { token::TK_value_at_first_transition, "value_at_first_transition" },
    { token::TK_value_at_latest_change, "value_at_latest_change" },
    { token::TK_value_at_latest_entrance, "value_at_latest_entrance" },
    { token::TK_value_at_latest_exit, "value_at_latest_exit" },
    { token::TK_value_at_latest_transition, "value_at_latest_transition" },
    { token::TK_value_at_transitions, "value_at_transitions" },
    { token::TK_value_in, "value_in" },
    { token::TK_value_in2, "value_in2" },
    { token::TK_value_out, "value_out" },
    { token::TK_value_out2, "value_out2" },
    { token::TK_weighted_cumulation, "weighted_cumulation" },
    { token::TK_weighted_duration, "weighted_duration" },

    // C++ reserved words, in alphabetic order
    // Source: http://en.cppreference.com/w/cpp/keyword
    { token::TK_alignas, "alignas" },
    { token::TK_alignof, "alignof" },
    { token::TK_and, "and" },
    { token::TK_and_eq, "and_eq" },
    { token::TK_asm, "asm" },
    { token::TK_auto, "auto" },
    { token::TK_bitand, "bitand" },
    { token::TK_bitor, "bitor" },
    { token::TK_bool, "bool" },
    { token::TK_break, "break" },
    { token::TK_case, "case" },
    { token::TK_catch, "catch" },
    { token::TK_char, "char" },
    { token::TK_char16_t, "char16_t" },
    { token::TK_char32_t, "char32_t" },
    { token::TK_class, "class" },
    { token::TK_compl, "compl" },
    { token::TK_const, "const" },
    { token::TK_constexpr, "constexpr" },
    { token::TK_const_cast, "const_cast" },
    { token::TK_continue, "continue" },
    { token::TK_decltype, "decltype" },
    { token::TK_default, "default" },
    { token::TK_delete, "delete" },
    { token::TK_do, "do" },
    { token::TK_double, "double" },
    { token::TK_dynamic_cast, "dynamic_cast" },
    { token::TK_else, "else" },
    { token::TK_enum, "enum" },
    { token::TK_explicit, "explicit" },
    { token::TK_export, "export" },
    { token::TK_extern, "extern" },
    { token::TK_false, "false" },
    { token::TK_float, "float" },
    { token::TK_for, "for" },
    { token::TK_friend, "friend" },
    { token::TK_goto, "goto" },
    { token::TK_if, "if" },
    { token::TK_inline, "inline" },
    { token::TK_int, "int" },
    { token::TK_long, "long" },
    { token::TK_mutable, "mutable" },
    { token::TK_namespace, "namespace" },
    { token::TK_new, "new" },
    { token::TK_noexcept, "noexcept" },
    { token::TK_not, "not" },
    { token::TK_not_eq, "not_eq" },
    { token::TK_nullptr, "nullptr" },
    { token::TK_operator, "operator" },
    { token::TK_or, "or" },
    { token::TK_or_eq, "or_eq" },
    { token::TK_private, "private" },
    { token::TK_protected, "protected" },
    { token::TK_public, "public" },
    { token::TK_register, "register" },
    { token::TK_reinterpret_cast, "reinterpret_cast" },
    { token::TK_return, "return" },
    { token::TK_short, "short" },
    { token::TK_signed, "signed" },
    { token::TK_sizeof, "sizeof" },
    { token::TK_static, "static" },
    { token::TK_static_assert, "static_assert" },
    { token::TK_static_cast, "static_cast" },
    { token::TK_struct, "struct" },
    { token::TK_switch, "switch" },
    { token::TK_template, "template" },
    { token::TK_this, "this" },
    { token::TK_thread_local, "thread_local" },
    { token::TK_throw, "throw" },
    { token::TK_true, "true" },
    { token::TK_try, "try" },
    { token::TK_typedef, "typedef" },
    { token::TK_typeid, "typeid" },
    { token::TK_typename, "typename" },
    { token::TK_union, "union" },
    { token::TK_unsigned, "unsigned" },
    { token::TK_using, "using" },
    { token::TK_virtual, "virtual" },
    { token::TK_void, "void" },
    { token::TK_volatile, "volatile" },
    { token::TK_wchar_t, "wchar_t" },
    { token::TK_while, "while" },
    { token::TK_xor, "xor" },
    { token::TK_xor_eq, "xor_eq" },

    // C++ operators & symbols
    // Source: http://en.cppreference.com/w/cpp/keyword

    // assignment
    { token::TK_ASSIGN, "=" },
    { token::TK_PLUS_ASSIGN, "+=" },
    { token::TK_MINUS_ASSIGN, "-=" },
    { token::TK_TIMES_ASSIGN, "*=" },
    { token::TK_DIV_ASSIGN, "/=" },
    { token::TK_MOD_ASSIGN, "%=" },
    { token::TK_BWAND_ASSIGN, "&=" },
    { token::TK_BWOR_ASSIGN, "|=" },
    { token::TK_BWXOR_ASSIGN, "^=" },
    { token::TK_LSHIFT_ASSIGN, "<<=" },
    { token::TK_RSHIFT_ASSIGN, ">>=" },

    // increment / decrement
    { token::TK_INCREMENT, "++" },
    { token::TK_DECREMENT, "--" },

    // arithmetic
    { token::TK_PLUS, "+" },
    { token::TK_MINUS, "-" },
    { token::TK_DIV, "/" },
    { token::TK_MOD, "%" },
    { token::TK_BW_NOT, "~" },
    { token::TK_BW_OR, "|" },
    { token::TK_BW_XOR, "^" },
    { token::TK_BW_LSHIFT, "<<" },
    { token::TK_BW_RSHIFT, ">>" },

    // logical
    { token::TK_LOGICAL_NOT, "!" },
    { token::TK_LOGICAL_AND, "&&" },
    { token::TK_LOGICAL_OR, "||" },

    // comparison
    { token::TK_EQ, "==" },
    { token::TK_NE, "!=" },
    { token::TK_LT, "<" },
    { token::TK_GT, ">" },
    { token::TK_LE, "<=" },
    { token::TK_GE, ">=" },

    // member access
    { token::TK_MEMBER_OF, "." },
    { token::TK_MEMBER_OF_POINTER, "->" },
    { token::TK_POINTER_TO_MEMBER, ".*" },
    { token::TK_POINTER_TO_MEMBER_OF_POINTER, "->*" },

    // multiple categories
    { token::TK_STAR, "*" },
    { token::TK_AMPERSAND, "&" },

    // other
    { token::TK_SCOPE_RESOLUTION, "::" },
    { token::TK_QUESTION_MARK, "?" },
    { token::TK_COLON, ":" },
    { token::TK_COMMA, "," },
    { token::TK_SEMICOLON, ";" },

    // braces, brackets, parentheses
    { token::TK_LEFT_BRACE, "{" },
    { token::TK_RIGHT_BRACE, "}" },
    { token::TK_LEFT_BRACKET, "[" },
    { token::TK_RIGHT_BRACKET, "]" },
    { token::TK_LEFT_PAREN, "(" },
    { token::TK_RIGHT_PAREN, ")" },


    { token::TK_error, "error" },

};

// NB: There is a direct correspondence between the following lines and code in token.cpp
// Maintain exactly the same order.
unordered_set<token_type, std::hash<int> > Symbol::om_outer_keywords =
{
    token::TK_agent,
    token::TK_agent_set,
    token::TK_aggregation,
    token::TK_classification,
    token::TK_counter_type,
    token::TK_dependency,
    token::TK_extend_parameter,
    token::TK_hide,
    token::TK_import,
    token::TK_index_type,
    token::TK_integer_type,
    token::TK_languages,
    token::TK_link,
    token::TK_model_generated_parameter_group,
    token::TK_model_type,
    token::TK_options,
    token::TK_parameter_group,
    token::TK_parameters,
    token::TK_partition,
    token::TK_range,
    token::TK_real_type,
    token::TK_string,
    token::TK_table,
    token::TK_table_group,
    token::TK_time_type,
    token::TK_track,
    token::TK_user_table,
    token::TK_version,
};

unordered_map<string, token_type> Symbol::string_token =
{
    { "actor", token::TK_agent },
    { "actor_id", token::TK_agent_id },
    { "actor_set", token::TK_agent_set },
    { "logical", token::TK_bool },
    { "TRUE", token::TK_true },
    { "FALSE", token::TK_false },
    { "NULL", token::TK_nullptr },
    { "TIME", token::TK_Time },
    { "TIME_INFINITE", token::TK_time_infinite },
    { "TIME_UNDEF", token::TK_time_undef },
};


unordered_set<string> Symbol::om_developer_functions =
{
    "Simulation",
    "CaseSimulation",
    "Start",
    "Finish",
    "PreSimulation",
    "UserTables",
};

forward_list<string> Symbol::cxx_comments;

forward_list<string> Symbol::c_comments;

void Symbol::post_parse(int pass)
{
}

CodeBlock Symbol::cxx_declaration_agent()
{
    // An empty CodeBlock to start with
    return CodeBlock();
}

CodeBlock Symbol::cxx_definition_agent()
{
    // An empty CodeBlock to start with
    return CodeBlock();
}

CodeBlock Symbol::cxx_declaration_global()
{
    // An empty CodeBlock to start with
    return CodeBlock();
}

CodeBlock Symbol::cxx_definition_global()
{
    // An empty CodeBlock to start with
    return CodeBlock();
}

void Symbol::populate_metadata(openm::MetaModelHolder & metaRows)
{
}



void Symbol::default_symbols()
{
    Symbol *sym;
    sym = new VersionSymbol( 1, 0, 0, 0 );
    sym = new TypeDeclSymbol( token::TK_Time, token::TK_double );
    sym = new TypeDeclSymbol( token::TK_model_type, token::TK_case_based );
    sym = new TypeDeclSymbol( token::TK_real, token::TK_double );
    sym = new TypeDeclSymbol( token::TK_counter, token::TK_int );
    sym = new TypeDeclSymbol( token::TK_integer, token::TK_int );
    sym = new TypeDeclSymbol( token::TK_index, token::TK_int );
}

/**
 * Check for existence of symbol with this unique name.
 *
 * @param   unm The unique name.
 *
 * @return  true if found, else false.
 */

bool Symbol::exists( const string& unm )
{
    return symbols.count( unm ) == 0 ? false : true;
}

/**
 * Check for existence of symbol with this member name in agent
 *
 * @param   nm      The member name.
 * @param   agent   The agent.
 *
 * @return  true if found, else false.
 */

bool Symbol::exists( const string& nm, const Symbol *agent )
{
    string unm = symbol_name( nm, agent );
    return exists( unm );
}

/**
 * Agent member unique name.
 *
 * @param   member  The member name, e.g. "time".
 * @param   agent   The agent qualifying the member name.
 *
 * @return  The unique name, e.g. "Person::time".
 */

string Symbol::symbol_name( const string& member, const Symbol *agent )
{
    return agent->name + "::" + member;
}


/**
 * Gets a symbol for a unique name
 *
 * @param   unm The unique name
 *
 * @return  The symbol.
 */

Symbol *Symbol::get_symbol(const string& unm)
{
    Symbol *sym = nullptr;
	auto it = symbols.find( unm );
    if ( it != symbols.end() ) 
        sym = it->second;
    else
        sym = new Symbol( unm );

    return sym;
}

/**
 * Gets a symbol for a member name in an agent.
 *
 * @param   member  The member name.
 * @param   agent   The agent.
 *
 * @return  The symbol.
 */

Symbol *Symbol::get_symbol( const string& member, const Symbol *agent )
{
    Symbol *sym = nullptr;
    string unm = Symbol::symbol_name( member, agent );
	auto it = symbols.find( unm );
	if (it != symbols.end() )
        sym = it->second;
    else
        sym = new Symbol( member, agent );

    return sym;
}




/**
 * Searches for the first symbol of the given class
 *
 * @param   ti  The typeinfo of the class to find.
 *
 * @return  null if it fails, else the found symbol.
 */

Symbol *Symbol::find_a_symbol( const type_info& ti )
{
    auto it = find_if(  symbols.begin(),
                        symbols.end(),
                        [&ti] ( const symbol_map_value_type& vt )
                        {
                            return typeid(*vt.second) == ti;
                        }
                     );
    if ( symbols.end() != it )
        return it->second;
    else
        return nullptr;
}

/**
 * Perform operations after the parser has parsed all input files.
 * 
 * This includes validating each symbol and populating post-parse symbol members containing
 * information on symbol relationships and pointer collections.
 */

void Symbol::post_parse_all()
{
    // pass 0: create support symbols not identified during parsing
    for_each(   symbols.begin(),
                symbols.end(),
                [] (symbol_map_value_type& vt)
                { 
                    (vt.second)->post_parse( 0 );
                }
            );

    // pass 1: create pp_ members and collections
    for_each(   symbols.begin(),
                symbols.end(),
                [] (symbol_map_value_type& vt)
                { 
                    (vt.second)->post_parse( 1 );
                }
            );

    // Sort all global collections in lexicographic order
    pp_all_types.sort([](TypeDeclSymbol *a, TypeDeclSymbol *b) { return a->name < b->name; });
    pp_all_agents.sort( [] (AgentSymbol *a, AgentSymbol *b) { return a->name < b->name ; } );
    pp_all_parameters.sort( [] (ParameterSymbol *a, ParameterSymbol *b) { return a->name < b->name ; } );
    pp_all_tables.sort( [] (TableSymbol *a, TableSymbol *b) { return a->name < b->name ; } );

    // Assign numeric identifiers to symbols in selected collections
    // These numeric id's are used for communicating with the meta-data API.
    int id = 0;
    for ( auto parameter : pp_all_parameters ) {
        parameter->pp_numeric_id = id;
        ++id;
    }
    id = 0;
    for ( auto table : pp_all_tables ) {
        table->pp_numeric_id = id;
        ++id;
    }

    // Sort collections in agents in lexicographic order
    for ( auto agent : pp_all_agents ) {
        agent->pp_agent_data_members.sort( [] (AgentDataMemberSymbol *a, AgentDataMemberSymbol *b) { return a->name < b->name ; } );
        agent->pp_agentvars.sort( [] (AgentVarSymbol *a, AgentVarSymbol *b) { return a->name < b->name ; } );
        agent->pp_agentevents.sort( [] (AgentEventSymbol *a, AgentEventSymbol *b) { return a->name < b->name ; } );
        agent->pp_agent_event_times.sort( [] (AgentEventTimeSymbol *a, AgentEventTimeSymbol *b) { return a->pp_event->name < b->pp_event->name ; } );
        agent->pp_agentfuncs.sort( [] (AgentFuncSymbol *a, AgentFuncSymbol *b) { return a->name < b->name ; } );
        agent->pp_agent_tables.sort( [] (TableSymbol *a, TableSymbol *b) { return a->name < b->name ; } );
    }

    // Sort collections in tables
    for ( auto table : pp_all_tables ) {
        // Sort expressions in sequence order
        table->pp_expressions.sort( [] (TableExpressionSymbol *a, TableExpressionSymbol *b) { return a->index < b->index; } );
        // Sort accumulators in sequence order
        table->pp_accumulators.sort( [] (TableAccumulatorSymbol *a, TableAccumulatorSymbol *b) { return a->index < b->index; } );
        // Sort referenced agentvars in sequence order
        table->pp_table_agentvars.sort( [] (TableAnalysisAgentVarSymbol *a, TableAnalysisAgentVarSymbol *b) { return a->index < b->index; } );
    }

    // Create an amalgamated set of event names, in all agents, sorted lexicographically.
    // Note that the set contains no duplicates, but event names can be duplicates in different agents.
    set<string> all_event_names;
    for ( auto *agent : pp_all_agents ) {
        for ( auto *event : agent->pp_agentevents )
        {
            all_event_names.insert( event->name );
        }
    }

    // For each event in the model, find the index in the sorted list, and assign it as event_id
    for ( auto *agent : pp_all_agents ) {
        for ( auto *event : agent->pp_agentevents )
        {
            auto iter = all_event_names.find( event->name );
            event->pp_event_id = distance( all_event_names.begin(), iter);
        }
    }

    // Pass 2: populate additional collections for subsequent code generation, e.g. for side_effect functions.
    // In this pass, symbols 'reach out' to dependent symbols and populate collections for implementing dependencies.
    for_each(   symbols.begin(),
                symbols.end(),
                [] (symbol_map_value_type& vt)
                { 
                    (vt.second)->post_parse( 2 );
                }
            );

}

void Symbol::process_cxx_comment(const string& cmt, yy::location& loc)
{
    // TODO just a placeholder container
    cxx_comments.push_front(cmt);
}

void Symbol::process_c_comment(const string& cmt, yy::location& loc)
{
    // TODO just a placeholder container
    c_comments.push_front(cmt);
}

bool Symbol::is_om_outer_keyword(const token_type& tok)
{
    return om_outer_keywords.count(tok) == 0 ? false : true;
}

bool Symbol::is_om_developer_function(const char* nm)
{
    return om_developer_functions.count(nm) == 0 ? false : true;
}


//static
const string Symbol::token_to_string(const token_type& e)
{
    auto i = token_string.find(e);
    return i->second;
}

//static
const token_type Symbol::string_to_token(const char * s)
{
    static bool initialization_done = false;

    if (!initialization_done) {
        // populate using reciprocal map
        for (auto i : token_string) string_token.emplace(i.second, i.first);
        initialization_done = true;
    }
    auto i = string_token.find(s);
    if (i == string_token.end()) return token::TK_error;
    else return i->second;
}

// static
const token_type Symbol::modgen_cumulation_operator_to_acc(const token_type& e)
{
    token_type result;
    switch (e) {
    case token::TK_delta:
        result = token::TK_sum;
        break;
    case token::TK_delta2:
        result = token::TK_sum;
        break;
    case token::TK_nz_delta:
        result = token::TK_sum;
        break;
    case token::TK_value_in:
        result = token::TK_sum;
        break;
    case token::TK_value_in2:
        result = token::TK_sum;
        break;
    case token::TK_nz_value_in:
        result = token::TK_sum;
        break;
    case token::TK_value_out:
        result = token::TK_sum;
        break;
    case token::TK_value_out2:
        result = token::TK_sum;
        break;
    case token::TK_nz_value_out:
        result = token::TK_sum;
        break;
    case token::TK_max_delta:
        result = token::TK_max;
        break;
    case token::TK_max_value_in:
        result = token::TK_max;
        break;
    case token::TK_max_value_out:
        result = token::TK_max;
        break;
    case token::TK_min_delta:
        result = token::TK_min;
        break;
    case token::TK_min_value_in:
        result = token::TK_min;
        break;
    case token::TK_min_value_out:
        result = token::TK_min;
        break;
    default:
        assert(false);
        result = token::TK_error;
    }
    return result;
}


//static
const token_type Symbol::modgen_cumulation_operator_to_incr(const token_type& e)
{
    token_type result;
    switch (e) {
    case token::TK_delta:
        result = token::TK_delta;
        break;
    case token::TK_delta2:
        result = token::TK_delta2;
        break;
    case token::TK_nz_delta:
        result = token::TK_sum;
        break;
    case token::TK_value_in:
        result = token::TK_value_in;
        break;
    case token::TK_value_in2:
        result = token::TK_value_in2;
        break;
    case token::TK_nz_value_in:
        result = token::TK_nz_value_in;
        break;
    case token::TK_value_out:
        result = token::TK_value_out;
        break;
    case token::TK_value_out2:
        result = token::TK_value_out2;
        break;
    case token::TK_nz_value_out:
        result = token::TK_nz_value_out;
        break;
    case token::TK_max_delta:
        result = token::TK_delta;
        break;
    case token::TK_max_value_in:
        result = token::TK_value_in;
        break;
    case token::TK_max_value_out:
        result = token::TK_value_out;
        break;
    case token::TK_min_delta:
        result = token::TK_delta;
        break;
    case token::TK_min_value_in:
        result = token::TK_value_in;
        break;
    case token::TK_min_value_out:
        result = token::TK_value_out;
        break;
    default:
        assert(false);
        result = token::TK_error;
    }
    return result;
}



