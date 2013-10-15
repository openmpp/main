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
#include "ast.h"
#include "parser_helper.h"

/**
 * Post-parse operations for AgentEventSymbol
 */

void AgentEventSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse( pass );

    switch ( pass ) {
    case 1:
        // Add this agentevent to the agent's list of all agentevents
        pp_agent->pp_agentevents.push_back( this );
        break;
    default:
        break;
    }
}

/**
 * Post-parse operations for AgentMemberSymbol
 */

void AgentMemberSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse( pass );

    switch ( pass ) {
    case 1:
        // assign direct pointer to agent for use post-parse
        pp_agent = dynamic_cast<AgentSymbol *> (agent);
        break;
    default:
        break;
    }
}

/**
 * Post-parse operations for the symbol
 */

void AgentDataMemberSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in the Symbol hierarchy
    super::post_parse( pass );

    // Now do operations specific to this level in the Symbol hierarchy
    switch ( pass ) {
    case 1:
        // Add this agent data symbol to the agent's list of all such symbols
        pp_agent->pp_agent_data_members.push_back( this );
        break;
    default:
        break;
    }
}

void AgentCallbackMemberSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in the Symbol hierarchy
    super::post_parse( pass );

    // Now do operations specific to this level in the Symbol hierarchy
    switch ( pass ) {
    case 1:
        // Nothing
        break;
    default:
        break;
    }
}

void AgentInternalSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in the Symbol hierarchy
    super::post_parse( pass );

    // Now do operations specific to this level in the Symbol hierarchy
    switch ( pass ) {
    case 1:
        // Nothing
        break;
    default:
        break;
    }
}

void AgentEventTimeSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in the Symbol hierarchy
    super::post_parse( pass );

    // Now do operations specific to this level in the Symbol hierarchy
    switch ( pass ) {
    case 1:
        // assign direct pointer(s) for use post-parse
        pp_event = dynamic_cast<AgentEventSymbol *> (event);
        // Add this agent event time symbol to the agent's list of all such symbols
        pp_agent->pp_agent_event_times.push_back( this );
        break;
    case 2:
        {
            // E.g. Person
            string agent_name = pp_agent->name;
            // E.g. Person::timeMortalityEvent
            string time_func_name = pp_event->time_function->unique_name;
            // create sorted unduplicated list of identifiers in body of event time function
            set<string> identifiers;
            auto rng = memfunc_bodyids.equal_range( time_func_name ); 
            for_each(   rng.first,
                        rng.second,
                        [&] (unordered_multimap<string, string>::value_type& vt)
                        { 
                            identifiers.insert( vt.second );
                        }
                    );
            // iterate through list looking for agentvars with name = identifier
            for ( auto identifier : identifiers ) {
                if ( exists( identifier, pp_agent ) ) {
                    auto sym = get_symbol( identifier, pp_agent );
                    auto av = dynamic_cast<AgentVarSymbol *>(sym);
                    if ( av ) {
                        // dependency of time function on av detected
                        // E.g. om_time_StartPlayingEvent.make_dirty();
                        string line = name + ".make_dirty();";
                        av->pp_side_effects.push_back( line );
                    }
                }
            }
            // add side-effect to time agentvar
            AgentVarSymbol *av = pp_agent->pp_time;
            // Eg. om_duration.wait( new_value - old_value );
            //string line = name + ".make_dirty();";
        }
        break;
    default:
        break;
    }
}



/**
 * Post-parse operations for AgentVarSymbol
 */

void AgentVarSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse( pass );

    switch ( pass ) {
    case 1:
        // Add this agentvar to the agent's list of all agentvars
        pp_agent->pp_agentvars.push_back( this );
        break;
    default:
        break;
    }
}


/**
 * Post-parse operations for DurationAgentVarSymbol
 */

void DurationAgentVarSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse( pass );

    switch ( pass ) {
    case 1:
        break;
    case 2:
        {
            // add side-effect to time agentvar
            AgentVarSymbol *av = pp_agent->pp_time;
            // Eg. om_duration.wait( new_value - old_value );
            string line = name + ".wait( new_value - old_value );";
            av->pp_side_effects.push_back( line );
        }
    default:
        break;
    }
}

/**
 * Post-parse operations for ConditionedDurationAgentVarSymbol
 */

void ConditionedDurationAgentVarSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse( pass );

    switch ( pass ) {
    case 1:
        // assign direct pointer for post-parse use
        pp_observed = dynamic_cast<AgentVarSymbol *> (observed);
        break;
    case 2:
        {
            // add side-effect to time agentvar
            AgentVarSymbol *av = pp_agent->pp_time;
            // Eg. om_duration.wait( new_value - old_value );
            string line = name + ".wait( new_value - old_value );";
            av->pp_side_effects.push_back( line );
        }
    default:
        break;
    }
}


/**
 * Post-parse operations for BuiltinAgentVarSymbol
 */

void BuiltinAgentVarSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse( pass );

    switch ( pass ) {
    case 1:
        break;
    case 2:
        if ( name == "age" ) {
            // add side-effect to time agentvar
            AgentVarSymbol *av = pp_agent->pp_time;
            string line = "age.set( age.get() + new_value - old_value );";
            av->pp_side_effects.push_back( line );
        }
    default:
        break;
    }
}

/**
 * Post-parse operations for TableExpressionSymbol
 */

void TableExpressionSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse( pass );

    switch ( pass ) {
    case 1:
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> (table);
        // Add this table expression to the table's list of expressions
        pp_table->pp_expressions.push_back( this );
        // Perform post-parse operation to each element of the expression
        post_parse_traverse( root );
        
        break;
    default:
        break;
    }
}

/**
    * Perform post-parse operations on nodes of a ExprForTable tree
    *
    * @param   node    The root of the expression tree.
    *
    * @return  Result as a \a CodeBlock
    */

void TableExpressionSymbol::post_parse_traverse( ExprForTable *node )
{
    auto leaf = dynamic_cast<ExprForTableLeaf *>( node );
    if ( leaf != nullptr ) {
        (leaf->pp_accumulator) = dynamic_cast<TableAccumulatorSymbol *>( leaf->accumulator );
    }
    else {
        auto op = dynamic_cast<ExprForTableOp *>(node);
        assert( op );
        post_parse_traverse ( op->left );
        post_parse_traverse ( op->right );
    }
}

/**
    * Get C++ expression using accumulators
    *
    * @param   node    The root of the expression tree.
    *
    * @return  Result as a \a CodeBlock
    */

string TableExpressionSymbol::get_expression( const ExprForTable *node, expression_style style )
{
    const ExprForTableLeaf* leaf = dynamic_cast<const ExprForTableLeaf *>( node );
    if ( leaf != nullptr ) {
        const TableAccumulatorSymbol *accumulator = leaf->pp_accumulator;
        string result;
        switch ( style ) {
        case cxx:
            result = "accumulators[" + to_string( accumulator->index ) + "][cell]";
            break;
        case sql:
            {
                string agg_func = "";
                switch ( accumulator->accumulator ) {
                case token::TK_sum:
                    agg_func = "OM_SUM";
                    break;
                case token::TK_min:
                    agg_func = "OM_MIN";
                    break;
                case token::TK_max:
                    agg_func = "OM_MAX";
                    break;
                default:
                    assert( 0 );
                }
                result = agg_func + "( acc" + to_string( accumulator->index ) + " )";
                break;
            }
        default:
            assert( 0 );
        }
        return result;
    }
    else {
        auto binary_node = dynamic_cast<const ExprForTableOp *>(node);
        assert( binary_node );
        string expr_left = get_expression ( binary_node->left, style );
        string expr_right = get_expression ( binary_node->right, style );
        return "( " + expr_left + " " + token_to_string( binary_node->op ) + " " + expr_right + " )";
    }
}

/**
 * Post-parse operations for TableExpressionSymbol
 */

void TableAccumulatorSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse( pass );

    switch ( pass ) {
    case 1:
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> ( table );
        assert(pp_table); // grammar guarantee

        // assign direct pointer to agentvar for post-parse use
        pp_agentvar = dynamic_cast<AgentVarSymbol *> ( agentvar );
        assert(pp_agentvar); // grammar guarantee

        // assign direct pointer to TableAnalysisAgentVarSymbol for post-parse use
        pp_analysis_agentvar = dynamic_cast<TableAnalysisAgentVarSymbol *> ( analysis_agentvar );
        assert(pp_analysis_agentvar); // grammar guarantee

        // Add this table accumulator to the table's list of accumulators
        pp_table->pp_accumulators.push_back( this );
        
        break;
    default:
        break;
    }
}

/**
 * Post-parse operations for TableAnalysisAgentVarSymbol
 */

void TableAnalysisAgentVarSymbol::post_parse( int pass )
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse( pass );

    switch ( pass ) {
    case 0:
        if ( need_value_in ) {
            // Create symbol for the data member which will hold the 'in' value of the increment.
            auto av = dynamic_cast<AgentVarSymbol *>( agentvar );
            assert( av ); // TODO: catch developer code error - table analysis agentvar not declared
            string member_name = in_agentvar_name();
            auto sym = new AgentInternalSymbol( member_name, av->agent, av->type );
        }
        break;
    case 1:
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> ( table );
        assert(pp_table); // grammar guarantee

        // assign direct pointer to agentvar for post-parse use
        pp_agentvar = dynamic_cast<AgentVarSymbol *> ( agentvar );
        assert(pp_agentvar); // grammar guarantee

        // Add this TableAnalysisAgentVarSymbol to the table's list of agentvars
        pp_table->pp_table_agentvars.push_back( this );
        
        break;
    default:
        break;
    }
}

CodeBlock TableAnalysisAgentVarSymbol::cxx_prepare_increment() const
{
    assert( pp_table );  // only call post-parse
    assert( pp_agentvar );  // only call post-parse
    CodeBlock c;
    // example:              DurationOfLife.value_in.alive = alive;\n
    c += in_agentvar_name() + " = " + pp_agentvar->name + ";";
    return c;
}

// Static members of the Symbol class. 

/**
 * The symbol table
 */

symbol_map_type Symbol::symbols;

/**
 * The agents in the model
 * 
 * Populated after parsing is complete.
 */

list<AgentSymbol *> Symbol::pp_agents;

/**
 * The tables in the model
 * 
 * Populated after parsing is complete.
 */

list<TableSymbol *> Symbol::pp_tables;

/**
 * The parameters in the model
 * 
 * Populated after parsing is complete.
 */

list<ParameterSymbol *> Symbol::pp_parameters;

/**
 * Map of member function qualified names to all identifiers used in the function body. Example
 * entry might be "Person::Mortality" ==> "alive".
 */

multimap<string, string> Symbol::memfunc_bodyids;

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
