/**
 * @file    Symbol.h
 * Declares the Symbol class and sub-classes.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <forward_list>
#include <map>
#include <typeinfo>
#include <cassert>
#include "parser_helper.h"
#include "location.hh"
#include "Literal.h"
#include "CodeBlock.h"

using namespace std;

// ======================================================

// TODO: Explain better the use of double indirection to persist of Symbols and allow morphing within the SYmbol class hierarchy.
// When they need to be used, Symbol*& is used to get the underlying object.
// That lets us morph an object to another type during / parsing, while maintaining
// the persistence of symbol uses in other objects.
// A rule must be followed, which is to never store a pointer to a Symbol, either in a Symbol or another object.
// Instead, store a reference to the pointer of, which can always be obtained using the member function get_rpSymbol().
// Actually, it's a reference to the second pair of the element for the symbol in the symbol table (the first element of the pair is the name)
// The underlying collection std::unordered_map guarantees that references and pointers remain valid even if rehashing occurs.

// Forward declarations of all classes in the Symbol hierarchy
class AgentDataMemberSymbol;
class AgentEventSymbol;
class AgentEventTimeSymbol;
class AgentFuncSymbol;
class AgentInternalSymbol;
class AgentMemberSymbol;
class AgentSymbol;
class AgentVarSymbol;
class BuiltinAgentVarSymbol;
class ConditionedDurationAgentVarSymbol;
class DurationAgentVarSymbol;
class ExprForTable;
class LanguageSymbol;
class ModelSymbol;
class ModuleSymbol;
class ParameterSymbol;
class SimpleAgentVarSymbol;
class Symbol;
class TableSymbol;
class TableAccumulatorSymbol;
class TableAnalysisAgentVarSymbol;
class TableExpressionSymbol;
class TableSymbol;
class TypeDeclSymbol;
class VersionSymbol;

/**
 * Defines an alias representing the type of the symbol table.
 * 
 * The symbol table maps strings to pointers to symbols.
 * The use of pointers to Symbols allows polymorphic symbols in the symbol table.
 * It also allows a given symbol to be 'morphed' to a different
 * kind of symbol during parsing by replacing the Symbol pointer by a pointer to a new instance
 * of a derived class of Symbol.  Morphing requires that any persistent use of a Symbol
 * in another object must be through an additional level of indirection, i.e. Symbol **.
 */
typedef unordered_map<string, Symbol *> symbol_map_type;

/**
 * Defines an alias representing the type of a pair in the symbol table.
 */
typedef unordered_map<string, Symbol *>::value_type symbol_map_value_type;




/**
 * A Symbol is a named syntactic entity in openM++.
 * 
 * When the scanner encounters a name in the model source code which is neither a C++ keyword
 * nor an openM++ keyword, it creates an associated Symbol object, places it in
 * the symbol table, and returns it to the parser.  The parser may subsequently morph this
 * original symbol table entry to a derived class of Symbol.
 * 
 * The Symbol class also contains static members which provide all functionality
 * associated with the symbol table.
 */
class Symbol {
public:

    /**
     * Constructor for new Symbol.
     *
     * @param   unm  The unique name for the symbol
     *              
     * If the symbol table does not contain an entry with unique name @a unm
     * a new symbol table entry is created with this unique name.
     * 
     * This constructor sets the name equal to the unique name, and is used for
     * symbols outside agent scope, e.g. classifications.
     *                           
     * If the symbol table does contain an entry with unique name @a unm the existing symbol is
     * morphed to a new base class Symbol. This is useful for replacing a default symbol
     * by a symbol specified in the model source code. For example, a default
     * TypeDeclSymbol with name 'Time' and type 'double' is inserted into the
     * symbol table at initialization.  If a time_type statement is encountered in model
     * code, this symbol is replaced by a new symbol with the developer-specified type
     * used for time.
     */
    Symbol( const string unm )
        : unique_name( unm )
        , name ( unm )
        {
            auto it = symbols.find( unique_name );
            if (it == symbols.end() ) {
                // add to symbol table
                symbol_map_value_type element( unique_name, this );
                symbols.insert( element );
            }
            else {
                // redeclaration or override of default - morph to new symbol
                // delete old symbol
                delete it->second;
                // replace with new symbol
                it->second = this;
            }
        }

    /**
     * Constructor.
     *
     * @param   nm      The name.
     * @param   agent   The agent qualifying the name
     *                  
     * This constructor creates a Symbol with unique name based on 
     * @a nm in the context of @a agent, e.g. "Person::time"
     *                           
     * If the symbol table does not contain a corresponding entry
     * a new symbol table entry is created with this unique name.
     * 
     * If the symbol table does contain a corresponding entry the existing symbol is
     * morphed to a new base class Symbol.
     */

    Symbol( const string nm, const Symbol *agent )
        : unique_name( symbol_name(nm, agent) )
        , name ( nm )
        {
            auto it = symbols.find( unique_name );
            if (it == symbols.end() ) {
                // add to symbol table
                symbol_map_value_type element( unique_name, this );
                symbols.insert( element );
            }
            else {
                // redeclaration or override of default - morph to new symbol
                // delete old symbol
                delete it->second;
                // replace with new symbol
                it->second = this;
            }
        }
    
    /**
     * Morph existing symbol to a new symbol.
     * 
     * This constructor is used to morph an existing symbol table entry to
     * a new kind of symbol.  It is called by constructors for derived Symbol classes.
     * The existing Symbol is destroyed, and the symbol table entry is modified
     * to refer to the new symbol.
     *
     * @param [in,out]  sym [in,out] Original symbol to be morphed.
     */
    Symbol( Symbol*& sym )
        : unique_name( sym->unique_name )
        , name ( sym->name )
        {
            // find symbol table entry for the existing symbol
            auto it = symbols.find( unique_name );
            // delete existing symbol
            delete it->second;
            // replace with new symbol
            it->second = this;
        }

    /**
     * Get stable reference to pointer to symbol.
     * 
     * This reference is a double indirection and is stable to morphing,
     * since unordered_map guarantees that references to existing map entries
     * will not change.
     *
     * @return  Reference to pointer to this Symbol.
     */
	Symbol*& get_rpSymbol() const
    {
    	auto it = symbols.find( unique_name );
    	return it->second;
    }

    virtual ~Symbol()
    {
    }

    /**
     * Perform post-parse operations
     * 
     */

    virtual void post_parse( int pass )
    {
    }

