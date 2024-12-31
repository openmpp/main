/**
 * @file    Symbol.h
 * Declarations for the Symbol class.
 */

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
#include "omc_location.hh"
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
class AttributeGroupSymbol;
class AnonGroupSymbol;
class DependencyGroupSymbol;
class GlobalFuncSymbol;
class AggregationSymbol;
class ImportSymbol;
class ModuleSymbol;
class ParameterGroupSymbol;
class TableGroupSymbol;
class GroupSymbol;
class EntityFuncSymbol;
class GlobalFuncSymbol;
class IdentityAttributeSymbol;
class DerivedAttributeSymbol;

namespace openm {
    struct MetaModelHolder;
}

using namespace std;

/**
 * Defines an alias used to reference bison-generated token values.
 * 
 * For example token::TK_table is the value of the bison-generated token associated with the
 * openM++ keyword 'table'.
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


// Specialization of std::hash for omc::position.
// Required for creating unordered_map for lookup of comment
// text by start position.

namespace std {
    template <>
    struct hash<omc::position>
    {
        size_t operator()(const omc::position& k) const
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
typedef unordered_map<omc::position, string> comment_map_type;

/**
 * Defines an alias representing type of a pair in the comment map.
 */
typedef unordered_map<omc::position, string>::value_type comment_map_value_type;


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
     * entity scope, e.g. classifications.
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
    explicit Symbol(string unm, omc::location decl_loc = omc::location())
        : unique_name( unm )
        , name ( unm )
        , decl_loc(decl_loc)
        , redecl_loc(omc::location())
        , reference_count(0)
        , sorting_group(10)
        , code_order(0)
        , pp_pass(eBeforeFirst)
        , pp_symbol_id(0)
        , code_label_allowed(true)
        , pp_module(nullptr)
    {
        modgen_unique_name = unique_name;  // is overridden if necessary
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
     * Name-based constructor in entity context.
     * 
     * This constructor creates a Symbol with unique name based on
     * @a nm in the context of @a entity, e.g. "Person::time"
     * 
     * If the symbol table does not contain a corresponding entry a new symbol table entry is
     * created with this unique name.
     * 
     * If the symbol table does contain a corresponding entry the existing symbol is morphed to a
     * new base class Symbol.
     *
     * @param nm       The name.
     * @param ent      The entity qualifying the name.
     * @param decl_loc (Optional) the declaration location.
     */
    explicit Symbol(const string nm, const Symbol * ent, omc::location decl_loc = omc::location())
        : unique_name(symbol_name(nm, ent))
        , name ( nm )
        , decl_loc(decl_loc)
        , redecl_loc(omc::location())
        , reference_count(0)
        , sorting_group(10)
        , code_order(0)
        , pp_pass(eBeforeFirst)
        , pp_symbol_id(0)
        , code_label_allowed(true)
        , pp_module(nullptr)
    {
        modgen_unique_name = unique_name;  // is overridden if necessary
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
    explicit Symbol(Symbol*& sym, omc::location decl_loc = omc::location())
        : unique_name(sym->unique_name)
        , name ( sym->name )
        , decl_loc(decl_loc)
        , redecl_loc(omc::location())
        , reference_count(0)
        , sorting_group(10)
        , code_order(0)
        , pp_pass(eBeforeFirst)
        , pp_symbol_id(0)
        , code_label_allowed(true)
        , pp_module(nullptr)
    {
        modgen_unique_name = unique_name;  // is overridden if necessary
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
     * Values that represent errors in the argument of RandUniform, etc.
     */

    enum random_stream_error {

        ///< missing stream argument
        eMissingStreamArgument = -1,

        ///< missing stream argument
        eInvalidStreamArgument = -2,
    };


    /**
     * Values that represent a post-parse pass of all symbols
     */

    enum post_parse_pass {

        ///< post-parse pass before first pass
        eBeforeFirst,

		///< post-parse pass to identify foreign types and process languages
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
     * collection of the \ref EntitySymbol class contains a list of all attributes of the entity after
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
     * Get the c++ declaration code (in entity scope) associated with the symbol.
     * 
     * The code fragment is valid within an entity class declaration.
     * 
     * When this function is called on an object in the \ref Symbol hierarchy, the call is first
     * passed upwards through the inheritance hierarchy so that operations are performed at all
     * hierarchical levels, and at higher levels before lower levels.
     *
     * @return A CodeBlock.
     */
    virtual CodeBlock cxx_declaration_entity();

    /**
     * Get the c++ definition code (for entity members) associated with the symbol.
     * 
     * The code fragment is qualified by the entity class.
     * 
     * When this function is called on an object in the \ref Symbol hierarchy, the call is first
     * passed upwards through the inheritance hierarchy so that operations are performed at all
     * hierarchical levels, and at higher levels before lower levels.
     *
     * @return A CodeBlock.
     */
    virtual CodeBlock cxx_definition_entity();

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
     * The code fragment is not qualified by any entity class.
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
     * True if this object is published to the database.
     *
     * This virtual function is overridden as appropriate by specializations of Symbol.
     */
    virtual bool is_published() const { return false; }

    /**
     * Returns unique_name with "::" replaced by "."
     *
     * @return The string result.
     */
    string dot_name() const
    {
        auto n = unique_name.find("::");
        if ((n != unique_name.npos) && (unique_name.length() > n + 2)) {
            return unique_name.substr(0, n) + "." + unique_name.substr(n + 2);
        }
        else {
            return unique_name;
        }
    }

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
    void pp_error(const string& msg) const;

    /**
     * Process a fatal semantic error encountered during the post-parse phase.
     * 
     * Source code location is that of Symbol.  An exception is thrown
     * and further post-parse processing is aborted.
     *
     * @param msg The message.
     */
    void pp_fatal(const string& msg) const;

    /**
     * Process a semantic warning encountered during the post-parse phase.
     * 
     * Source code location is that of Symbol
     *
     * @param msg The message.
     */
    void pp_warning(const string& msg) const;

    /**
     * Process a semantic warning encountered during the post-parse phase.
     *
     * Source code position is second argument
     *
     * @param msg The message.
     */
    void pp_warning(const string& msg, const omc::position& pos) const;

    /**
     * Process a semantic message during the post-parse phase.
     *
     * Source code location is that of Symbol
     *
     * @param msg The message.
     */
    void pp_message(const string& msg);

    /**
     * Output a warning or error message to the log.
     * 
     * Source code location is that of Symbol
     *
     * @param msg The message.
     */
    void pp_log_message(const string& msg) const;

    /**
     * Output a warning or error message to the log.
     *
     * Source code location is second argument.
     *
     * @param msg The message.
     */
    void pp_log_message(const string& msg, const omc::position &pos) const;

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

    /** English label provided for built-in symbols (optional) */
    string builtin_english_label;

    /** alingual label provided for built-in symbols (optional) */
    string builtin_alingual_label;

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
     * Associate explicit label or note with symbol.
     *
     * An explicit label is a C++ one-line comment like LABEL(key,EN) text
     * A note is a C-style comment like NOTE(key,EN) text
     * Alternatively, explicit labels and notes can be supplied
     * by files in the authored input doc directory, with names like
     * LABEL.TheSymbol.EN.md or NOTE.TheSymbol.EN.md.
     * 
     * @param key The key used for searching, e.g. MyTable::Dim0
     */
    void associate_explicit_label_or_note(const string& key);

    /**
     * The unique identifier for the symbol
     * 
     * To create unique names for all attributes, member names in the model source code are prefixed
     * with the entity context to create a unique name for symbol table lookup,
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
     * A unique identifier for the symbol used in LABEL and NOTE comments.
     * 
     * Differs from unique_name for table dimensions, for historical reasons.
     * Modgen numbered dimensions Dim0,Dim1,... including the expression dimension
     * Ompp numbers dimensions Dim0,Dim1,... excluding the expression dimension
     */
    string modgen_unique_name;

    /**
     * The symbol label for each language.
     * (indexed by language)
     */
    vector<string> pp_labels;

    /**
     * True if symbol label was provided explicitly in model source
     * (indexed by language)
     */
    vector<bool> pp_labels_explicit;

    /**
     * Code position of symbol label
     * (indexed by language)
     */
    vector<omc::position> pp_labels_pos;

    /**
     * True if symbol label was provided explicitly in model source for the default language
     */
    bool is_label_supplied(void) const
    {
        // 0 is the default language
        return (pp_labels_explicit.size() == 0) ? false : pp_labels_explicit[0];
    }

    /**
     * Search for and process a comment label at a source line.
     *
     * @param pos The position.
     *
     * @return true if it a valid comment line was found and processed, else false.
     */
    bool process_symbol_label(const omc::position& pos);

    /**
     * The symbol notes for each language.
     * (indexed by language)
     */
    vector<string> pp_notes;

    /**
     * Code position of symbol note
     * (indexed by language)
     */
    vector<omc::position> pp_notes_pos;

    /**
     * True if symbol note was provided explicitly in model source for the default language
     */
    bool is_note_supplied(void) const
    {
        // 0 is the default language
        return (pp_notes.size() == 0) ? false : (pp_notes[0].size() > 0);
    }

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
    omc::location decl_loc;

    /**
     * The redeclaration location.
     * 
     * Set using location information of syntactic elements during parsing.
     */
    omc::location redecl_loc;

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
     * Most recent post-parse pass
     */
    post_parse_pass pp_pass;

    /**
     * Unique arbitary identifier
     */
    int pp_symbol_id;

    /**
     * Indicates if a label from model source code is allowed for this symbol
     */
    bool code_label_allowed;

    /**
     * If non-null, the module where this symbol was declared.
     */
    ModuleSymbol *pp_module;

    /**
     * Parent groups of this symbol.
     */
    set <GroupSymbol*> pp_parent_groups;

    bool has_parent_group(void) const
    {
        return (pp_parent_groups.size() > 0);
    }

    /**
     * All EntityFuncSymbol which use this Symbol
     */
    set<EntityFuncSymbol *> pp_entity_funcs_using;

    /**
     * All GlobalFuncSymbol which use this Symbol
     */
    set<GlobalFuncSymbol *> pp_global_funcs_using;

    /**
     * All IdentityAttributeSymbol which use this Symbol
     */
    set<IdentityAttributeSymbol*> pp_identity_attributes_using;

    /**
     * All DerivedAttributeSymbol which use this Symbol
     */
    set<DerivedAttributeSymbol*> pp_derived_attributes_using;

    /**
     * All EntityTableSymbol which use this Symbol
     */
    set<EntityTableSymbol*> pp_entity_tables_using;

    /**
     * All EntitySetSymbol which use this Symbol
     */
    set<EntitySetSymbol*> pp_entity_sets_using;

    /**
     * Check for existence of symbol with this unique name.
     *
     * @param unm The unique name.
     *
     * @return true if found, else false.
     */
    static bool exists(const string& unm);

    /**
     * Check for existence of symbol with this member name in entity.
     *
     * @param nm    The member name.
     * @param ent   The entity.
     *
     * @return true if found, else false.
     */
    static bool exists(const string& nm, const Symbol * ent);

    /**
     * Agent member unique name.
     *
     * @param nm    The member name, e.g. "time".
     * @param ent   The entity qualifying the member name.
     *
     * @return The unique name, e.g. "Person::time".
     */
    static string symbol_name(const string& nm, const Symbol * ent);

    /**
     * Gets a symbol for a unique name.
     *
     * @param unm The unique name.
     *
     * @return The symbol, or nullptr if it doesn't exist.
     */
    static Symbol *get_symbol(const string& unm);

    /**
     * Gets a symbol for a member name in an entity.
     * 
     * @param nm    The member name.
     * @param ent   The entity.
     *
     * @return The symbol, or nullptr if it doesn't exist.
     */
    static Symbol *get_symbol(const string& nm, const Symbol * ent);

    /**
     * Creates (or gets if exists) a symbol for a unique name.
     *
     * @param unm The unique name.
     *
     * @return The symbol.
     */
    static Symbol *create_symbol(const string& unm);

    /**
     * Creates (or gets if exists) a symbol for a member name in an entity.
     *
     * @param nm    The member name.
     * @param ent   The entity.
     *
     * @return The symbol.
     */
    static Symbol *create_symbol(const string& nm, const Symbol * ent);

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
     * Formatted declaration location of a Symbol (given as name)
     *
     * @param unm    The symbol name.
     *
     * Example result: Mortality.mpp[42]
     * 
     * @return The declaration location as a string, possibly empty.
     */
    static string symbol_decl_pretty(const string& unm);

    /**
     * Get the string corresponding to a token.
     *
     * @param e The token value, e.g. TK_entity.
     *
     * @return The string representation of the token, e.g. "entity".
     */
    static const string token_to_string(const token_type& e);

    /**
     * Get the token corresponding to a string.
     *
     * @param s A string with an associated token, e.g. "entity".
     *
     * @return The token associated with the string, e.g. token::TK:entity. If the string is not a
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
     * Extract statistic from Modgen cumulation operator.
     *
     * @param   e    The Modgen cumulation operator, e.g. token::TK_max_value_in.
     * @param   dflt The default statistic.
     *
     * @returns The associated statistic, e.g. token::TK_maximum.
     */
    static const token_type modgen_cumulation_operator_to_stat(const token_type& e, const token_type& dflt);

    /**
     * Extract increment from Modgen cumulation operator.
     *
     * @param   e    The Modgen cumulation operator, e.g. token::TK_max_value_in.
     * @param   dflt The default increment.
     *
     * @returns The associated increment, e.g. token::TK_value_in.
     */
    static const token_type modgen_cumulation_operator_to_incr(const token_type& e, const token_type& dflt);

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
    static void pp_logmsg(const omc::location& loc, const string& msg);

    /**
     * Process an error encountered during the post-parse phase.
     * 
     * @param msg The message.
     */
    static void pp_error(const omc::location& loc, const string& msg);

    /**
     * Determines if location is in a source file included via a 'use' statement.
     * 
     * @param loc The location.
     */
    static bool is_use_file(const omc::location& loc);

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
     * code parsing for subsequent processing by this function.
     * This function removes options from the map if recognized and processed.
     * Some options require delayed processing elsewhere, e.g. local_random_streams, lifecycle_attributes
     */
    static void do_options();

    /**
     * Assign provenance of ModuleSymbols
     */
    static void do_module_provenance(void);

    /**
     * Create missing global functions
     */
    static void create_missing_global_funcs(void);

    /**
     * Look for and handle unrecognized options after post-parse phases
     */
    static void do_unrecognized_options();

    /**
     * Determine if @a tok is an om outer keyword (introducing a syntactic declarative island)
     * 
     * For example the tokens for 'entity' and 'table' are outer level keywords, but the token for
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
     * Create and apply heuristic short names for dimensions and measures
     *
     */
    static void heuristic_short_names(void);

    /**
     * Generate code to specify generated short names for dimensions and measures
     *
     * @return a CodeBlock containing the generated code
     */
    static CodeBlock build_NAME_code(void);

    /**
     * Generate notional code which lists generated event dependencies
     *
     * @return a CodeBlock containing the generated code
     */
    static std::list<std::string> build_event_dependencies_csv(void);

    /**
     * Generate csv echoing model import statements
     *
     * @return a string
     */
    static std::string build_imports_csv(void);

    /**
     * Replace Modgen syntax in a NOTE by equivalent markdown.
     *
     * @param note The note.
     */
    static std::string note_modgen_to_markdown(const std::string& note);

    /**
     * Expand embedded constructs in a NOTE.
     *          
     * Example: "GetLabel(SymbolName)" is expanded to the label of the Symbol
     *
     * @param   lang_index  Zero-based index of the language.
     * @param   note        The note contents.
     *
     * @returns A std::string.
     */
    std::string note_expand_embeds(int lang_index, const std::string& note);

    /**
     * Slurp a developer-supplied .md documentation file.
     *
     * @returns A std::string.
     */
    static std::string slurp_doc_file_md(const std::string& stem);

    /**
     * Slurp a developer-supplied .txt documentation file.
     *
     * @returns A std::string.
     */
    static std::string slurp_doc_file_txt(const std::string& stem);

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
     * 'mpp' source files for the model.
     *
     * A subset of all_source_files, just those 
     * in the model code directory.
     * It includes both mpp and ompp files.
     */
    static list<string> mpp_source_files;

	/**
     * 'use' source files for the model.
     * 
     * A subset of all_source_files, just those 
     * included during compilation through a 'use' statement.
     */
    static list<string> use_source_files;

    /**
     * 'dat' source files for the model.
     *
     * A subset of all_source_files, just those
     * in the model parameters/Default directory.
     * It includes both dat and odat files.
     */
    static list<string> dat_source_files;

    /**
     * line counts of parsed files.
     * 
     * key is full name with path
     */
    static map<string, int> source_files_line_count;

    /**
     * Syntactic island line counts of parsed files.
     *
     * key is full name with path
     */
    static map<string, int> source_files_island_line_count;

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
     * The entities in the model
     * 
     * Populated after parsing is complete.
     */
    static list<EntitySymbol *> pp_all_entities;

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
     * True if model contains any screened entity tables
     *
     * Assigned after parsing is complete.
     */
    static bool pp_any_screened_entity_tables;

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
     * The attribute groups in the model
     *
     * Populated after parsing is complete.
     */
    static list<AttributeGroupSymbol*> pp_all_attribute_groups;

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
    * The modules in the model
    *
    * Populated after parsing is complete.
    */
    static list<ModuleSymbol*> pp_all_modules;

    /**
    * The event names in the model
    *
    * Note that the set contains no duplicates, but event names can be duplicates in different entities.
    * Populated after parsing is complete.
    */
    static set<string> pp_all_event_names;

    /**
    * The visible member names in the model
    *
    * Note that the set contains no duplicates, but visible member names can be duplicates in different entities.
    * Populated after parsing is complete.
    */
    static set<string> pp_visible_member_names;

    /**
     * Map of names of all identifiers used in the body of a function defined in model code.
     * 
     * An example entry might be "Person::MortalityEvent" ==> "alive".
     */
    static multimap<string, string> function_body_identifiers;

    /**
     * Map of member function qualified names to all member pointers used in the body of the function.
     *
     * An example entry might be "Person::MortalityEvent" ==> {"lParent","alive"}.
     */
    static multimap<string, pair<string, string>> function_body_pointers;

    /**
     * Map of member function qualified names to all rng streams used in the body of the function.
     *
     * An example entry might be "Person::MortalityEvent" ==> 12.
     */
    static multimap<string, int> function_rng_streams;

    /**
     * Map of member function qualified names to all rng Normal streams used in the body of the function.
     *
     * An example entry might be "Person::MortalityEvent" ==> 12.
     */
    static multimap<string, int> function_rng_normal_streams;

    /**
     * Size of an array whose index is a random number stream in this model.
     */
    static int size_streams;

    /**
     * Map of member function qualified names to the parameter list of each (as a vector of strings).
     * 
     * Examples:
     *
     * "Person::timeMortality" ==> <empty>
     * "Person::timeMoveEvent" ==> int * p_destination
     */
    static map<string, vector<string> > function_parmlist;

    /**
     * Map of member function qualified names to the definition location in model code.
     */
    static map<string, omc::location> function_defn_loc;

    /**
     * Multimap of rng stream to code location of call.
     */
    static multimap<int, string> rng_stream_calls;

    /**
     * Set of global funcs which need a declaration or definition.
     */
    static set<GlobalFuncSymbol*> pp_missing_global_funcs;

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
     * This maps "symbol,language" as a key to the associated {label,location} specified
     * explicitly by a //LABEL statement in the model source code.
     */
    static unordered_map<string, pair<string, omc::location>> explicit_labels;

    /**
     * Map of NAME from symbol name to short name.
     *
     * This maps "symbol" as a key to the associated short {name,location} specified
     * explicitly by a //NAME statement in the model source code, eg.
     *   //NAME MyParam.Dim0 Region
     *   //NAME MyTable.Expr2 avg_income
     */
    static unordered_map<string, pair<string, omc::location>> explicit_names;

    /**
     * Map of EXPR from symbol name to expression (for derived tables).
     *
     * This maps "symbol" as a key to an override SQL expression {expr,location} specified
     * explicitly by a //EXPR statement in the model source code, eg.
     *   //EXPR MyTable.Expr2 OM_AVG(acc2)
     */
    static unordered_map<string, pair<string, omc::location>> explicit_exprs;

    /**
     * Map of NOTE from symbol name to note (model source)
     * 
     * This maps "symbol,language" as a key to the associated {note,location} specified
     * explicitly by a NOTE statement in the model source code.
     */
    static unordered_map<string, pair<string, omc::location>> notes_source;

    /**
     * Map of NOTE from symbol name to note (parameter input values)
     * 
     * This maps "symbol,language" as a key to the associated {note,location} specified
     * explicitly by a NOTE statement in the model source code.
     */
    static unordered_map<string, pair<string, omc::location>> notes_input;

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
     * The set of om outer keywords.
     * 
     * This is a fixed set independent of any model.  During parsing these keywords cause a
     * transition from C++ code to om declarative code islands.  There is an exact one-to-one
     * relationship between this set and the first block of entries in the list @a token_string.
     *
     * @return The om outer keywords.
     */
    static unordered_set<token_type, std::hash<int> > om_outer_keywords;

    /**
     * The set of om developer functions.
     * 
     * This is a fixed set independent of any model.  These are C++ functions with fixed names
     * supplied by the om developer in the model source code.  An example is the 'Simulation'
     * function.
     */
    static unordered_set<string> om_developer_functions;

    /**
     * The set of om random number functions.
     *
     * This is a fixed set independent of any model.  These are C++ functions with fixed names which
     * can be called by the om developer in the model source code.  An example is the 'RandUniform'
     * function.
     */
    static unordered_set<string> om_rng_functions;

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
     * The C type of the mutable type 'counter'
     */
    static token_type counter_ctype;

    /**
     * The C type of the mutable type 'big_counter'
     */
    static token_type big_counter_ctype;

    /**
     * The C type of the mutable type 'integer'
     */
    static token_type integer_ctype;

    /**
     * The C type of the mutable type 'real'
     */
    static token_type real_ctype;

    /**
     * The C type of the mutable type 'Time'
     */
    static token_type Time_ctype;

    /**
     * If true, parser has encountered a languages statement
     */
    static bool languages_statement_encountered;

    /**
     * If true, model contains one or more hide statements
     */
    static bool any_hide;

    /**
     * If true, model contains one or more show statements
     */
    static bool any_show;

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

    ///  Method to compute run results from subs
    enum run_table_method {

        /// average measures across subs expression_style = sql_accumulators
        average,

        /// aggregate measures across subs expression_style = sql_aggregated_accumulators
        aggregate,

        /// ossemble measures across subs expression_style = sql_assembled_accumulators
        assemble,
    };

    /**
     * The method used to compute run level results from subs.
     */
    static run_table_method measures_method;

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
    static unordered_multimap<string, pair<string, omc::location>> options;

    /**
     * true if event_trace capability is requested in options statement.
     */
    static bool option_event_trace;

    /**
     * true if event_trace microdata warning is enabled in options statement.
     */
    static bool option_event_trace_warning;

    /**
     * true/false depending on bounds_errors in options statement.
     */
    static bool option_bounds_errors;

    /**
     * true/false depending on index_errors in options statement.
     */
    static bool option_index_errors;

    /**
     * true if case_checksum activated in options statement.
     */
    static bool option_case_checksum;

    /**
     * true if checkpoints option is on.
     */
    static bool option_checkpoints;

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
     * true or false depending on verify_timelike_attribute_access value in options statement.
     */
    static bool option_verify_timelike_attribute_access;

    /**
     * true or false depending on verify_valid_table_increment value in options statement.
     */
    static bool option_verify_valid_table_increment;

    /**
     * true or false depending on weighted_tabulation_allow_time_based value in options statement.
     */
    static bool option_weighted_tabulation_allow_time_based;

    /**
     * true or false depending on weighted_tabulation value in options statement.
     */
    static bool option_weighted_tabulation;

    /**
     * true or false depending on resource_use value in options statement.
     */
    static bool option_resource_use;

    /**
     * true or false depending on entity_member_packing value in options statement.
     */
    static bool option_entity_member_packing;

    /**
     * true or false depending on all_attributes_visible value in options statement.
     */
    static bool option_all_attributes_visible;

    /**
     * true or false depending on whether microdata output is enabled.
     */
    static bool option_microdata_output;

    /**
     * true or false depending on whether microdata output runtime warning is emitted.
     */
    static bool option_microdata_output_warning;

    /**
     * true or false depending on whether microdata is written when entity enters the simulation.
     */
    static bool option_microdata_write_on_enter;

    /**
     * true or false depending on whether microdata is written when entity exits the simulation.
     */
    static bool option_microdata_write_on_exit;

    /**
     * true or false depending on whether microdata is written after any event.
     */
    static bool option_microdata_write_on_event;

    /**
     * enumeration size above which microdata attribute is not published.
     */
    static size_t option_microdata_max_enumerators;

    /**
     * true or false depending on use_heuristic_short_names in options statement.
     */
    static bool option_use_heuristic_short_names;

    /**
     * true or false depending on enable_heuristic_names_for_enumerators in options statement.
     */
    static bool option_enable_heuristic_names_for_enumerators;

    /**
     * true or false depending on convert_modgen_note_syntax in options statement.
     */
    static bool option_convert_modgen_note_syntax;

    /**
     * size of the screened1 extremas collections
     */
    static size_t option_screened1_extremas_size;

    /**
     * size of the screened2 extremas collections
     */
    static size_t option_screened2_extremas_size;

    /**
     * size of the screened3 extremas collections
     */
    static size_t option_screened3_extremas_size;

    /**
     * size of the screened4 extremas collections
     */
    static size_t option_screened4_extremas_size;

    /**
     * maximum length of a short name
     */
    static size_t short_name_max_length;

    /**
     * true or false depending on censor_event_time value in options statement.
     */
    static bool option_censor_event_time;

    /**
     * true or false depending on ascii_infinity in options statement.
     */
    static bool option_ascii_infinity;

    /**
     * true to issue warning if enumeration has no label.
     */
    static bool option_missing_label_warning_enumeration;

    /**
     * true to issue warning if parameter has no label.
     */
    static bool option_missing_label_warning_parameter;

    /**
     * true to issue warning if table or table expression has no label.
     */
    static bool option_missing_label_warning_table;

    /**
     * true to issue warning if attribute has no label.
     */
    static bool option_missing_label_warning_attribute;

    /**
     * true to issue warning if published enumeration has no label.
     */
    static bool option_missing_label_warning_published_enumeration;

    /**
     * true to issue warning if published parameter has no label.
     */
    static bool option_missing_label_warning_published_parameter;

    /**
     * true to issue warning if published table or table expression has no label.
     */
    static bool option_missing_label_warning_published_table;

    /**
     * true to issue warning if published attribute has no label.
     */
    static bool option_missing_label_warning_published_attribute;

    /**
     * true to issue warning if published parameter has no note.
     */
    static bool option_missing_note_warning_published_parameter;

    /**
     * true to issue warning if published table has no note.
     */
    static bool option_missing_note_warning_published_table;

    /**
     * true to issue warning if published attribute has no note.
     */
    static bool option_missing_note_warning_published_attribute;

    /**
     * true to issue warning if symbol has untranslated label.
     */
    static bool option_missing_translated_label_warning_any;

    /**
     * true to issue warning if symbol has untranslated note.
     */
    static bool option_missing_translated_note_warning_any;

    /**
     * true to issue warning if published symbol has untranslated label.
     */
    static bool option_missing_translated_label_warning_published_any;

    /**
     * true to issue warning if published symbol has untranslated note.
     */
    static bool option_missing_translated_note_warning_published_any;

    /**
     * true to issue warning if classification level has no explicit short name.
     */
    static bool option_missing_name_warning_classification;

    /**
     * true to issue warning if parameter dimension has no explicit short name.
     */
    static bool option_missing_name_warning_parameter;

    /**
     * true to issue warning if table dimension or expression has no explicit short name.
     */
    static bool option_missing_name_warning_table;

    /**
     * true to issue warning if published classification level has no explicit short name.
     */
    static bool option_missing_name_warning_published_classification;

    /**
     * true to issue warning if published parameter dimension has no explicit short name.
     */
    static bool option_missing_name_warning_published_parameter;

    /**
     * true to issue warning if published table dimension or expression has no explicit short name.
     */
    static bool option_missing_name_warning_published_table;

    /**
     * true to turn on generated symbol reference content in model documentation.
     */
    static bool option_generated_documentation;

    /**
     * true to turn on authored stand-alone content in model documentation.
     */
    static bool option_authored_documentation;

    /**
     * true to turn on most other option_symref_* options.
     */
    static bool option_symref_developer_edition;

    /**
     * false to exclude unpublished symbols in symbol reference.
     */
    static bool option_symref_unpublished_symbols;

    /**
     * true to include in symbol reference the main topic including navigation aid links.
     */
    static bool option_symref_main_topic;

    /**
     * true to include in symbol reference the unique symbol named "model".
     */
    static bool option_symref_model_symbol;

    /**
     * true to include in symbol reference the parameter hierarchy.
     */
    static bool option_symref_parameter_hierarchy;

    /**
     * true to include in symbol reference the table hierarchy.
     */
    static bool option_symref_table_hierarchy;

    /**
     * true to include in symbol reference the parameter major group section.
     */
    static bool option_symref_parameter_major_groups;

    /**
     * true to include in symbol reference the table major group section.
     */
    static bool option_symref_table_major_groups;

    /**
     * true to include in symbol reference the parameter alphabetic list topic.
     */
    static bool option_symref_parameters_alphabetic;

    /**
     * true to include in symbol reference the table alphabetic list topic.
     */
    static bool option_symref_tables_alphabetic;

    /**
     * true to include in symbol reference the enumerations alphabetic list topic.
     */
    static bool option_symref_enumerations_alphabetic;

    /**
     * true to include in symbol reference the attributes alphabetic list topic.
     */
    static bool option_symref_attributes_alphabetic;

    /**
     * true to include in symbol reference individual parameter topics.
     */
    static bool option_symref_topic_parameters;

    /**
     * true to include in symbol reference individual table topics.
     */
    static bool option_symref_topic_tables;

    /**
     * true to include in symbol reference individual attribute topics.
     */
    static bool option_symref_topic_attributes;

    /**
     * true to include in symbol reference individual internal attribute topics.
     */
    static bool option_symref_topic_attributes_internal;

    /**
     * true to include in symbol reference individual enumeration topics.
     */
    static bool option_symref_topic_enumerations;

    /**
     * true to include in symbol reference individual mentity set topics.
     */
    static bool option_symref_topic_entity_sets;

    /**
     * true to include in symbol reference individual module topics.
     */
    static bool option_symref_topic_modules;

    /**
     * true to include in symbol reference individual module topics for use modules.
     */
    static bool option_symref_topic_modules_use;

    /**
     * true to include symbols declared table in individual module topics.
     */
    static bool option_symref_topic_modules_symbols_declared;

    /**
     * true to include in symbol reference any authored NOTEs.
     */
    static bool option_symref_topic_notes;

    /**
     * true to place notes early in an individual symbol topic.
     */
    static bool option_symref_topic_notes_early;

    /**
     * true to show the Note heading in an individual symbol topic
     */
    static bool option_symref_topic_note_heading;

    /**
     * true or false depending on alternate_attribute_dependency_implementation in options statement.
     */
    static bool option_alternate_attribute_dependency_implementation;

    /**
     * name of parameter containing population size.
     */
    static string option_memory_popsize_parameter;

    /**
     * Code page to use for source file conversion
     */
    static string code_page;

    /**
     * True to disable inserted #line directives in generated files
     */
    static bool no_line_directives;

    /**
     * True to suppress the metadata publishing phase of model build
     */
    static bool no_metadata;

    /**
     * True to generate model documentation (end user version)
     */
    static bool model_doc;

    /**
     * The directory containing input authored documentation is used.
     */
    static bool in_doc_active;

    /**
     * The directory containing input authored documentation
     * 
     * Normalized with forward slashes and trailing / if not current directory.
     */
    static string in_doc_dir;

    /**
     * List of stems of .md files in the input authored documentation directory
     */
    static set<string> in_doc_stems_md;

    /**
     * List of stems of .txt files in the input authored documentation directory
     */
    static set<string> in_doc_stems_txt;

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



