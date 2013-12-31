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
#include "token.h"
#include "location.hh"
#include "Literal.h"
#include "CodeBlock.h"

class AgentSymbol;
class ParameterSymbol;
class TableSymbol;

using namespace std;

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
//class AgentDataMemberSymbol;
//class AgentEventSymbol;
//class AgentEventTimeSymbol;
//class AgentFuncSymbol;
//class AgentInternalSymbol;
//class AgentMemberSymbol;
//class AgentSymbol;
//class AgentVarSymbol;
//class BuiltinAgentVarSymbol;
//class ConditionedDurationAgentVarSymbol;
//class DurationAgentVarSymbol;
//class ExprForTable;
//class LanguageSymbol;
//class ModelSymbol;
//class ModuleSymbol;
//class ParameterSymbol;
//class SimpleAgentVarSymbol;
//class Symbol;
//class TableSymbol;
//class TableAccumulatorSymbol;
//class TableAnalysisAgentVarSymbol;
//class TableExpressionSymbol;
//class TableSymbol;
//class TypeDeclSymbol;
//class VersionSymbol;





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
     * 
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
    
    /**
    * The symbol table
    */

    static symbol_map_type symbols;

    /**
    * The agents in the model
    *
    * Populated after parsing is complete.
    */

    static list<AgentSymbol *> pp_agents;

    /**
    * The tables in the model
    *
    * Populated after parsing is complete.
    */

    static list<TableSymbol *> pp_tables;

    /**
     * The parameters in the model
     * 
     * Populated after parsing is complete.
     */

    static list<ParameterSymbol *> pp_parameters;

    /**
     * Map of member function qualified names to all identifiers used in the function body. Example
     * entry might be "Person::Mortality" ==> "alive".
     */

    static multimap<string, string> memfunc_bodyids;

    static unordered_set<token_type, std::hash<int> > om_outer_keywords;
    static unordered_set<string> om_developer_functions;

    // TODO comments are just dumped into placeholder collections for testing.
    // Need to retrieve by location searching, at some point, to match up with identifiers, languages, etc.
    static forward_list<string> cxx_comments;
    static forward_list<string> c_comments;
};