    /**
     * Gets a short pretty name for a symbol
     * Used for diagnostics and in comments in generated code.
     *
     * @return  The short pretty name as a string.
     */

    virtual string pretty_name()
    {
        return name;
    }

    /**
     * The unique identifier for the symbol
     *
     * To create unique names for all agentvars, 
     * member names in the model source code are prefixed
     * with the agent context to create a unique name for symbol table lookup,
     * e.g. 'time' becomes 'Person::time'.
     */

    string unique_name;

    /**
     * The identifier for the symbol (possibly non-unique)
     * 
     * E.g. 'time'
     */

	string name;

    // static functions
	static bool exists( const string& unm );
	static bool exists( const string& nm, const Symbol *agent );
    static string symbol_name( const string& nm, const Symbol *agent );
	static Symbol *get_symbol( const string& unm );
	static Symbol *get_symbol( const string& nm, const Symbol *agent );
	static Symbol *find_a_symbol( const type_info& cls );
    static void default_symbols();
    static void post_parse_all();

    /**
     * Store a C++ style single-line comment for later use
     *
     * @param   cmt         The comment.
     * @param [in,out]  loc The source code location.
     */

    static void process_cxx_comment(const string& cmt, yy::location& loc);

    /**
     * Store a C style comment for later use
     *
     * @param   cmt         The comment.
     * @param [in,out]  loc The source code location.
     */

    static void process_c_comment(const string& cmt, yy::location& loc);

    /**
     * Query if 'tok' is an om outer keyword (introducing a syntactic declarative island)
     *
     * @param   tok The token.
     *
     * @return  true if an om outer keyword, false if not.
     */

    static bool is_om_outer_keyword(const token_type& tok);

    /**
    * Query if 'nm' is an om developer-supplied function
    * These are functions with special names, e.g. Start
    *
    * @param   nm  The name.
    *
    * @return  true if om developer function, false if not.
    */

    static bool is_om_developer_function(const char* nm);

    // static data members
	static symbol_map_type symbols;
    static list<AgentSymbol *> pp_agents;
    static list<TableSymbol *> pp_tables;
    static list<ParameterSymbol *> pp_parameters;
    static multimap<string, string> memfunc_bodyids;

    static unordered_set<token_type, std::hash<int> > om_outer_keywords;
    static unordered_set<string> om_developer_functions;

    // TODO comments are just dumped into placeholder collections for testing.
    // Need to retrieve by location searching, at some point, to match up with identifiers, languages, etc.
    static forward_list<string> cxx_comments;
    static forward_list<string> c_comments;
};

/**
 * ParameterSymbol.
 */
class ParameterSymbol : public Symbol
{
private:
   typedef Symbol super;

public:
    ParameterSymbol( Symbol *sym, token_type type )
        : Symbol ( sym )
        , type ( type )
    {
    }

    void post_parse( int pass )
    {
        // First perform post-parse operations at next level up in Symbol hierarchy
        super::post_parse( pass );

        switch ( pass ) {
        case 1:
            // add this parameter to the complete list of parameters
            pp_parameters.push_back( this );
            break;
        default:
            break;
        }
    }

    // members
    token_type type;

    /**
     * Numeric identifier.
     * Used for communicating with metadata API.
     */

    int pp_numeric_id;
};

class AgentMemberSymbol : public Symbol
{
private:
   typedef Symbol super;

public:
    AgentMemberSymbol( Symbol *sym, const Symbol *agent )
        : Symbol ( sym )
        , agent( agent->get_rpSymbol() )
        , pp_agent ( nullptr )
    {
    }

    AgentMemberSymbol( const string name, const Symbol *agent )
        : Symbol ( name, agent )
        , agent( agent->get_rpSymbol() )
        , pp_agent ( nullptr )
    {
    }

    void post_parse( int pass );

    /**
     * Reference to pointer to agent.
     *
     * Stable to symbol morhing during parse phase.
     */

    Symbol*& agent;

    /**
     * Direct pointer to agent.
     * 
     * Set post-parse for convenience.
     */

    AgentSymbol *pp_agent;
};

/**
 * Symbol for an Agentevent
 * 
 * An AgentEventSymbol is created when an 'event' statement is encountered
 * in the model source code.  The name of the symbol
 * is the name of the event implement function specified in the 'event' statement. 
 */
class AgentEventSymbol : public AgentMemberSymbol
{
private:
   typedef AgentMemberSymbol super;

public:

    /**
     * Constructor.
     *
     * @param [in,out]  sym     The symbol to be morphed (event implement function)
     * @param   agent           The agent.
     * @param   time_function   The symbol for the associated event time function.
     */

    AgentEventSymbol( Symbol *sym, const Symbol *agent, const Symbol *time_function )
        : AgentMemberSymbol ( sym, agent )
        , time_function ( time_function->get_rpSymbol() )
    {
    }

    CodeBlock cxx_declaration_agent_scope()
    {
        CodeBlock h;
        h += "";
        // example:         //* Mortality */
        h += "//* " + name + " */";
        // example: void MortalityEvent(); 
        h += "void " + name + "();";
        // example: Time timeMortalityEvent(); 
        h += "Time " + time_function->name + "();";
        return h;
    }

    void post_parse( int pass );

    /** The Symbol for the time function of the event.*/
    Symbol*& time_function;

    /**
     * Numeric identifier for the event.
     * The numeric identifier is the ordinal of the event name
     * in all events in the model.  If two agents have an
     * event with the same name, \a pp_event_id will be identical
     * for those two events.
     */

    int pp_event_id;
};

class AgentDataMemberSymbol : public AgentMemberSymbol
{
private:
   typedef AgentMemberSymbol super;

public:
    AgentDataMemberSymbol( Symbol *sym, const Symbol *agent, token_type type )
        : AgentMemberSymbol ( sym, agent )
        , type ( type )
    {
    }

    AgentDataMemberSymbol( const string member_name, const Symbol *agent, token_type type )
        : AgentMemberSymbol ( member_name, agent )
        , type (type)
    {
    }

    /**
     * Gets the initial value for the data member
     *
     * @return  The initial value as a string.
     */

    virtual string initial_value() const
    {
        string result;
        switch ( type ) {

        case token::TK_bool:
            // for bool, default initial value is false
            result = token_to_string( token::TK_false );
            break;

        default:
            // for all other fundamental types, default initial value is 0
            result = "( " + token_to_string( type ) + " ) 0";

        };
        return result;
    }

