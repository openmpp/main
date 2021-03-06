/**
 * @file    Symbol.h
 * Declarations for the Symbol class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <forward_list>
#include <map>
#include <set>
#include <tuple>
#include "omc_common.h"
#include "location.hh"
#include "SpecialGlobal.h"
#include "parser.hpp"

class CodeBlock;
class TypeSymbol;
class EnumerationSymbol;
class EnumerationWithEnumeratorsSymbol;
class LanguageSymbol;
class StringSymbol;
class EntitySymbol;
class ParameterSymbol;
class EntitySetSymbol;
class TableSymbol;
class EntityTableSymbol;
class DerivedTableSymbol;
class ParameterGroupSymbol;
class TableGroupSymbol;
class AnonGroupSymbol;
class DependencyGroupSymbol;
class GlobalFuncSymbol;
class AggregationSymbol;
class ImportSymbol;
class ParameterGroupSymbol;
class TableGroupSymbol;

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


// Specialization of std::hash for yy::position.
// Required for creating unordered_map for lookup of comment
// text by start position.

namespace std {
    template <>
    struct hash<yy::position>
    {
        size_t operator()(const yy::position& k) const
        {
            // Comments are uniquely determined by first position in location
            return (
                      (hash<unsigned int>()(k.column))
                    ^ (hash<unsigned int>()(k.line) << 7) // assume max 128 columns
                    ^ (hash<unsigned int>()(*k.filename->cbegin()) << 10) // assume max 1024 lines 
                   );
        }
    };
}

/**
 * Defines an alias representing type of the comment map.
 */
typedef unordered_map<yy::position, string> comment_map_type;

/**
 * Defines an alias representing type of a pair in the comment map.
 */
typedef unordered_map<yy::position, string>::value_type comment_map_value_type;


// ======================================================

