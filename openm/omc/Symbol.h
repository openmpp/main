/**
 * @file    Symbol.h
 * Declarations for the Symbol class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <forward_list>
#include <map>
#include "location.hh"
#include "parser.hpp"

class CodeBlock;
class TypeSymbol;
class LanguageSymbol;
class AgentSymbol;
class ParameterSymbol;
class TableSymbol;
namespace openm {
    struct MetaModelHolder;
}

using namespace std;


/**
 * Defines an alias used to reference bison-generated token values.
 * 
 * For example token::TK_agent is the value of the bison-generated token associated with the
 * openM++ keyword 'agent'.
 */

typedef yy::parser::token token;


/**
 * Defines an alias for the bison-generated enum holding token values.
 * 
 * Used to declare members, function arguments, and return values for token values.
 */

typedef yy::parser::token_type token_type;


/**
 * Defines an alias representing the type of the symbol table.
 * 
 * The symbol table maps strings to pointers to symbols. The use of pointers to Symbols allows
 * polymorphic symbols in the symbol table. It also allows a given symbol to be 'morphed' to a
 * different kind of symbol during parsing by replacing the Symbol pointer by a pointer to a new
 * instance of a derived class of Symbol.  Morphing requires that any persistent use of a Symbol
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


/**
 * A Symbol is a named syntactic entity in openM++.
 * 
 * When the scanner encounters a name in the model source code which is neither a C++ keyword
 * nor an openM++ keyword, it creates an associated Symbol object, places it in the symbol table,
 * and returns it to the parser.  The parser may subsequently morph this original symbol table
 * entry to a derived class of Symbol.
 * 
 * The Symbol class also contains static members which provide all functionality associated with
 * the symbol table.
 */

class Symbol {
public:

    /**
     * Constructor for new Symbol.
     * 
     * If the symbol table does not contain an entry with unique name @a unm a new symbol table
     * entry is created with this unique name.
     * 
     * This constructor sets the name equal to the unique name, and is used for symbols outside
     * agent scope, e.g. classifications.
     * 
     * If the symbol table does contain an entry with unique name @a unm the existing symbol is
     * morphed to a new base class Symbol. This is useful for replacing a default symbol by a symbol
     * specified in the model source code. For example, a default @a TypedefTypeSymbol with name 'Time'
     * and type 'double' is inserted into the symbol table at initialization.  If a time_type
     * statement is encountered in model code, this symbol is replaced by a new symbol with the
     * developer-specified type used for time.
     *
     * @param unm The unique name for the symbol.
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
     * This constructor creates a Symbol with unique name based on
     * @a nm in the context of @a agent, e.g. "Person::time"
     * 
     * If the symbol table does not contain a corresponding entry a new symbol table entry is
     * created with this unique name.
     * 
     * If the symbol table does contain a corresponding entry the existing symbol is morphed to a
     * new base class Symbol.
     *
     * @param nm    The name.
     * @param agent The agent qualifying the name.
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
     * This constructor is used to morph an existing symbol table entry to a new kind of symbol.  It
     * is called by constructors for derived Symbol classes. The existing Symbol is destroyed, and
     * the symbol table entry is modified to refer to the new symbol.
     *
     * @param [in,out] sym [in,out] Original symbol to be morphed.
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
     * This reference is a double indirection and is stable to morphing, since unordered_map
     * guarantees that references to existing map entries will not change.
     *
     * @return Reference to pointer to this Symbol.
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
     * Values that represent a post-parse pass of all symbols
     */

    enum post_parse_pass {
        ///< post-parse pass to create missing symbols
        eCreateMissingSymbols,
        ///< post-parse pass to populate pp_ collections
        ePopulateCollections,
        ///< post-parse pass to populate dependencies
        ePopulateDependencies
    };

    /**
     * Perform post-parse operations.
     * 
     * Post-parse operations create derived information based on information obtained during the
     * parse phase. This information is stored in members of the Symbol class hierarchy which by
     * convention have a 'pp_' prefix in their name. For example, the \ref pp_agentvars collection
     * of the \ref AgentSymbol class contains a list of all agentvars of the agent after post-parse
     * processing.  These pp_ members are used subsequently in the code-generation phase.
     * 
     * When post_parse is called on an object in the \ref Symbol hierarchy, the call is first passed
     * upwards through the inheritance hierarchy so that post-parse operations are performed at all
     * hierarchical levels, and at higher levels before lower levels (for each @a pass).
     * 
     * Post-parse operations occur in a series of sequential passes, with the results of each pass
     * usable by subsequent passes.  \ref post_parse is called on all symbols through the function
     * \ref Symbol::post_parse_all.
     *
     * @param pass The pass number.
     */