    virtual CodeBlock cxx_initialize_expression() const
    {
        // example:              time.initialize(0);\n
        CodeBlock c;
        c += name + ".initialize( " + initial_value() + " );";
        return c;
    }

    /**
     * Get the c++ declaration code (in agent scope) associated with the symbol.
     */

    virtual CodeBlock cxx_declaration_agent_scope()
    {
        CodeBlock h;
        h += "";
        // example:         //* time */
        h += doxygen( name );
        return h;
    }

    /**
     * Get the c++ definition code associated with the symbol.
     */

    virtual CodeBlock cxx_definition()
    {
        CodeBlock c;
        c += "";
        // example:         //* time */
        c += doxygen( name );
        return c;
    }

    virtual void post_parse( int pass );

    /**
     * C++ fundamental type.
     *
     * Recorded as a token value, e.g. int_KW.
     */

    token_type type;

};

/**
 * Internal data symbol.
 * Abstracts a member of an agent class which stores information
 * used internally by om.  These members are not accessible to
 * developer code, and have no side-effects.
 */

class AgentInternalSymbol : public AgentDataMemberSymbol
{
private:
   typedef AgentDataMemberSymbol super;

public:
    AgentInternalSymbol( Symbol *sym, const Symbol *agent, token_type type )
        : AgentDataMemberSymbol ( sym, agent, type )
    {
    }

    AgentInternalSymbol( const string member_name, const Symbol *agent, token_type type )
        : AgentDataMemberSymbol ( member_name, agent, type )
    {
    }

    void post_parse( int pass );

    virtual CodeBlock cxx_initialize_expression() const
    {
        CodeBlock c;
        // example:              om_in_DurationOfLife_alive = false;\n
        c += name + " = " + initial_value() + ";";
        return c;
    }

    /**
     * Get the c++ declaration code associated with the symbol.
     */

    virtual CodeBlock cxx_declaration_agent_scope()
    {
        // First get declaration code at next level up in the Symbol hierarchy
        CodeBlock h = super::cxx_declaration_agent_scope();
        // Now add declaration code specific to this level in the Symbol hierarchy
        h += token_to_string(type) + " " + name + ";";
        return h;
    }

    /**
     * Get the c++ definition code associated with the symbol.
     */

    virtual CodeBlock cxx_definition()
    {
        // First get definition code at next level up in the Symbol hierarchy
        CodeBlock c = super::cxx_definition();
        return c;
    }

};

/**
 * Agent callback member symbol.
 * Contains functionality to support callback functions at containing agent scope.
 */

class AgentCallbackMemberSymbol : public AgentDataMemberSymbol
{
private:
   typedef AgentDataMemberSymbol super;

public:
    AgentCallbackMemberSymbol( Symbol *sym, const Symbol *agent, token_type type )
        : AgentDataMemberSymbol ( sym, agent, type )
    {
    }

    AgentCallbackMemberSymbol( const string member_name, const Symbol *agent, token_type type )
        : AgentDataMemberSymbol ( member_name, agent, type )
    {
    }

    void post_parse( int pass );

    /**
     * Get the c++ declaration code associated with the symbol.
     */

    virtual CodeBlock cxx_declaration_agent_scope()
    {
        // First get declaration code at next level up in the Symbol hierarchy
        CodeBlock h = super::cxx_declaration_agent_scope();

        // Now add declaration code specific to this level in the Symbol hierarchy

        return h;
    }

    /**
     * Get the c++ definition code associated with the symbol.
     */

    virtual CodeBlock cxx_definition()
    {
        // First get definition code at next level up in the Symbol hierarchy
        CodeBlock c = super::cxx_definition();

        // Now add definition code specific to this level in the Symbol hierarchy

        return c;
    }

};

/**
 * Event time member symbol for an event.
 * Abstracts a member of an agent class which stores the event time
 * of an agent event.
 */

class AgentEventTimeSymbol : public AgentCallbackMemberSymbol
{
private:
   typedef AgentCallbackMemberSymbol super;

public:
    AgentEventTimeSymbol( const string member_name, const Symbol *agent, const Symbol *event )
        : AgentCallbackMemberSymbol ( member_name, agent, token::TK_Time )
        , event ( event->get_rpSymbol() )
    {
    }

    static string member_name( const Symbol *event )
    {
	    string result = "om_time_" + event->name;
        return result;
    }

    static string symbol_name( const Symbol *agent, const Symbol *event )
    {
	    string member = AgentEventTimeSymbol::member_name( event );
        string result = Symbol::symbol_name( member, agent );
        return result;
    }

    void post_parse( int pass );

    /**
     * Gets the initial value for the data member
     * Events are disabled at initialization by setting the event_time to time_infinite.
     * @return  The initial value as a string.
     */

    virtual string initial_value() const
    {
        return token_to_string( token::TK_time_infinite );
    }

    /**
     * Get the c++ declaration code associated with the symbol.
     */

    virtual CodeBlock cxx_declaration_agent_scope()
    {
        // First get declaration code at next level up in the Symbol hierarchy
        CodeBlock h = super::cxx_declaration_agent_scope();
        // Now add declaration code specific to this level in the Symbol hierarchy
        //h += token_to_string(type) + " " + name + ";";
        string implement_func = pp_event->unique_name;
        string time_func = pp_event->time_function->unique_name;
        int event_id = pp_event->pp_event_id;
        h += "Event<" + agent->name + ", "
                      + to_string( event_id ) + ", "
                      + "0, " // TODO event priority
                      + "&" + implement_func + ", "
                      + "&" + time_func + "> "
                      + name
                      + ";" "\n";
        return h;
    }

    /**
     * Get the c++ definition code associated with the symbol.
     */

    virtual CodeBlock cxx_definition()
    {
        // First get definition code at next level up in the Symbol hierarchy
        CodeBlock c = super::cxx_definition();
        return c;
    }

    /**
     * The associated event.
     */

    Symbol*& event;

    /**
     * The associated event.
     */

    AgentEventSymbol *pp_event;
};

class AgentVarSymbol : public AgentCallbackMemberSymbol
{
private:
   typedef AgentCallbackMemberSymbol super;

public:
    AgentVarSymbol( Symbol *sym, const Symbol *agent, token_type type )
        : AgentCallbackMemberSymbol ( sym, agent, type )
    {
    }

    AgentVarSymbol( const string member_name, const Symbol *agent, token_type type )
        : AgentCallbackMemberSymbol ( member_name, agent, type )
    {
    }

    void post_parse( int pass );

