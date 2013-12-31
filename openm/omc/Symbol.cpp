/**
 * @file    Symbol.cpp
 * Implements functionality of the static members of the Symbol class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
#include <typeinfo>
#include <set>
#include "Symbol.h"
#include "Symbol_simple.h" // For VersionSymbol, TypeDeclSymbol, ParameterSymbol
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
#include "token.h"

symbol_map_type Symbol::symbols;

list<AgentSymbol *> Symbol::pp_agents;

list<TableSymbol *> Symbol::pp_tables;

list<ParameterSymbol *> Symbol::pp_parameters;

multimap<string, string> Symbol::memfunc_bodyids;

// NB: There is a direct correspondence between the following lines and code in parser_helper.cpp
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

// symbols which are always in the symbol table,
// and which have default properties unless specified by om developer.
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
    pp_agents.sort( [] (AgentSymbol *a, AgentSymbol *b) { return a->name < b->name ; } );
    pp_parameters.sort( [] (ParameterSymbol *a, ParameterSymbol *b) { return a->name < b->name ; } );
    pp_tables.sort( [] (TableSymbol *a, TableSymbol *b) { return a->name < b->name ; } );

    // Assign numeric identifiers to symbols in selected collections
    // These numeric id's are used for communicating with the meta-data API.
    int id = 0;
    for ( auto parameter : pp_parameters ) {
        parameter->pp_numeric_id = id;
        ++id;
    }
    id = 0;
    for ( auto table : pp_tables ) {
        table->pp_numeric_id = id;
        ++id;
    }

    // Sort collections in agents in lexicographic order
    for ( auto agent : pp_agents ) {
        agent->pp_agent_data_members.sort( [] (AgentDataMemberSymbol *a, AgentDataMemberSymbol *b) { return a->name < b->name ; } );
        agent->pp_agentvars.sort( [] (AgentVarSymbol *a, AgentVarSymbol *b) { return a->name < b->name ; } );
        agent->pp_agentevents.sort( [] (AgentEventSymbol *a, AgentEventSymbol *b) { return a->name < b->name ; } );
        agent->pp_agent_event_times.sort( [] (AgentEventTimeSymbol *a, AgentEventTimeSymbol *b) { return a->pp_event->name < b->pp_event->name ; } );
        agent->pp_agentfuncs.sort( [] (AgentFuncSymbol *a, AgentFuncSymbol *b) { return a->name < b->name ; } );
        agent->pp_agent_tables.sort( [] (TableSymbol *a, TableSymbol *b) { return a->name < b->name ; } );
    }

    // Sort collections in tables
    for ( auto table : pp_tables ) {
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
    for ( auto *agent : pp_agents ) {
        for ( auto *event : agent->pp_agentevents )
        {
            all_event_names.insert( event->name );
        }
    }

    // For each event in the model, find the index in the sorted list, and assign it as event_id
    for ( auto *agent : pp_agents ) {
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