// TODO: Explain better the use of double indirection to persist of Symbols and allow morphing within the SYmbol class hierarchy.
// When they need to be used, Symbol*& is used to get the underlying object.
// That lets us morph an object to another type during / parsing, while maintaining
// the persistence of symbol uses in other objects.
// A rule must be followed, which is to never store a pointer to a Symbol, either in a Symbol or another object.
// Instead, store a reference to the pointer of, which can always be obtained using the member function stable_rp().
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
     * Query if this object is an underived instance of the base Symbol class.
     * 
     * Every class derived from Symbol has a polymorphic override of this function which returns
     * false.  The definition of this function is placed at the beginning of the class declaration
     * to help remember to include it when copying code to create new derived classes of Symbol.
     *
     * @return true if base symbol, false if not.
     */
    virtual bool is_base_symbol() const { return true; }

    /**
     * Name-based constructor.
     * 
     * If the symbol table does not contain an entry with unique name @a unm a new symbol table
     * entry is created with this unique name.
     * 
     * This constructor sets the name equal to the unique name, and is used for symbols outside
     * agent scope, e.g. classifications.
     * 
     * If the symbol table does contain an entry with unique name @a unm the existing symbol is
     * morphed to a new base class Symbol. This is useful for replacing a default symbol by a symbol
     * specified in the model source code. For example, a default NumericSymbol with name 'Time' and
     * type 'double' is inserted into the symbol table at initialization.  If a time_type statement
     * is encountered in model code, this symbol is replaced by a new symbol with the developer-
     * specified type used for time.
     *
     * @param unm      The unique name for the symbol.
     * @param decl_loc (Optional) the declaration location.
     */
    explicit Symbol(string unm, yy::location decl_loc = yy::location())
        : unique_name( unm )
        , name ( unm )
        , decl_loc(decl_loc)
        , redecl_loc(yy::location())
        , reference_count(0)
        , sorting_group(10)
        , code_order(0)
        , code_label_allowed(true)
    {
        auto it = symbols.find( unique_name );
        if (it == symbols.end() ) {
            // add to symbol table
            symbol_map_value_type element( unique_name, this );
            symbols.insert( element );
        }
        else {
            // redeclaration or override of default - morph to new symbol
            // preserve reference count
            reference_count = it->second->reference_count;
            // delete old symbol
            delete it->second;
            // replace with new symbol
            it->second = this;
        }
    }

    /**
     * Name-based constructor in agent context.
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
     * @param nm       The name.
     * @param agent    The agent qualifying the name.
     * @param decl_loc (Optional) the declaration location.
     */
    explicit Symbol(const string nm, const Symbol *agent, yy::location decl_loc = yy::location())
        : unique_name(symbol_name(nm, agent))
        , name ( nm )
        , decl_loc(decl_loc)
        , redecl_loc(yy::location())
        , reference_count(0)
        , sorting_group(10)
        , code_order(0)
        , code_label_allowed(true)
    {
        auto it = symbols.find( unique_name );
        if (it == symbols.end() ) {
            // add to symbol table
            symbol_map_value_type element( unique_name, this );
            symbols.insert( element );
        }
        else {
            // redeclaration or override of default - morph to new symbol
            // preserve reference count
            reference_count = it->second->reference_count;
            // delete old symbol
            delete it->second;
            // replace with new symbol
            it->second = this;
        }
    }

    /**
     * Morphing constructor.
     * 
     * This constructor is used to morph an existing symbol table entry to a new kind of symbol.  It
     * is called by constructors for derived Symbol classes. The existing Symbol is destroyed, and
     * the symbol table entry is modified to refer to the new symbol.
     *
     * @param [in,out] sym [in,out] Original symbol to be morphed.
     * @param decl_loc     (Optional) the declaration location.
     */
    explicit Symbol(Symbol*& sym, yy::location decl_loc = yy::location())
        : unique_name(sym->unique_name)
        , name ( sym->name )
        , decl_loc(decl_loc)
        , redecl_loc(yy::location())
        , reference_count(0)
        , sorting_group(10)
        , code_order(0)
        , code_label_allowed(true)
    {
        // find symbol table entry for the existing symbol
        auto it = symbols.find( unique_name );
        // preserve reference count
        reference_count = it->second->reference_count;
        // delete existing symbol
        delete it->second;
        // replace with new symbol
        it->second = this;
    }

    /**
     * Get stable reference to pointer to symbol.
     * 
     * The result is a double indirection and is stable to morphing, since unordered_map guarantees
     * that references to existing map entries never change.
     *
     * @return Reference to pointer to this Symbol.
     */
	Symbol*& stable_rp() const
    {
    	auto it = symbols.find( unique_name );
        assert(it->second); // code integrity guarantee
        it->second->reference_count++;
    	return it->second;
    }

    /**
     * Get stable pointer to pointer to symbol.
     * 
     * The result is a double indirection and is stable to morphing, since unordered_map guarantees
     * that pointers and references to existing map entries never change. It's better to use
     * stable_rp() if possible, but sometimes pointers are needed instead, for example to store a
     * double indirection to a Symbol in a list&lt;&gt;.
     *
     * @return pointer to pointer to this Symbol.
     */
	Symbol** stable_pp() const
    {
    	auto it = symbols.find( unique_name );
        assert(it->second); // code integrity guarantee
        it->second->reference_count++;
    	return &(it->second);
    }

    virtual ~Symbol()
    {
    }


    /**
     * Values that represent a post-parse pass of all symbols
     */

    enum post_parse_pass {

		///< post-parse pass to identify foreign types
		eCreateForeignTypes,

		///< post-parse pass to create/morph missing symbols
		eCreateMissingSymbols,

		///< post-parse pass to assign labels, etc.
        eAssignLabel,

        ///< post-parse pass to assign pp_ members, etc.
        eAssignMembers,

        ///< post-parse pass to resolve data types of derived attributes, etc.
        eResolveDataTypes,

        ///< post-parse pass to populate pp_ collections, etc.
        ePopulateCollections,

        ///< post-parse pass to populate dependencies, side-effects, etc.
        ePopulateDependencies,

    };

    /**
     * Perform post-parse operations.
     * 
     * Post-parse operations create derived information based on information obtained during the
     * parse phase. This information is stored in members of the Symbol class hierarchy which by
     * convention have a 'pp_' prefix in their name. For example, the \ref pp_callback_members
     * collection of the \ref EntitySymbol class contains a list of all agentvars of the agent after
     * post-parse processing.  These pp_ members are used subsequently in the code-generation phase.
     * 
     * When post_parse is called on an object in the \ref Symbol hierarchy, the call is first passed
     * upwards through the inheritance hierarchy so that post-parse operations are performed at all
     * hierarchical levels, and at higher levels before lower levels (for each @a pass).
     * 
     * Post-parse operations occur in a series of sequential passes, with the results of each pass
     * usable by subsequent passes.  \ref post_parse is called on all symbols through the function \
     * ref Symbol::post_parse_all.
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
     * When this function is called on an object in the \ref Symbol hierarchy, the call is first
     * passed upwards through the inheritance hierarchy so that operations are performed at all
     * hierarchical levels, and at higher levels before lower levels.
     *
     * @param [in,out] metaRows The meta rows.
     */
    virtual void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * Gets a short pretty name for a symbol
     * 
     * Used for diagnostics and in comments in generated code.
     *
     * @return The short pretty name as a string.
     */
    virtual string pretty_name() const;

    /**
     * Description of code injection into side-effect function.
     * 
     * Used to insert a comment in generated code.
     *
     * @return A string.
     */
    CodeBlock injection_description();

    /**
     * Process a semantic error encountered during the post-parse phase.
     * 
     * Source code location is that of Symbol
     *
     * @param msg The message.
     */
    void pp_error(const string& msg);

    /**
     * Process a fatal semantic error encountered during the post-parse phase.
     * 
     * Source code location is that of Symbol.  An exception is thrown
     * and further post-parse processing is aborted.
     *
     * @param msg The message.
     */
    void pp_fatal(const string& msg);

    /**
     * Process a semantic warning encountered during the post-parse phase.
     * 
     * Source code location is that of Symbol
     *
     * @param msg The message.
     */
    void pp_warning(const string& msg);

    /**
     * Output a warning or error message to the log.
     * 
     * Source code location is that of Symbol
     *
     * @param msg The message.
     */
    void pp_log_message(const string& msg);

    /**
     * Get the symbol label in the given language.
     * 
     * Language-specific symbol labels come from structured comments in the model source code
     * or from a symbol-specific value.
     *
     * @param language The language.
     *
     * @return A string.
     */
    virtual string label(const LanguageSymbol& language) const;

    /**
     * Get the default symbol label for the given language.
     * 
     * @param language The language.
     *
     * @return A string.
     */
    virtual string default_label(const LanguageSymbol& language) const;

    /**
     * Gets the symbol label in the default language.
     *
     * @return A string.
     */
    string label() const;

    /**
     * Get the symbol note in the given language.
     * 
     * Language-specific symbol notes come from structured comments in the model source code.
     *
     * @param language The language.
     *
     * @return A string.
     */
    virtual string note(const LanguageSymbol & language) const;

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

    /**
     * The symbol label for each language.
     */
    vector<string> pp_labels;

    /**
     * Search for and process a comment label at a source line.
     *
     * @param pos The position.
     *
     * @return true if it a valid comment line was found and processed, else false.
     */
    bool process_symbol_label(const yy::position& pos);

    /**
     * The symbol notes for each language.
     */
    vector<string> pp_notes;

    /**
     * Unique name lexicographically-compatible with Modgen order for code generation compatibility
     * 
     * Implemented as a virtual function to allow Modgen-compatible differences
     * in lexicographical ordering where desired and/or necessary.
     *
     * @return A string.
     */
    virtual string pp_modgen_name() const
    {
        string result = unique_name;
        // convert to lowercase to match MFC string comparison CompareNoCase.
        // in stl, '_' lies between lower and upper case, but is before any letter in CompareNoCase
        // which transforms letters to lowercase before performing the comparison using ASCII values.
        for (auto & ch : result) {
            ch = tolower(ch);
        }
        return result;
    }

    /**
     * The declaration location.
     * 
     * Set using location information of syntactic elements during parsing.
     */
    yy::location decl_loc;

    /**
     * The redeclaration location.
     * 
     * Set using location information of syntactic elements during parsing.
     */
    yy::location redecl_loc;

    /**
     * Number of references by other symbols to this Symbol
     * 
     * For debugging purposes.  Maintained by stable_rp() and maintained during symbol morphing.
     */
    int reference_count;

    /**
     * The sorting group the Symbol belongs in.
     * 
     * Used to control the order in which code is injected into side-effect call-back functions
     * to handle interdependencies in derived attributes.
     */
    int sorting_group;

    /**
     * The order for code insertion (higher values insert first).
     */
    int code_order;

    /**
     * Indicates if a label from model source code is allowed for this symbol
     */
    bool code_label_allowed;

    /**
     * Check for existence of symbol with this unique name.
     *
     * @param unm The unique name.
     *
     * @return true if found, else false.
     */
    static bool exists(const string& unm);

    /**
     * Check for existence of symbol with this member name in agent.
     *
     * @param nm    The member name.
     * @param agent The agent.
     *
     * @return true if found, else false.
     */
    static bool exists(const string& nm, const Symbol *agent);

    /**
     * Agent member unique name.
     *
     * @param nm    The member name, e.g. "time".
     * @param agent The agent qualifying the member name.
     *
     * @return The unique name, e.g. "Person::time".
     */
    static string symbol_name(const string& nm, const Symbol *agent);

    /**
     * Gets a symbol for a unique name.
     *
     * @param unm The unique name.
     *
     * @return The symbol, or nullptr if it doesn't exist.
     */
    static Symbol *get_symbol(const string& unm);

    /**
     * Gets a symbol for a member name in an agent.
     * 
     * @param nm    The member name.
     * @param agent The agent.
     *
     * @return The symbol, or nullptr if it doesn't exist.
     */
    static Symbol *get_symbol(const string& nm, const Symbol *agent);

    /**
     * Creates (or gets if exists) a symbol for a unique name.
     *
     * @param unm The unique name.
     *
     * @return The symbol.
     */
    static Symbol *create_symbol(const string& unm);

    /**
     * Creates (or gets if exists) a symbol for a member name in an agent.
     *
     * @param nm    The member name.
     * @param agent The agent.
     *
     * @return The symbol.
     */
    static Symbol *create_symbol(const string& nm, const Symbol *agent);

    /**
     * Searches for the first symbol of the given class.
     *
     * @param cls The typeinfo of the class to find.
     *
     * @return null if it fails, else the found symbol.
     */
    static Symbol *find_a_symbol(const type_info& cls);

    /**
     * Gets post-parse direct pointer to a symbol
     * 
     * During the parse phase, relationships between Symbols are stored as stable references to
     * pointers to Symbols (the second element of the pair in the symbol table map
     * Symbols::symbols).  That allows symbol morphing when generic Symbols created by the scanner
     * (when it encounters new names)
     * are resolved in the parser to specialized Symbols.
     *
     * @param [in,out] rp_symbol Symbol stored in the symbol table.
     *
     * @return null if it fails, else the pp symbol.
     */
    static Symbol *pp_symbol(Symbol *& rp_symbol);
    static Symbol *pp_symbol(Symbol ** rp_symbol);

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
     * Storage size of a fundamental C++ type.
     *
     * @param tok The token representing the type, eg. token::TK_int.
     *
     * @return A size_t.
     */
    static const size_t storage_size(token_type tok);

    /**
     * Determine integer/character storage type which can store a bounded value
     * 
     * Used to optimize the storage type used for classications, ranges, and partitions.
     *
     * @param min_value The minimum value.
     * @param max_value The maximum value.
     *
     * @return A token_type.
     */
    static const token_type optimized_storage_type( long long min_value, long long max_value );

    /**
     * Extract accumulator from Modgen cumulation operator.
     *
     * @param e The Modgen cumulation operator, e.g. token::TK_max_value_in.
     *
     * @return The associated accumulator, e.g. token::TK_maximum.
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
     * Populate default symbols in symbol table
     * 
     * These are symbols which are added to the symbol table when it is created.  They have default
     * properties which can be overridden by the om developer.  An example is the @ref NumericSymbol
     * with name 'Time'.  By default, the type is double, but this can be overridden by the model
     * developer using the time_type statement in the model source code.
     */
    static void populate_default_symbols(const string &model_name, const string &scenario_name);

    /**
     * Invalidate the parse phase symbol table.
     * 
     * In a post-parse phase, a direct pointer member (with prefix pp_)
     * is assigned for each double indirection pointer member. All code subsequent to this post-
     * parse phase must use these direct pointer members and not the double indirection pointer
     * members, since they may be invalid due to post-parse fixups carried out by pp_symbol().  To
     * help catch erroneous subsequent use of double indirection pointers, this function changes the
     * Symbol* pointer in each symbol table reference to nullptr.  That way, any code which uses a
     * double-indirection reference to pointer will fail immediately.  The reference remains valid
     * because the symbol table pair still exists.  The pointer stored in that reference, however,
     * will be nullptr.
     * 
     * To examine any symbol by name when debugging the post_parse phase, use the list pp_symbols.
     */
    static void invalidate_symbols();

    /**
     * Populate pp symbols
     * 
     * pp_symbols is a version of the symbol table sorted in a known order, unlike the symbol table which is an unordered map.
     */
    static void populate_pp_symbols();

    /**
     * Sort pp_symbols in default order
     */
    static void default_sort_pp_symbols();

    /**
     * Sort pp_symbols for Modgen compatible code-generation (1).
     * 
     * This sort ignores sorting_group.  It is used just to reproduce Modgen self-scheduling numeric
     * identifiers.
     */
    static void modgen_sort_pp_symbols1();

    /**
     * Sort pp_symbols for Modgen compatible code-generation (2).
     * 
     * This sort respects sorting group.  Within sorting group, order is by Modgen-compatible name.
     */
    static void modgen_sort_pp_symbols2();

    /**
     * Output a log message with source location.
     * 
     * @param msg The message.
     */
    static void pp_logmsg(const yy::location& loc, const string& msg);

    /**
     * Process an error encountered during the post-parse phase.
     * 
     * @param msg The message.
     */
    static void pp_error(const yy::location& loc, const string& msg);

    /**
     * Determines if location is in a source file included via a 'use' statement.
     * 
     * @param loc The location.
     */
    static bool is_use_file(const yy::location& loc);

    /**
     * Perform operations after the parser has parsed all input files.
     * 
     * This includes validating each symbol and populating post-parse symbol members containing
     * information on symbol relationships and pointer collections.
     */
    static void post_parse_all();

    /**
     * Parse all options in model source code
     * 
     * All key-value pairs in options statements are placed in the map Symbol::options during model
     * code parsing for subsequent processing by this function. Unrecognized options are ignored, as
     * are unrecognized values for recognized options.
     */
    static void defaults_and_options();

    /**
     * Determine if @a tok is an om outer keyword (introducing a syntactic declarative island)
     * 
     * For example the tokens for 'agent' and 'table' are outer level keywords, but the token for
     * 'int' is not.
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
    * Mangle name to length 8 for API
    *
    * @param name  The original name.
    * @param index Numeric index associated with the name.
    *
    * @return mangled version of name
    */
    static string mangle_name(string name, int index);

    /**
     * Pathnames of use folders.
     * 
     * Used to resolve 'use' statements to physical files. 
     * It can be specified in omc.ini or as command line argument.
     * By default relative to location of omc.exe executable.
     */
    static vector<string> use_folders;

	/**
     * Source files for the model.
     * 
     * Consists of all source files parsed by the compiler, in parse order. Source file names
     * generally include a leading path portion, which may be absolute or relative to the working
     * directory. This includes files supplied on the command line as well as files specified with
     * the 'use' statement from within model source code.
     */
    static list<string> all_source_files;

	/**
     * 'use' source files for the model.
     * 
     * A subset of all_source_files, consisting just of those which 
     * were included in compilation through a 'use' statement.
     */
    static list<string> use_source_files;

    /**
     * The symbol table.
     * 
     * Map from a Symbol's unique_name to a reference to a pointer to the Symbol. The extra level of
     * indirection allows morphing of a Symbol to a specialized Symbol in the Symbol hierarchy when
     * the declaration is encountered. When morphing is done, the reference in the symbol table is
     * not changed, but the pointer is. That means that references to the Symbol in other objects,
     * which are stored as Symbol &amp;* during parsing, remain valid.
     */
    static symbol_map_type symbols;

    /**
     * The post-parse symbol table.
     * 
     * Populated after parsing is complete by populate_pp_symbols(). Sorted in lexicographic order
     * by unique_name.  Stored as pairs with unique_name in first to make it easy to find a symbol
     * when debugging.
     */
    static list<symbol_map_value_type> pp_symbols;

    /**
     * List of symbols to ignore in post-parse pass.
     * 
     * Symbols morphed in a post-parse pass (eCreateForeignTypes, eCreateMissingSymbols)
	 * will have invalid pair.second (Symbol *) in pp_symbols, so should not be processed
	 * if they happen to come later in pp_symbols than the symbol which morphed them.
	 * Any post_parse code which creates a new symbol or morphs an
     * existing symbol in these passes must push the name into this hash to ensure it is skipped.
     */
    static unordered_set<string> pp_symbols_ignore;

    /**
     * List of all identifiers in model source code.
     * 
     * Includes identifiers in both C++ code and in ompp declarative islands.
     */
    static unordered_set<string> identifiers_in_model_source;

    /**
     * The fundamental types in the model
     * 
     * Ex. Time, real Populated after parsing is complete.
     */
    static list<TypeSymbol *> pp_all_types0;

    /**
     * The types in the model implemented using helper template classes
     * 
     * Ex. classifications, ranges Populated after parsing is complete.
     */
    static list<TypeSymbol *> pp_all_types1;

    /**
     * The languages in the model
     * 
     * Populated after parsing is complete.
     */
    static list<LanguageSymbol *> pp_all_languages;

    /**
     * The strings in the model
     * 
     * Populated after parsing is complete.
     */
    static list<StringSymbol *> pp_all_strings;

    /**
     * The enumerations in the model
     * 
     * Populated after parsing is complete.
     */
    static list<EnumerationSymbol *> pp_all_enumerations;

    /**
     * The enumerations with enumerators in the model
     * 
     * Populated after parsing is complete.
     */
    static list<EnumerationWithEnumeratorsSymbol *> pp_all_enumerations_with_enumerators;

    /**
     * The agents in the model
     * 
     * Populated after parsing is complete.
     */
    static list<EntitySymbol *> pp_all_agents;

    /**
     * The entity sets in the model
     * 
     * Populated after parsing is complete.
     */
    static list<EntitySetSymbol *> pp_all_entity_sets;

    /**
     * The tables in the model
     * 
     * Populated after parsing is complete.
     */
    static list<TableSymbol *> pp_all_tables;

    /**
     * The entity tables in the model
     * 
     * Populated after parsing is complete.
     */
    static list<EntityTableSymbol *> pp_all_entity_tables;

    /**
     * The derived tables in the model
     * 
     * Populated after parsing is complete.
     */
    static list<DerivedTableSymbol *> pp_all_derived_tables;

    /**
     * The parameters in the model
     * 
     * Populated after parsing is complete.
     */
    static list<ParameterSymbol *> pp_all_parameters;

    /**
     * The parameter groups in the model
     * 
     * Populated after parsing is complete.
     */
    static list<ParameterGroupSymbol *> pp_all_parameter_groups;

    /**
     * The table groups in the model
     * 
     * Populated after parsing is complete.
     */
    static list<TableGroupSymbol *> pp_all_table_groups;

    /**
     * The anon groups in the model
     * 
     * Populated after parsing is complete.
     */
    static list<AnonGroupSymbol *> pp_all_anon_groups;

    /**
     * The global funcs in the model
     * 
     * Populated after parsing is complete.
     */
    static list<GlobalFuncSymbol *> pp_all_global_funcs;

    /**
     * The aggregations in the model
     * 
     * Populated after parsing is complete.
     */
    static list<AggregationSymbol *> pp_all_aggregations;

    /**
     * The dependency groups in the model
     * 
     * Populated after parsing is complete.
     */
    static list<DependencyGroupSymbol *> pp_all_dependency_groups;

    /**
    * The imports in the model
    *
    * Populated after parsing is complete.
    */
    static list<ImportSymbol*> pp_all_imports;

    /**
    * The event names in the model
    *
    * Note that the set contains no duplicates, but event names can be duplicates in different entities.
    * Populated after parsing is complete.
    */
    static set<string> pp_all_event_names;

    /**
     * Map of member function qualified names to all identifiers used in the body of the function.
     * 
     * An example entry might be "Person::MortalityEvent" ==> "alive".
     */
    static multimap<string, string> memfunc_bodyids;

    /**
     * Map of member function qualified names to the parameter list of each (as a vector of strings).
     * 
     * Examples:
     *
     * "Person::timeMortality" ==> <empty>
     * "Person::timeMoveEvent" ==> int * p_destination
     */
    static map<string, vector<string> > memfunc_parmlist;

    /**
     * Map of member function qualified names to the definition location.
     */
    static map<string, yy::location> memfunc_defn_loc;

    /**
     * A map of all the C++ style single line comments in the model source code, indexed by location
     * 
     * This map is used with other collections to retrieve a comment based on its location.
     */
    static comment_map_type cxx_comments;

    /**
     * A map of all the C style multi-line comments in the model source code, indexed by location
     * 
     * This map is used with other collections to retrieve a comment based on its location.
     */
    static comment_map_type c_comments;

    /**
     * Map of LABEL from symbol name to label.
     * 
     * This maps "symbol,language" as a key to the associated label specified
     * explicitly by a //LABEL statement in the model source code.
     */
    static unordered_map<string, string> explicit_labels;

    /**
     * Map of NOTE from symbol name to note (model source)
     * 
     * This maps "symbol,language" as a key to the associated note specified
     * explicitly by a NOTE statement in the model source code.
     */
    static unordered_map<string, string> notes_source;

    /**
     * Map of NOTE from symbol name to note (parameter input values)
     * 
     * This maps "symbol,language" as a key to the associated note specified
     * explicitly by a NOTE statement in the model source code.
     */
    static unordered_map<string, string> notes_input;

    /**
     * Set of functions to scan for human language string literal as first argument
     * 
     * Used to identify strings for translation, e.g. LT, logMsg, etc.
     */
    static unordered_set<string> tran_funcs;

    /**
     * Set of human language strings for translation.
     * 
     * String literals in model source which are arguments of 
     * tran_funcs, e.g. e.g. LT, logMsg, etc.
     */
    static unordered_set<string> tran_strings;

    /**
     * Map from a token to the preferred string representation of that token.
     * 
     * This is a fixed map independent of any model.  It has a unique key and maps the symbol enum
     * to the preferred term. There is an exact one-to-one correspondence with code in @a parser.y.
     * Maintain this correspondence in all changes or additions. Unfortunately, bison 2.7 with C++
     * does not expose yytname so we need to create this hard-coded equivalent.
     */
    static unordered_map<token_type, string, std::hash<int> > token_string;

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
    static unordered_map<string, token_type> string_token;

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

    /**
     * PreSimulation functions in model code.
     */
    static SpecialGlobal pre_simulation;

    /**
     * PostSimulation functions in model code.
     */
    static SpecialGlobal post_simulation;

    /**
     * UserTables functions in model code.
     */
    static SpecialGlobal derived_tables;

    /**
     * If true, model contains one or more parameters_suppress statements
     */
    static bool any_parameters_suppress;

    /**
     * If true, model contains one or more parameters_retain statements
     */
    static bool any_parameters_retain;

    /**
     * If true, model contains one or more tables_suppress statements
     */
    static bool any_tables_suppress;

    /**
     * If true, model contains one or more tables_retain statements
     */
    static bool any_tables_retain;

    /**
     * If true, model contains one or more parameters_to_tables statements
     */
    static bool any_parameters_to_tables;

    /**
     * If true, table measures are aggregated across simulation members.
     * If false, table measures are averaged across simulation members.
     */
    static bool measures_are_aggregated;

    /**
     * A count of type changes made in a single post-parse pass.
     */
    static int type_changes;

    /**
     * Maximum errors before bailing.
     */
    static const int max_error_count = 50;

    /**
     * A count of errors identified during post-parse processing.
     */
    static int post_parse_errors;

    /**
     * A count of warnings identified during post-parse processing.
     */
    static int post_parse_warnings;

    /**
     * All key-value pairs given in options statements in model code.
     */
    static unordered_map<string, string> options;

    /**
     * true if event_trace capability is requested in options statement.
     */
    static bool option_event_trace;

    /**
     * true/false depending on bounds_errors in options statement.
     */
    static bool option_bounds_errors;

    /**
     * true if case_checksum activated in options statement.
     */
    static bool option_case_checksum;

    /**
     * true if allow_time_travel activated in options statement.
     */
    static bool option_allow_time_travel;

    /**
     * true if allow_clairvoyance activated in options statement.
     */
    static bool option_allow_clairvoyance;

    /**
     * true if time_infinite_is_32767 activated in options statement.
     */
    static bool option_time_infinite_is_32767;

    /**
     * true if time_undef_is_minus_one activated in options statement.
     */
    static bool option_time_undef_is_minus_one;

    /**
     * true or false depending on verify_attribute_modification value in options statement.
     */
    static bool option_verify_attribute_modification;

    /**
     * Code page to use for source file conversion
     */
    static string code_page;

    /**
     * True to disable inserted #line directives in generated files
     */
    static bool no_line_directives;

    /**
     * True to enable detailed output from parser
     */
    static bool trace_parsing;

    /**
     * True to enable detailed output from scanner
     */
    static bool trace_scanning;

    /**
     * true to sort post-parse symbol table using Modgen name order where it matters.
     * 
     * For compatible event trace outputs between equivalent Modgen and ompp models,
     * the lexicographical order of self-scheduling attributes must correspond.
     */
    static bool modgen_sort_option;
};