    /**
     * Get the c++ declaration code associated with the symbol.
     */

    virtual CodeBlock cxx_declaration_agent_scope()
    {
        // First get declaration code at next level up in the Symbol hierarchy
        CodeBlock h = super::cxx_declaration_agent_scope();

        // Now add declaration code specific to this level in the Symbol hierarchy
        // example:        void time_side_effects(Time old_value, Time new_value);
        h += side_effects_decl() + ";";
        return h;
    }

    /**
     * Get the c++ definition code associated with the symbol.
     */

    virtual CodeBlock cxx_definition()
    {
        // First get definition code at next level up in the Symbol hierarchy
        CodeBlock c = super::cxx_definition();

        // example:         Person::time_side_effects(Time old_value, Time new_value)
        c += side_effects_decl_qualified();
        c += "{";
        for ( string line : pp_side_effects ) {
            c += line;
        }
        c += "}";

        return c;
    }

    /**
     * Get name of member function which implements side-effects
     * 
     * Example:  Person::time_side_effects
     *
     * @return  The qualified function name as a string
     */

    const string side_effects_func()
    {
        return name + "_side_effects";
    }

    /**
     * Get declaration of member function which implements side-effects
     *
     * Example:  void time_side_effects(Time old_value, Time new_value)
     * Note that there is no terminating semicolon.
     */

    const string side_effects_decl()
    {
        return "void " + side_effects_func() + "(" + token_to_string( type ) + " old_value, " + token_to_string( type )+ " new_value)";
    }

    /**
     * Get qualified declaration of member function which implements side-effects
     *
     * Example:  void Person::time_side_effects(Time old_value, Time new_value)
     * Note that there is no terminating semicolon.
     */

    const string side_effects_decl_qualified()
    {
        return "void " + agent->name + "::" + side_effects_func() + "(" + token_to_string( type ) + " old_value, " + token_to_string( type )+ " new_value)";
    }

    /**
     * The lines of C++ code which implement side-effects.
     */

    list<string> pp_side_effects;

};

/**
 * BuiltinAgentVarSymbol.
 */
class BuiltinAgentVarSymbol : public AgentVarSymbol
{
private:
   typedef AgentVarSymbol super;

public:
    BuiltinAgentVarSymbol( const string member_name, const Symbol *agent, token_type type )
        : AgentVarSymbol ( member_name, agent, type )
    {
    }

    void post_parse( int pass );

    /**
     * Gets the c++ declaration for the builtin agentvar.
     */

    CodeBlock cxx_declaration_agent_scope()
    {
        // get declaration code common to all agentvars
        CodeBlock h = super::cxx_declaration_agent_scope();

        // example:         AgentVar<Time, Person, &Person::time_side_effects> time;
        h += "AgentVar<" + token_to_string( type ) + ", "
                         + agent->name + ", "
                         + "&" + agent->name + "::" + side_effects_func() + "> "
                         + name + ";";

        return h;
    }

};

/**
 * SimpleAgentVarSymbol.
 */
class SimpleAgentVarSymbol : public AgentVarSymbol
{
private:
   typedef AgentVarSymbol super;

public:
    SimpleAgentVarSymbol( Symbol *sym, const Symbol *agent, token_type type, Literal *initializer )
        : AgentVarSymbol ( sym, agent, type )
        , initializer ( initializer )
    {
    }

    ~SimpleAgentVarSymbol()
    {
		if ( initializer != nullptr ) delete initializer;
    }

    /**
     * Gets the initial value for the agentvar
     *
     * @return  The initial value as a string.
     */
    string initial_value() const
    {
        string result;
        if ( initializer != nullptr )
            result = initializer->cxx_token;
        else
            result = AgentVarSymbol::initial_value();

        return result;
    }

    void post_parse( int pass )
    {
        // First perform post-parse operations at next level up in Symbol hierarchy
        super::post_parse( pass );
    }

    /**
     * Gets the c++ declaration for the simple agentvar.
     */

    CodeBlock cxx_declaration_agent_scope()
    {
        // obtain declaration code common to all agentvars
        CodeBlock h = super::cxx_declaration_agent_scope();

        // add declaration code specific to simple agentvars

        // example:         SimpleAgentVar<bool, Person, &Person::alive_side_effects> alive;
        h += "SimpleAgentVar<" + token_to_string( type ) + ", "
                               + agent->name + ", "
                               + "&" + agent->name + "::"+ side_effects_func() + "> "
                               + name + ";";

        return h;
    }

    // members

    /** C++ literal to initialize simple agentvar at agent creation */
    Literal *initializer;

};

/**
 * DurationAgentVarSymbol.
 * 
 * Symbol for derived agentvars of the form duration().
 */

class DurationAgentVarSymbol : public AgentVarSymbol
{
private:
   typedef AgentVarSymbol super;

public:
    // constructor for 0-argument derived agentvars, e.g. duration()
    DurationAgentVarSymbol( const Symbol *agent )
        : AgentVarSymbol ( DurationAgentVarSymbol::member_name(), agent, token::TK_Time )
    {
    }

    static string member_name()
    {
	    string result = "om_" + token_to_string(token::TK_duration);
        return result;
    }

    static string symbol_name( const Symbol *agent )
    {
	    string member = DurationAgentVarSymbol::member_name();
        string result = Symbol::symbol_name( member, agent );
        return result;
    }

    /**
     * Get a symbol for a derived agentvar of form duration()
     *
     * @param   agent   The agent.
     *
     * @return  The symbol.
     */

    static Symbol * get_symbol( const Symbol *agent )
    {
        Symbol *sym = nullptr;
        string nm = DurationAgentVarSymbol::symbol_name( agent );
	    auto it = symbols.find( nm );
	    if (it != symbols.end() )
            sym = it->second;
        else
            sym = new DurationAgentVarSymbol( agent );

        return sym;
    }


    void post_parse( int pass );

    /**
     * Generates the c++ declaration for the derived agentvar.
     */

    CodeBlock cxx_declaration_agent_scope()
    {
        // get declaration code common to all agentvars
        CodeBlock h = super::cxx_declaration_agent_scope();

        // add declaration code specific to derived agentvars

        // example:         DurationAgentVar<Time, Person, &duration_offset, &Person::duration_side_effects, nullptr> om_duration
        h += "DurationAgentVar<" + token_to_string( type ) + ", "
                                 + agent->name + ", "
                                 + "&" + agent->name + "::"+ side_effects_func() + ", "
                                 + "nullptr> "
                                 + name + ";" "\n";

        return h;
    }

};