    virtual void post_parse(int pass);


    /**
     * Get the c++ declaration code (in agent scope) associated with the symbol.
     * 
     * The code fragment is valid within an agent class declaration.
     * 
     * When this function is called on an object in the \ref Symbol hierarchy, the call is first
     * passed upwards through the inheritance hierarchy so that operations are performed at all
     * hierarchical levels, and at higher levels before lower levels.
     *
     * @return A CodeBlock.
     */

    virtual CodeBlock cxx_declaration_agent();


    /**
     * Get the c++ definition code (for agent members) associated with the symbol.
     * 
     * The code fragment is qualified by the agent class.
     *
     * When this function is called on an object in the \ref Symbol hierarchy, the call is first
     * passed upwards through the inheritance hierarchy so that operations are performed at all
     * hierarchical levels, and at higher levels before lower levels.
     *
     * @return A CodeBlock.
     */

    virtual CodeBlock cxx_definition_agent();

    
    /**
    * Get the c++ declaration code associated with the symbol.
    *
    * The code fragment is valid outside all class declarations.
    *
    * When this function is called on an object in the \ref Symbol hierarchy, the call is first
    * passed upwards through the inheritance hierarchy so that operations are performed at all
    * hierarchical levels, and at higher levels before lower levels.
    *
    * @return A CodeBlock.
    */

    virtual CodeBlock cxx_declaration_global();


    /**
    * Get the c++ definition code associated with the symbol.
    *
    * The code fragment is not qualified by any agent class.
    *
    * When this function is called on an object in the \ref Symbol hierarchy, the call is first
    * passed upwards through the inheritance hierarchy so that operations are performed at all
    * hierarchical levels, and at higher levels before lower levels.
    *
    * @return A CodeBlock.
    */

    virtual CodeBlock cxx_definition_global();


    /**
    * Populate metadata associated with the symbol.
    *
    *
    * When this function is called on an object in the \ref Symbol hierarchy, the call is first
    * passed upwards through the inheritance hierarchy so that operations are performed at all
    * hierarchical levels, and at higher levels before lower levels.
    */

    virtual void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * Gets a short pretty name for a symbol
     * 
     * Used for diagnostics and in comments in generated code.
     *
     * @return The short pretty name as a string.
     */

    virtual string pretty_name()
    {
        return name;
    }


    /**
     * The unique identifier for the symbol
     * 
     * To create unique names for all agentvars, member names in the model source code are prefixed
     * with the agent context to create a unique name for symbol table lookup,
     * e.g. 'time' becomes 'Person::time'.
     */

    string unique_name;


    /**
     * The identifier for the symbol (possibly non-unique)
     * 
     * E.g. 'time'.
     */

	string name;

    // static functions
	static bool exists( const string& unm );
	static bool exists( const string& nm, const Symbol *agent );
    static string symbol_name( const string& nm, const Symbol *agent );
	static Symbol *get_symbol( const string& unm );
	static Symbol *get_symbol( const string& nm, const Symbol *agent );
	static Symbol *find_a_symbol( const type_info& cls );


    /**
     * Get the string corresponding to a token.
     *
     * @param e The token value, e.g. TK_agent.
     *
     * @return The string representation of the token, e.g. "agent".
     */

    static const string token_to_string(const token_type& e);


    /**
     * Get the token corresponding to a string.
     *
     * @param s A string with an associated token, e.g. "agent".
     *
     * @return The token associated with the string, e.g. token::TK:agent. If the string is not a
     *         token, the special value token::TK_error is returned.
     */

    static const token_type string_to_token(const char * s);


    /**
     * Extract accumulator from Modgen cumulation operator.
     *
     * @param e The Modgen cumulation operator, e.g. token::TK_max_value_in.
     *
     * @return The associated accumulator, e.g. token::TK_max.
     */

    static const token_type modgen_cumulation_operator_to_acc(const token_type& e);


    /**
     * Extract increment from Modgen cumulation operator.
     *
     * @param e The Modgen cumulation operator, e.g. token::TK_max_value_in.
     *
     * @return The associated increment, e.g. token::TK_value_in.
     */

    static const token_type modgen_cumulation_operator_to_incr(const token_type& e);