/**
 * ConditionedDurationAgentVarSymbol
 * 
 * Symbol for derived agentvars of the form duration( agentvar, value).
 * E.g. duration(happy, true)
 */

class ConditionedDurationAgentVarSymbol : public AgentVarSymbol
{
private:
   typedef AgentVarSymbol super;

public:
    ConditionedDurationAgentVarSymbol( const Symbol *agent, const Symbol *observed, const Literal *constant )
        : AgentVarSymbol ( ConditionedDurationAgentVarSymbol::member_name(observed, constant),
                            agent,
                            token::TK_Time 
                        )
        , observed ( observed->get_rpSymbol() )
        , constant ( constant )
        , pp_observed ( nullptr )
    {
    }

    /**
     * The member name for a specific symbol of this kind
     *
     * @param   observed    The observed agentvar
     * @param   constant    The constant with which the agentvar is compared to condition the duration
     *
     * @return  The member namne as a string
     */

    static string member_name( const Symbol *observed, const Literal *constant )
    {
    	string result = "om_" + token_to_string(token::TK_duration) + "_" + observed->name + "_" + constant->cxx_token;
        return result;
    }

    static string symbol_name( const Symbol* agent, const Symbol* observed, const Literal* constant )
    {
	    string member = ConditionedDurationAgentVarSymbol::member_name( observed, constant );
        string result = Symbol::symbol_name( member, agent );
        return result;
    }

    static Symbol * get_symbol( const Symbol* agent, const Symbol* observed, const Literal* constant )
    {
        Symbol *sym = nullptr;
	    string nm = ConditionedDurationAgentVarSymbol::symbol_name( agent, observed, constant);
	    auto it = symbols.find( nm );
	    if (it != symbols.end() )
            sym = it->second;
        else
            sym = new ConditionedDurationAgentVarSymbol( agent, observed, constant );

        return sym;
    }

    void post_parse( int pass );

    /**
     * Get name of member function which implements the condition expression
     * 
     * Example:  om_duration_happy_true_condition.
     *
     * @return  .
     */

    const string condition_func()
    {
        return name + "_condition";
    }

    /**
     * Get declaration of static member function which implements the condition expression
     *
     * Example:  bool om_duration_happy_true_condition()
     */

    const string condition_decl()
    {
        return "bool " + condition_func() + "()";
    }

    /**
     * Get declaration of static member function which implements the condition expression
     *
     * Example:  bool Person::om_duration_happy_true_condition()
     */

    const string condition_decl_qualified()
    {
        return "bool " + agent->name + "::" + condition_func() + "()";
    }

    /**
     * Generates the c++ declaration for the derived agentvar.
     */

    CodeBlock cxx_declaration_agent_scope()
    {
        // get declaration code common to all agentvars
        CodeBlock h = super::cxx_declaration_agent_scope();

        // add declaration code specific to this kind of derived agentvar
        // 
        // example:        bool om_duration_alive_true_condition();
        h += condition_decl() + ";";

        // example:         DurationAgentVar<Time, Person, &om_duration_alive_true_offset, &Person::om_duration_alive_true_side_effects, &Person::om_duration_alive_true_condition> om_duration
        h += "DurationAgentVar<" + token_to_string( type ) + ", "
                                 + agent->name + ", "
                                 + "&" + agent->name + "::"+ side_effects_func() + ", "
                                 + "&"+ agent->name + "::" + condition_func() + "> "
                                 + name + ";";

        return h;
    }

    /**
     * Generates the c++ definition for the derived agentvar.
     */

    CodeBlock cxx_definition()
    {
        // start with definition code from next higher level in symbol hierarchy
        CodeBlock c = super::cxx_definition();

        // add definition code specific to this kind of derived agentvar
        // example:        bool Person::om_duration_alive_true_condition()
        c += condition_decl_qualified();
        c += "{";
        // example:             return ( alive == true );
        c += "return (" + observed->name + " == " + constant->cxx_token + " );";
        c += "}";

        return c;
    }
   
    /**
     * agentvar observed (reference to pointer)
     *
     * Stable to symbol morphing during parse phase.
     */

    Symbol*& observed;

    /**
     * agentvar observed (pointer)
     * 
     * Only valid after post-parse phase 1.
     */

    const Literal *constant;
    AgentVarSymbol *pp_observed;
};

/**
 * AgentSymbol.
 */
class AgentSymbol : public Symbol
{
private:
   typedef Symbol super;

public:

    /**
     * Constructor.
     *
     * @param [in,out]  sym The symbol to be morphed.
     */
    AgentSymbol( Symbol *sym )
        : Symbol ( sym )
        , pp_time ( nullptr )
        , pp_age ( nullptr )
        , pp_events ( nullptr )
    {
        // Create builtin agentvars for this agent: time, age, events, agent_id
        if ( ! exists( "time", this ) )
            new BuiltinAgentVarSymbol( "time", this, token::TK_Time );
        if ( ! exists( "age", this ) )
            new BuiltinAgentVarSymbol( "age", this, token::TK_Time );
        if ( ! exists( "events", this ) )
            new BuiltinAgentVarSymbol( "events", this, token::TK_counter );
        if ( ! exists( "agent_id", this ) )
            new BuiltinAgentVarSymbol( "agent_id", this, token::TK_int );

        // TODO: Remove test - Create internal data members for this agent: allow_assignment
        if ( ! exists( "allow_assignment", this ) )
            new AgentInternalSymbol( "allow_assignment", this, token::TK_bool );
    }

    void post_parse( int pass )
    {
        // First perform post-parse operations at next level up in Symbol hierarchy
        super::post_parse( pass );

        switch ( pass ) {
        case 1:
            // Add this agent to the complete list of agents.
            pp_agents.push_back( this );
            break;
        default:
            break;
        }

        // assign pointers to builtin members for use post-parse
        pp_time = dynamic_cast<BuiltinAgentVarSymbol *>(get_symbol( "time", this ));
        pp_age = dynamic_cast<BuiltinAgentVarSymbol *>(get_symbol( "age", this ));
        pp_events = dynamic_cast<BuiltinAgentVarSymbol *>(get_symbol( "events", this ));
    }

    BuiltinAgentVarSymbol *pp_time;
    BuiltinAgentVarSymbol *pp_age;
    BuiltinAgentVarSymbol *pp_events;

    /**
     * The agentvars of this agent
     *
     *  Populated after parsing is complete.
     */

    list<AgentDataMemberSymbol *> pp_agent_data_members;

    /**
     * The agentvars of this agent
     *
     *  Populated after parsing is complete.
     */

    list<AgentVarSymbol *> pp_agentvars;

    /**
     * The agentevents of this agent
     *
     *  Populated after parsing is complete.
     */

    list<AgentEventSymbol *> pp_agentevents;

    /**
     * The AgentEventTime symbols of this agent
     *
     *  Populated after parsing is complete.
     */

    list<AgentEventTimeSymbol *> pp_agent_event_times;

    /**
     * The agentfuncs of this agent
     *
     *  Populated after parsing is complete.
     */

    list<AgentFuncSymbol *> pp_agentfuncs;

    /**
     * The tables of this agent
     *
     *  Populated after parsing is complete.
     */

    list<TableSymbol *> pp_agent_tables;
};

/**
 * TableExpressionSymbol.
 */
class TableExpressionSymbol : public Symbol
{
private:
   typedef Symbol super;

public:
    TableExpressionSymbol( Symbol *table, ExprForTable *root, int index )
        : Symbol ( symbol_name( table, index) )
        , root ( root )
        , index ( index )
        , table ( table->get_rpSymbol() )
        , pp_table ( nullptr )
    {
    }

    // Construct symbol name for the table expression symbol.
    // Example: BasicDemography.Expr0
    static string symbol_name( const Symbol* table, int index )
    {
        assert( table );
	    return table->name + ".Expr" + to_string( index );
    }


    void post_parse( int pass );

    void post_parse_traverse( ExprForTable *node );

    enum expression_style {
        cxx,
        sql
    };
    string get_expression( const ExprForTable *node, expression_style style );

    /**
     * Root of the expression tree
     */

    ExprForTable *root;

    /**
     * Zero-based index of the expression in the expression table dimension 
     */

    int index;

    /**
     * The table containing this expression (reference to pointer)
     *
     * Stable to symbol morphing during parse phase.
     */

    Symbol*& table;

    /**
     * The table containing this expression (pointer)
     *
     * Only valid after post-parse phase 1.
     */

    TableSymbol* pp_table;
};

/**
 * TableAccumulatorSymbol.
 */
class TableAccumulatorSymbol : public Symbol
{
private:
   typedef Symbol super;

public:
    TableAccumulatorSymbol( Symbol *table, token_type accumulator, token_type increment, Symbol *agentvar, Symbol *analysis_agentvar, int index )
        : Symbol ( symbol_name( table, accumulator, increment, agentvar) )
        , table ( table->get_rpSymbol() )
        , accumulator ( accumulator )
        , increment ( increment )
        , agentvar ( agentvar->get_rpSymbol() )
        , analysis_agentvar ( analysis_agentvar->get_rpSymbol() )
        , index ( index )
        , pp_table ( nullptr )
        , pp_agentvar ( nullptr )
        , pp_analysis_agentvar ( nullptr )
    {
        // grammar guarantee
        assert(    accumulator == token::TK_sum
                || accumulator == token::TK_min
                || accumulator == token::TK_max
              );

        // grammar guarantee
        assert(    increment == token::TK_delta
                || increment == token::TK_delta2
                || increment == token::TK_nz_delta
                || increment == token::TK_value_in
                || increment == token::TK_value_in2
                || increment == token::TK_nz_value_in
                || increment == token::TK_value_out
                || increment == token::TK_value_out2
                || increment == token::TK_nz_value_out
              );
    }

    /**
     * Get the unique name for this TableAccumulatorSymbol.
     *
     * @param   table       The table.
     * @param   accumulator The accumulator, e.g. token::TK_sum
     * @param   increment   The increment, e.g. token::TK_delta
     * @param   agentvar    The agentvar.
     *
     * @return  The name, e.g. om_ta_DurationOfLife_sum_delta_om_duration.
     */

    static string symbol_name(const Symbol *table, token_type accumulator, token_type increment, const Symbol *agentvar)
    {
        string result;
        result = "om_ta_" + table->name + "_" + token_to_string( accumulator ) + "_" + token_to_string( increment ) + "_" + agentvar->name;
        return result;
    }

    /**
     * Check for existence of symbol with this unique name.
     *
     * @param   table       The table.
     * @param   accumulator The accumulator, e.g. token::TK_sum.
     * @param   increment   The increment, e.g. token::TK_delta.
     * @param   agentvar    The agentvar.
     *
     * @return  true if found, else false.
     */

    static bool exists( const Symbol *table, token_type accumulator, token_type increment, const Symbol *agentvar )
    {
        string unm = symbol_name( table, accumulator, increment, agentvar );
        return symbols.count( unm ) == 0 ? false : true;
    }

    void post_parse( int pass );

    virtual string pretty_name()
    {
        // example:     accumulator 0: sum(delta(duration))
        string result = " accumulator " + to_string(index) + ": " + token_to_string( accumulator ) + "(" + token_to_string( increment ) + "(" + agentvar->pretty_name() + "))";
        return result;
    }

    /**
     * The table containing this accumulator (reference to pointer)
     *
     * Stable to symbol morphing during parse phase.
     */

    Symbol*& table;

    token_type accumulator;

    token_type increment;

    /**
     * The agentvar being accumulated (reference to pointer)
     *
     * Stable to symbol morphing during parse phase.
     */

    Symbol*& agentvar;

    /**
     * The analysis agentvar being accumulated (reference to pointer)
     *
     * Stable to symbol morphing during parse phase.
     */

    Symbol*& analysis_agentvar;

    /**
     * Zero-based index of the accumulator within this table.
     */

    int index;

    /**
     * The table containing this accumulator (pointer)
     *
     * Only valid after post-parse phase 1.
     */

    TableSymbol* pp_table;

    /**
     * The agentvar being accumulated (pointer)
     *
     * Only valid after post-parse phase 1.
     */

    AgentVarSymbol* pp_agentvar;

    /**
     * The analysis agentvar being accumulated (pointer)
     *
     * Only valid after post-parse phase 1.
     */

    TableAnalysisAgentVarSymbol* pp_analysis_agentvar;
};

/**
 * TableAnalysisAgentVarSymbol.
 * Represents an agentvar used in the analysis dimension
 * (also known as the expression dimension) of a table.
 * An agentvar can be used multiple times in the analysis dimension of a table.
 * If so, it is represented by a single instance of this class.
 * This class handles the creation of an associated agentvar which holds the
 * 'in' value of the agentvar when a table increment is started.
 */