    /**
     * Default symbols in symbol table
     * 
     * These are symbols which are added to the symbol table when it is created.  They have default
     * properties which can be overridden by the om developer.  An example is the @a TypedefTypeSymbol
     * with name 'Time'.  By default, the type is double, but this can be overridden by the model
     * developer using the time_type statement in the model source code.
     */

    static void default_symbols();
    static void post_parse_all();


    /**
     * Store a C++ style single-line comment for later use.
     *
     * @param cmt          The comment.
     * @param [in,out] loc The source code location.
     */

    static void process_cxx_comment(const string& cmt, yy::location& loc);


    /**
     * Store a C style comment for later use.
     *
     * @param cmt          The comment.
     * @param [in,out] loc The source code location.
     */

    static void process_c_comment(const string& cmt, yy::location& loc);


    /**
     * Determine if @a tok is an om outer keyword (introducing a syntactic declarative island)
     *
     * For example the tokens for 'agent' and 'table' are outer level keywords,
     * but the token for 'int' is not.
     * 
     * @param tok The token.
     *
     * @return true if an om outer keyword, false if not.
     */

    static bool is_om_outer_keyword(const token_type& tok);


    /**
     * Query if @a nm is an om developer-supplied function. These are functions with special names,
     * for example 'Simulation'.
     *
     * @param nm The name.
     *
     * @return true if om developer function, false if not.
     */

    static bool is_om_developer_function(const char* nm);


    /**
     * The symbol table.
     */

    static symbol_map_type symbols;


    /**
    * The types in the model
    *
    * Populated after parsing is complete.
    */

    static list<TypeSymbol *> pp_all_types;


    /**
    * The languages in the model
    *
    * Populated after parsing is complete.
    */

    static list<LanguageSymbol *> pp_all_languages;


    /**
    * The agents in the model
    *
    * Populated after parsing is complete.
    */

    static list<AgentSymbol *> pp_all_agents;


    /**
     * The tables in the model
     * 
     * Populated after parsing is complete.
     */

    static list<TableSymbol *> pp_all_tables;


    /**
     * The parameters in the model
     * 
     * Populated after parsing is complete.
     */

    static list<ParameterSymbol *> pp_all_parameters;


    /**
     * Map of member function qualified names to all identifiers used in the body of the function.
     * 
     * An example entry might be "Person::Mortality" ==> "alive".
     */

    static multimap<string, string> memfunc_bodyids;


    /**
     * A list of all the C++ style single line comments in the model source code
     * 
     * This is a place holder collection at the moment, for testing.
     * It will be replaced by a collection which also includes source code location information.
     */

    static forward_list<string> cxx_comments;


    /**
    * A list of all the C style multi-line comments in the model source code
    *
    * This is a place holder collection at the moment, for testing.
    * It will be replaced by a collection which also includes source code location information.
    */

    static forward_list<string> c_comments;


    /**
     * Map from a token to the preferred string representation of that token.
     * 
     * This is a fixed map independent of any model.  It has a unique key and maps the symbol enum
     * to the preferred term. There is an exact one-to-one correspondence with code in @a parser.y.
     * Maintain this correspondence in all changes or additions. Unfortunately, bison 2.7 with C++
     * does not expose yytname so we need to create this hard-coded equivalent.
     *
     * @return The token string.
     */

    static unordered_map<token_type, string, std::hash<int> > Symbol::token_string;


    /**
     * Map from a string to the token associated with that string.
     * 
     * This is a fixed map independent of any model.  The lexical scanner calls @a string_to_token
     * to map each word to the corresponding preferred token using this map. The map is initialized
     * with deprecated synonyms for preferred terms. These deprecated terms are only in the map @a
     * string_token, not in the map @a token_string. On the first call to the helper function @a
     * string_to_token, this map is populated using all entries in the reciprocal map @a
     * token_string.
     */

    static unordered_map<string, token_type> Symbol::string_token;


    /**
     * The list of om outer keywords.
     * 
     * This is a fixed map independent of any model.  During parsing these keywords cause a
     * transition from C++ code to om declarative code islands.  There is an exact one-to-one
     * relationship between this list and the first block of entries in the list @a token_string.
     *
     * @return The om outer keywords.
     */

    static unordered_set<token_type, std::hash<int> > om_outer_keywords;


    /**
     * The list of om developer functions.
     * 
     * This is a fixed map independent of any model.  These are C++ functions with fixed names which
     * are supplied by the om developer in the model source code.  An example is the 'Simulation'
     * function.
     */

    static unordered_set<string> om_developer_functions;


};