class TableAnalysisAgentVarSymbol : public Symbol
{
private:
   typedef Symbol super;

public:
    TableAnalysisAgentVarSymbol( Symbol *table, Symbol *agentvar, int index )
        : Symbol ( symbol_name( table, agentvar) )
        , table ( table->get_rpSymbol() )
        , agentvar ( agentvar->get_rpSymbol() )
        , index ( index )
        , need_value_in ( false )
        , pp_table ( nullptr )
        , pp_agentvar ( nullptr )
    {
    }

    /**
     * Get the unique name for a specified symbol of this kind.
     *
     * @param   table       The table.
     * @param   agentvar    The agentvar used in the analysis dimension
     *
     * @return  The symbol name as a string.
     *          Example: om_taav_DurationOfLife_alive.
     */

    static string symbol_name(const Symbol *table, const Symbol *agentvar)
    {
        string result;
        result = "om_taav_" + table->name + "_" + agentvar->name;
        return result;
    }

    /**
     * Check for existence of a specific table analysis agentvar.
     *
     * @param   table       The table.
     * @param   agentvar    The agentvar.
     *
     * @return  true if found, else false.
     */

    static bool exists( const Symbol *table, const Symbol *agentvar )
    {
        string unm = symbol_name( table, agentvar );
        return symbols.count( unm ) == 0 ? false : true;
    }

    void post_parse( int pass );

    /**
     * Gets the agentvar name for the 'in' value.
     * This is the name of the data member which holds the
     * "in" value of the agentvar.  It holds the value of the agentvar
     * at the start of an increment, and is used to compute 'delta', etc.
     * when the increment is finalized and passed to an accumulator.
     *
     * @return  The name as a string.
     */

    string in_agentvar_name() const
    {
        string result;
        result = "om_in_" + table->name + "_" + agentvar->name;
        return result;
    }

    CodeBlock cxx_prepare_increment() const;

    /**
     * The table using the agentvar
     *
     * Stored as a reference to a pointer, which is stable to symbol morphing during the parse phase.
     */

    Symbol*& table;

    /**
     * The agentvar used in the analysis dimension of the table
     *
     * Stored as a reference to a pointer, which is stable to symbol morphing during the parse phase.
     */

    Symbol*& agentvar;

    /**
     * Zero-based index of the analysis agentvar
     * in the list of all analysis agentvars used in the table.
     */

    const int index;

    /**
     * true means that generated code needs the member storing the "value_in" of the agentvar
     *
     * Only valid after post-parse phase 1.
     */

    bool need_value_in;

    /**
     * The table using the agentvar
     *
     * Stored as a pointer, which is only valid after post-parse phase 1.
     */

    TableSymbol* pp_table;

    /**
     * The agentvar used in the analysis dimension of the table.
     *
     * Stored as a pointer, which is only valid after post-parse phase 1.
     */

    AgentVarSymbol* pp_agentvar;

};

/**
 * ModelSymbol.
 */
class ModelSymbol : public Symbol
{
private:
   typedef Symbol super;

public:
    ModelSymbol( string nm )
        : Symbol (nm)
    {
    }
};

/**
 * TableSymbol.
 */
class TableSymbol : public Symbol
{
private:
   typedef Symbol super;

public:
    TableSymbol( Symbol *sym, const Symbol *agent )
        : Symbol ( sym )
        , agent( agent->get_rpSymbol() )
    {
    }

    void post_parse( int pass )
    {
        // First perform post-parse operations at next level up in Symbol hierarchy
        super::post_parse( pass );

        switch ( pass ) {
        case 1:
            // assign direct pointer to agent for use post-parse
            pp_agent = dynamic_cast<AgentSymbol *> (agent);
            // add this table to the complete list of tables
            pp_tables.push_back( this );
            // Add this table to the agent's list of tables
            pp_agent->pp_agent_tables.push_back( this );
            break;
        default:
            break;
        }
    }

    /**
     * Get name of member function which handles increments used in the table
     * 
     * Example:  DurationOfLife_do_increments
     *
     * @return  The qualified function name as a string
     */

    const string do_increments_func()
    {
        // Eg. DurationOfLife_do_increments
        return name + "_do_increments";
    }

    /**
     * Get declaration of member function which handles increments used in the table
     *
     * Example:  void Person::DurationOfLife_do_increments(bool prepare = true, bool process = true);
     */

    const string do_increments_decl()
    {
        // E.g. void DurationOfLife_do_increments(bool prepare = true, bool process = true)
        return "void " + do_increments_func() + "( bool prepare = true, bool process = true );";
    }

    /**
     * Get definition of member function which handles increments used in the table
     *
     * Example:  void Person::DurationOfLife_do_increments(bool prepare = true, bool process = true)
     */

    const string do_increments_defn()
    {
        // E.g. void Person::DurationOfLife_do_increments(bool prepare = true, bool process = true)
        return "void " + agent->name + "::" + do_increments_func() + "( bool prepare, bool process )";
    }

    /**
     * Get the c++ declaration code (in agent scope) associated with the symbol.
     */

    virtual CodeBlock cxx_declaration_agent_scope()
    {
        CodeBlock h;
        h += "";
        // example:         // DurationOfLife
        h += "// " + name;
        // example:        void DurationOfLife_do_increments( bool prepare = 0, bool process = 0 );
        h += do_increments_decl();
        return h;
    }

    /**
     * Get the c++ definition code associated with the symbol.
     */

    virtual CodeBlock cxx_definition()
    {
        CodeBlock c;

        c += "";

        // example:         // DurationOfLife
        c += "// " + name;

        // example:        void DurationOfLife_do_increments( bool prepare = 0, bool process = 0 )
        c += do_increments_defn();
        c += "{";
        c += "if ( process ) {";
        for ( auto acc : pp_accumulators ) {
            // name of agentvar
            string agentvar_name = acc->agentvar->name;
            // name of 'in' for agentvar
            string in_agentvar_name = acc->pp_analysis_agentvar->in_agentvar_name();
            // index of accumulator as string
            string accumulator_index = to_string(acc->index);
            // expression for the accumulator as string
            string accumulator_expr = "the" + name + ".accumulators[" + accumulator_index + "][0]"; 

            // expression evaluating to value of increment
            string increment_expr;
            switch ( acc->increment ) {
            case token::TK_value_in:
                increment_expr = in_agentvar_name;
                break;
            case token::TK_value_out:
                increment_expr = agentvar_name;
                break;
            case token::TK_delta:
                increment_expr = "( " + agentvar_name + " - " + in_agentvar_name + " )";
                break;
            // TODO - all other increment operators
            default:
                assert(0); // grammar guarantee
            }

            c += "{";
            c += "// " + acc->pretty_name();
            c += "double dIncrement = " + increment_expr + ";";
            switch ( acc->accumulator ) {
            case token::TK_sum:
                c += accumulator_expr + " += dIncrement;";
                break;
            case token::TK_min:
                c += "double dAccumulator = " + accumulator_expr + ";";
                c += "if ( dIncrement < dAccumulator ) " + accumulator_expr + " = dIncrement;";
                break;
            case token::TK_max:
                c += "double dAccumulator = " + accumulator_expr + ";";
                c += "if ( dIncrement > dAccumulator ) " + accumulator_expr + " = dIncrement;";
                break;
            default:
                assert(0); // grammar guarantee
            }
            c += "}";
        }
        c += "}";
        c += "if ( prepare ) {";
        for ( auto table_agentvar : pp_table_agentvars ) {
            if ( table_agentvar->need_value_in )
                c += table_agentvar->cxx_prepare_increment();
        }
        c += "}";
        c += "}";

        return c;
    }

    /**
     * Reference to pointer to agent.
     *
     * Stable to symbol morhing during parse phase.
     */

    Symbol*& agent;

    /**
     * Direct pointer to agent.
     * 
     * Set post-parse for convenience.
     */

    AgentSymbol *pp_agent;

    /**
     * The expressions in the table
     */

    list<TableExpressionSymbol *> pp_expressions;

    /**
     * The agentvars used in all expressions in the table
     */

    list<TableAnalysisAgentVarSymbol *> pp_table_agentvars;

    /**
     * The accumulators used in all expressions in the table
     */

    list<TableAccumulatorSymbol *> pp_accumulators;

    /**
     * Numeric identifier.
     * Used for communicating with metadata API.
     */

    int pp_numeric_id;
};

/**
 * TypeDeclSymbol.
 * 
 * The symbol table is initialized with one TypeDeclSymbol
 * corresponding to each kind of type declaration statement in openM++, e.g. time_type, real_type.
 * The symbol type (first argument of constructor) is the token associated with the corresponding type name,
 * e.g. TK_Time for the time_type statement, TK_index to the index_type statement.  
 * The one exception is the model_type statement which has an associated TypeDeclSymbol with 'type' TK_model_type.
 * These default symbols are morphed if the corresponding statement is found
 * in the model source code.
 */
class TypeDeclSymbol : public Symbol
{
private:
   typedef Symbol super;

public:

    /**
     * Constructor.
     *
     * @param   type    The token for the keyword of the type, e.g. token::KW_Time
     * @param   value   The token for the associated type, e.g. token::KW_double
     */
    TypeDeclSymbol( token_type type, token_type value )
        : Symbol ( token_to_string(type) )
        , value ( value )
    {
    }

    /** The C++ type of the given openM++ type */
    token_type value;
};

/**
 * Symbol for a function member of an agent.
 * 
 * The symbol table contains an AgentFuncSymbol for each agent function,
 * apart from event implement functions, which are represented by AgentEventSymbol's.
 */
class AgentFuncSymbol : public AgentMemberSymbol
{
private:
   typedef AgentMemberSymbol super;

public:

    /**
     * Constructor.
     *
     * @param [in,out]  sym The symbol to be morphed.
     */
    AgentFuncSymbol( Symbol *sym, const Symbol *agent )
        : AgentMemberSymbol ( sym, agent )
    {
    }

    void post_parse( int pass )
    {
        // First perform post-parse operations at next level up in Symbol hierarchy
        super::post_parse( pass );

        switch ( pass ) {
        case 1:
            // Add this agentfunc to the agent's list of agentfuncs
            pp_agent->pp_agentfuncs.push_back( this );
            break;
        default:
            break;
        }
    }

};

/**
 * Symbol for a source code module.
 * 
 * The name of a ModuleSymbol is the name of the model source code file,
 * including the .mpp or .ompp file extension.
 */
class ModuleSymbol : public Symbol
{
private:
   typedef Symbol super;

public:

    /**
     * Constructor.
     * 
     * Unlike most symbols, a ModuleSymbol is created outside of
     * the parser and scanner.
     *
     * @param   nm  The name of the module.
     *              
     */
    ModuleSymbol( string nm )
        : Symbol (nm)
    {
    }
};

// remove GNU macro defines for major and minor
#ifdef major
    #undef major
#endif
#ifdef minor
    #undef minor
#endif

/**
 * Symbol containing model version information
 * 
 * There is only one VersionSymbol in the symbol table, with name 'version'.
 * A default VersionSymbol with value 1,0,0,0 is created at initialization.
 * This default VersionSymbol will be morphed if a 'version' statement
 * is encountered in the model source code.
 */
class VersionSymbol : public Symbol
{
private:
   typedef Symbol super;

public:

    /**
     * Constructor.
     *
     * @param[in]   i_major           Major model version.
     * @param[in]   i_minor           Minor model version.
     * @param[in]   i_sub_minor       Sub-minor model version.
     * @param[in]   i_sub_sub_minor   Sbu-sub minor version.
     */
    VersionSymbol( int i_major, int i_minor, int i_sub_minor, int i_sub_sub_minor )
        : Symbol ( token_to_string( token::TK_version ) )
        , major ( i_major )
        , minor ( i_minor )
        , sub_minor ( i_sub_minor )
        , sub_sub_minor ( i_sub_sub_minor )
    {
    }

    /** major model version (position 1) */
    int major;

    /** minor model version (position 2) */
    int minor;

    /** sub-minor model version (position 3) */
    int sub_minor;

    /** sub-sub-minor model version (position 4) */
    int sub_sub_minor;
};

/**
 * Symbol for a supported interface language in the model
 * 
 * Each of the comma-separated language codes in the openM++ 'languages' statement
 * is a LanguageSymbol in the symbol table.
 */
class LanguageSymbol : public Symbol
{
private:
   typedef Symbol super;

public:

    /**
     * Constructor.
     *
     * @param [in,out]  sym Symbol to be morphed
     * @param   deflang     true if this is the model's default language.
     */
    LanguageSymbol( Symbol *sym, bool deflang )
        : Symbol ( sym )
        , default_language ( deflang )
    {
    }

    /** True if this is the model's default language */
    bool default_language;
};

