/**
 * @file    Symbol.cpp
 * Definitions for the Symbol class.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <limits>
#include <algorithm>
#include <typeinfo>
#include <unordered_map>
#include <set>
#include "location.hh"
#include "libopenm/omCommon.h"
#include "../libopenm/include/modelSqlBuilder.h" // for OM_MAX_BUILTIN_TYPE_ID
#include "CodeBlock.h"
#include "Symbol.h"
#include "LanguageSymbol.h"
#include "StringSymbol.h"
#include "VersionSymbol.h"
#include "ModelTypeSymbol.h"
#include "NumericSymbol.h"
#include "RealSymbol.h"
#include "TimeSymbol.h"
#include "EnumerationSymbol.h"
#include "EnumeratorSymbol.h"
#include "BoolSymbol.h"
#include "UnknownTypeSymbol.h"
#include "TypeOfLinkSymbol.h"
#include "ParameterSymbol.h"
#include "AgentSymbol.h"
#include "AgentDataMemberSymbol.h"
#include "AgentDataMemberSymbol.h"
#include "AgentEventSymbol.h"
#include "AgentFuncSymbol.h"
#include "AgentVarSymbol.h"
#include "DerivedAgentVarSymbol.h"
#include "LinkAgentVarSymbol.h"
#include "AgentMultilinkSymbol.h"
#include "IdentityAgentVarSymbol.h"
#include "TableExpressionSymbol.h"
#include "TableAccumulatorSymbol.h"
#include "TableAnalysisAgentVarSymbol.h"
#include "EntitySetSymbol.h"
#include "TableSymbol.h"
#include "GroupSymbol.h"
#include "ParameterGroupSymbol.h"
#include "TableGroupSymbol.h"
#include "HideGroupSymbol.h"
#include "DependencyGroupSymbol.h"
#include "GlobalFuncSymbol.h"

using namespace std;
using namespace openm;

string Symbol::use_folder;

list<string> Symbol::all_source_files;

symbol_map_type Symbol::symbols;

list<symbol_map_value_type> Symbol::pp_symbols;

list<TypeSymbol *> Symbol::pp_all_types0;

list<TypeSymbol *> Symbol::pp_all_types1;

list<EnumerationSymbol *> Symbol::pp_all_enumerations;

list<EnumerationWithEnumeratorsSymbol *> Symbol::pp_all_enumerations_with_enumerators;

list<LanguageSymbol *> Symbol::pp_all_languages;

list<StringSymbol *> Symbol::pp_all_strings;

list<AgentSymbol *> Symbol::pp_all_agents;

list<EntitySetSymbol *> Symbol::pp_all_entity_sets;

list<TableSymbol *> Symbol::pp_all_tables;

list<ParameterSymbol *> Symbol::pp_all_parameters;

list<ParameterGroupSymbol *> Symbol::pp_all_parameter_groups;

list<TableGroupSymbol *> Symbol::pp_all_table_groups;

list<HideGroupSymbol *> Symbol::pp_all_hide_groups;

list<DependencyGroupSymbol *> Symbol::pp_all_dependency_groups;

list<GlobalFuncSymbol *> Symbol::pp_all_global_funcs;

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
    { token::TK_aggregation, "aggregation" },
    { token::TK_big_counter_type, "big_counter_type" },
    { token::TK_classification, "classification" },
    { token::TK_counter_type, "counter_type" },
    { token::TK_dependency, "dependency" },
    { token::TK_developer_table, "developer_table" },
    { token::TK_entity, "entity" },
    { token::TK_entity_set, "entity_set" },
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
    { token::TK_use, "use" },
    { token::TK_user_table, "user_table" },
    { token::TK_version, "version" },

    // body level om keywords, in alphabetic order
    { token::TK_active_spell_delta, "active_spell_delta" },
    { token::TK_active_spell_duration, "active_spell_duration" },
    { token::TK_active_spell_weighted_duration, "active_spell_weighted_duration" },
    { token::TK_aggregate, "aggregate" },
    { token::TK_all_base_states, "all_base_states" },
    { token::TK_all_derived_states, "all_derived_states" },
    { token::TK_all_internal_states, "all_internal_states" },
    { token::TK_all_links, "all_links" },
    { token::TK_big_counter, "big_counter" },
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
    { token::TK_ldouble, "ldouble" },
    { token::TK_llong, "llong" },
    { token::TK_maximum, "maximum" },
    { token::TK_max_delta, "max_delta" },
    { token::TK_max_over, "max_over" },
    { token::TK_max_value_in, "max_value_in" },
    { token::TK_max_value_out, "max_value_out" },
    { token::TK_minimum, "minimum" },
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
    { token::TK_schar, "schar" },
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
    { token::TK_ullong, "ullong" },
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
    { token::TK_unused, "unused" },

};

// NB: There is a direct correspondence between the following lines
// and code earlier in this module for the map Symbol::token_string,
// and in parser.y. Maintain exactly the same order.
unordered_set<token_type, std::hash<int> > Symbol::om_outer_keywords =
{
    token::TK_aggregation,
    token::TK_big_counter_type,
    token::TK_classification,
    token::TK_counter_type,
    token::TK_dependency,
    token::TK_developer_table,
    token::TK_entity,
    token::TK_entity_set,
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
    token::TK_use,
    token::TK_user_table,
    token::TK_version,
};

unordered_map<string, token_type> Symbol::string_token =
{
    { "actor", token::TK_entity },
    { "actor_set", token::TK_entity_set },
    { "logical", token::TK_bool },
    { "user_table", token::TK_developer_table },
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

comment_map_type Symbol::cxx_comments;

comment_map_type Symbol::c_comments;

int Symbol::type_changes = 0;

int Symbol::post_parse_errors = 0;

int Symbol::post_parse_warnings = 0;

bool Symbol::option_event_trace = false;

bool Symbol::modgen_sort_option = true;

void Symbol::post_parse(int pass)
{
    // This class is the top of the post_parse hierarchical calling chain.
    //super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        // Integrity check (debugging omc only)
        // A name can be mis-identified as agent context when it should be global.
        // This situation is an intrinsic consequence of the 'distributed declaration' feature of the language.
        if (is_base_symbol()) {
            // This Symbol was never declared.
            if (name != unique_name) {
                // Example: name is "SEX" and unique_name is "Person::SEX".
                // The lexer provisionally assigned a unique name with agent context to this symbol (e.g. "Person::SEX")
                // because no global symbol with that name existed in the symbol table at the time.
                // As it turned out, the source code contained no declared symbol with this name in agent context.
 
                // Search the symbol table for a symbol with the same name in global context.
                auto it_global = symbols.find(name);
                if (it_global != symbols.end()) {
                    // Global symbol table entry with this name exists.
                        
                    // Now get the symbol table entry for this symbol.
                    auto it_this = symbols.find(unique_name);
                    assert(it_this != symbols.end()); // logic guarantee
                    // This is the symbol returned by pp_symbol()
                }
                else {
                    // The source code contained an agent-qualified name token which was never declared,
                    // and no global with the same name exists.
                    pp_error("Error - '" + name + "' was never declared");
                    // OK to continue
                }
            }
            else {
                // I don't think we should get here.  All symbols should be derived symbols at this point.
                // A syntax error should have been detected earlier.
                pp_error("Error - unresolved symbol '" + name + "'");
                throw HelperException("Finish omc");
            }
        }
        break;
    }
    default:
        break;
    }
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

string Symbol::pretty_name()
{
    return name;
}

CodeBlock Symbol::injection_description()
{
    CodeBlock c;
    c += "";
    c += "// Code Injection: group=" + to_string(sorting_group) + ", injector=" + pretty_name();
    return c;
}


void Symbol::pp_error(const string& msg)
{
    post_parse_errors++;
    yy::location l = decl_loc;
    theLog->logFormatted("%s(%d) %s", l.begin.filename->c_str(), l.begin.line, msg.c_str());
}

void Symbol::pp_warning(const string& msg)
{
    post_parse_warnings++;
    yy::location l = decl_loc;
    theLog->logFormatted("%s(%d) %s", l.begin.filename->c_str(), l.begin.line, msg.c_str());
}

string Symbol::label(const LanguageSymbol & language) const
{
    // placeholder implementation
    return name + " [no label (" + language.name + ")]";
}

string Symbol::label() const
{
    // placeholder implementation
    return name + " [no label]";
}

string Symbol::note(const LanguageSymbol & language) const
{
    // placeholder implementation
    return name + " note (" + language.name + ")";
}

void Symbol::populate_default_symbols()
{
    Symbol *sym;
    sym = new VersionSymbol( 1, 0, 0, 0 );
    sym = new ModelTypeSymbol(token::TK_case_based, false);

    // types

    // The call order below creates an ascending type_id member
    // used with the meta-data API. This order can be changed 
    // without breaking existing data in API database, because each model
    // has its own dictionary of types in the database.
    // 
    // For clarity, the order below corresponds to the order in the
    // grammar (parser.y).
    // 
    // N.B. If the number of TypedefTypeSymbols below is changed, a manifest constant
    // in the API must also be changed if it is not big enough.  That manifest constant
    // is used to distinguish model-specific types (classifications, ranges, partitions)
    // in the meta-data API.  An assert() statement at the end of this function will detect
    // if this is necessary, but requires that the compiler be run in debug mode (of course).

    // C++ ambiguous integral type
    // (in C/C++, the signedness of char is not specified)
    sym = new NumericSymbol(token::TK_char, "0");

    // C++ signed integral types
    sym = new NumericSymbol(token::TK_schar, token::TK_signed, token::TK_char, "0");
    sym = new NumericSymbol(token::TK_short, "0");
    sym = new NumericSymbol(token::TK_int, "0");
    sym = new NumericSymbol(token::TK_long, "0");
    sym = new NumericSymbol(token::TK_llong, token::TK_long, token::TK_long, "0");

    // C++ unsigned integral types (including bool)
    sym = new BoolSymbol();
    sym = new NumericSymbol(token::TK_uchar, token::TK_unsigned, token::TK_char, "0");
    sym = new NumericSymbol(token::TK_ushort, token::TK_unsigned, token::TK_short, "0");
    sym = new NumericSymbol(token::TK_uint, token::TK_unsigned, token::TK_int, "0");
    sym = new NumericSymbol(token::TK_ulong, token::TK_unsigned, token::TK_long, "0");
    sym = new NumericSymbol(token::TK_ullong, token::TK_unsigned, token::TK_long, token::TK_long, "0");

    // C++ floating point types
    sym = new NumericSymbol(token::TK_float, "0.0");
    sym = new NumericSymbol(token::TK_double, "0.0");
    sym = new NumericSymbol(token::TK_ldouble, token::TK_long, token::TK_double, "0");
 
    // Changeable numeric types
    sym = new TimeSymbol(token::TK_double); // Time
    sym = new RealSymbol(token::TK_double); // real
    sym = new NumericSymbol(token::TK_integer, token::TK_int, "0"); // integer
    sym = new NumericSymbol(token::TK_counter, token::TK_int, "0"); // counter
    sym = new NumericSymbol(token::TK_big_counter, token::TK_unsigned, token::TK_long, token::TK_long, "0"); // counter
    sym = new UnknownTypeSymbol();

    // Not implemented (a string)
    //sym = new NumericSymbol(token::TK_file, "");

    // check that there aren't more built-in types than the API permits.
    assert(OM_MAX_BUILTIN_TYPE_ID >= TypeSymbol::next_type_id - 1);

    // Set type_id counter so that API will correctly detect these as 'enum types'.
    TypeSymbol::next_type_id = OM_MAX_BUILTIN_TYPE_ID + 1;
}

void Symbol::invalidate_symbols()
{
    for (auto pr : symbols) {
        pr.second = nullptr;
    }
}


bool Symbol::exists( const string& unm )
{
    return symbols.count( unm ) == 0 ? false : true;
}

bool Symbol::exists( const string& nm, const Symbol *agent )
{
    string unm = symbol_name( nm, agent );
    return exists( unm );
}

string Symbol::symbol_name( const string& member, const Symbol *agent )
{
    return agent->name + "::" + member;
}


Symbol *Symbol::get_symbol(const string& unm)
{
    Symbol *sym = nullptr;
	auto it = symbols.find( unm );
    if ( it != symbols.end() ) 
        sym = it->second;
    return sym;
}

Symbol *Symbol::get_symbol( const string& member, const Symbol *agent )
{
    string unm = Symbol::symbol_name( member, agent );
    return Symbol::get_symbol(unm);
}

Symbol *Symbol::create_symbol(const string& unm)
{
    Symbol *sym = Symbol::get_symbol(unm);
    if (!sym) sym = new Symbol(unm);
    return sym;
}

Symbol *Symbol::create_symbol( const string& member, const Symbol *agent )
{
    Symbol *sym = Symbol::get_symbol(member, agent);
    if (!sym) sym = new Symbol(member, agent);
    return sym;
}

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

// static
Symbol *Symbol::pp_symbol(Symbol *& rp_sym)
{
    return Symbol::pp_symbol(&rp_sym);
}

// static
Symbol *Symbol::pp_symbol(Symbol ** pp_sym)
{
    assert(pp_sym);
    Symbol *sym = *pp_sym;
    assert(sym);

    // Identify problem  cases where a name was mis-identified as agent context rather than global context.
    // This situation can arise as an instrinsic consequence of the 'distributed declaration' feature of the language.
    if (sym->is_base_symbol()) {
        // This Symbol was never declared, so there's an issue.
        if (sym->name != sym->unique_name) {
            // Example: name is "SEX" and unique_name is "Person::SEX".
            // The lexer provisionally assigned a unique name with agent context to this symbol (e.g. "Person::SEX")
            // because no global symbol with that name existed in the symbol table at the time.
            // As it turned out, the source code contained no declared symbol with this name in agent context.

            // Search the symbol table for a symbol with the same name in global context.
            auto it_global = symbols.find(sym->name);
            if (it_global != symbols.end()) {
                // A global symbol with this name exists in the symbol table.
                // That's the correct symbol so return it.
                sym = it_global->second;
                assert(sym); // logic guarantee
            }
            else {
                // The source code contained an agent-qualified name token which was never declared,
                // and no global with the same name exists.
                pp_error(sym->decl_loc, "Error - '" + sym->name + "' was never declared");
                // OK to continue
            }
        }
        else {
            // I don't think we should get here.  All symbols should be derived symbols at this point.
            // A syntax error should have been detected earlier.
            pp_error(sym->decl_loc, "Error - unresolved symbol '" + sym->name + "'");
            throw HelperException("Finish omc");
        }
    }
    return sym;
}

void Symbol::populate_pp_symbols()
{
    pp_symbols.clear();
    for (auto sym : symbols) {
        pp_symbols.push_back(sym);
    }
    pp_symbols.sort(
        [](symbol_map_value_type a, symbol_map_value_type b)
        {
            return a.second->sorting_group < b.second->sorting_group
                || (a.second->sorting_group == b.second->sorting_group && a.second->unique_name < b.second->unique_name);
        }
    );
}

void Symbol::modgen_sort_pp_symbols()
{
    pp_symbols.sort(
        [](symbol_map_value_type a, symbol_map_value_type b)
        {
            return a.second->sorting_group < b.second->sorting_group
                || (a.second->sorting_group == b.second->sorting_group && a.second->pp_modgen_name() < b.second->pp_modgen_name());
        }
    );
}

// static
void Symbol::pp_error(const yy::location& loc, const string& msg)
{
    post_parse_errors++;
    yy::location l = loc;
    theLog->logFormatted("%s(%d) %s", l.begin.filename->c_str(), l.begin.line, msg.c_str());
}

void Symbol::post_parse_all()
{
    // Create pp_symbols now to easily find Symbols while debugging.
    populate_pp_symbols();

    // pass 1: create additional symbols not created during parse phase
    // symbols will be processed in lexicographical order
    for (auto pr : pp_symbols) {
        pr.second->post_parse( eCreateMissingSymbols );
    }

    // Recreate pp_symbols because symbols may have changed or been added.
    populate_pp_symbols();

    // pass 2: create pp_ members
    // Symbols will be processed in lexicographical order.
    for (auto pr : pp_symbols) {
        pr.second->post_parse( eAssignMembers );
    }

    // pass 3: resolve derived agentvar data types
    // Symbols will be processed in lexicographical order.
    type_changes = 1;
    int type_change_passes = 0;
    while (type_changes != 0) {
        type_changes = 0;
        for (auto pr : pp_symbols) {
            pr.second->post_parse( eResolveDataTypes );
        }
        ++type_change_passes;
        if (type_change_passes > 20) {
            theLog->logMsg("error - More than 20 post-parse type change passes.");
            throw HelperException("Finish omc");
        }
    }

    // pass 4: create pp_ collections
    // Symbols will be processed in lexicographical order.
    for (auto pr : pp_symbols) {
        pr.second->post_parse( ePopulateCollections );
    }

    // invalidate the parse phase symbol table symbols
    invalidate_symbols();

    // Sort all global collections
    pp_all_languages.sort([](LanguageSymbol *a, LanguageSymbol *b) { return a->language_id < b->language_id; });
    pp_all_strings.sort([](StringSymbol *a, StringSymbol *b) { return a->name < b->name; });
    pp_all_types0.sort([](TypeSymbol *a, TypeSymbol *b) { return a->type_id < b->type_id; });
    pp_all_types1.sort([](TypeSymbol *a, TypeSymbol *b) { return a->type_id < b->type_id; });
    pp_all_agents.sort([](AgentSymbol *a, AgentSymbol *b) { return a->name < b->name; });
    pp_all_parameters.sort( [] (ParameterSymbol *a, ParameterSymbol *b) { return a->name < b->name ; } );
    pp_all_entity_sets.sort( [] (EntitySetSymbol *a, EntitySetSymbol *b) { return a->name < b->name ; } );
    pp_all_tables.sort( [] (TableSymbol *a, TableSymbol *b) { return a->name < b->name ; } );
    pp_all_parameter_groups.sort( [] (ParameterGroupSymbol *a, ParameterGroupSymbol *b) { return a->name < b->name ; } );
    pp_all_table_groups.sort( [] (TableGroupSymbol *a, TableGroupSymbol *b) { return a->name < b->name ; } );
    pp_all_hide_groups.sort( [] (HideGroupSymbol *a, HideGroupSymbol *b) { return a->name < b->name ; } );
    pp_all_dependency_groups.sort( [] (DependencyGroupSymbol *a, DependencyGroupSymbol *b) { return a->name < b->name ; } );
    pp_all_global_funcs.sort( [] (GlobalFuncSymbol *a, GlobalFuncSymbol *b) { return a->name < b->name ; } );

    // Assign numeric identifiers to symbols in selected collections
    // These numeric id's are used for communicating with the meta-data API.
    int id = 0;
    for ( auto parameter : pp_all_parameters ) {
        parameter->pp_parameter_id = id;
        ++id;
    }
    id = 0;
    for ( auto table : pp_all_tables ) {
        table->pp_table_id = id;
        ++id;
    }

    // Sort collection of enumerators in each enumeration in ordinal order
    for (auto enumeration : pp_all_enumerations_with_enumerators) {
        enumeration->pp_enumerators.sort([](EnumeratorSymbol *a, EnumeratorSymbol *b) { return a->ordinal < b->ordinal; });
    }

    // Sort collections in agents in lexicographic order
    for ( auto agent : pp_all_agents ) {
        agent->pp_agent_data_members.sort( [] (AgentDataMemberSymbol *a, AgentDataMemberSymbol *b) { return a->name < b->name ; } );
        agent->pp_callback_members.sort( [] (AgentDataMemberSymbol *a, AgentDataMemberSymbol *b) { return a->name < b->name ; } );
        agent->pp_identity_agentvars.sort( [] (IdentityAgentVarSymbol *a, IdentityAgentVarSymbol *b) { return a->name < b->name ; } );
        agent->pp_agent_events.sort( [] (AgentEventSymbol *a, AgentEventSymbol *b) { return a->event_name < b->event_name ; } );
        agent->pp_agent_funcs.sort( [] (AgentFuncSymbol *a, AgentFuncSymbol *b) { return a->name < b->name ; } );
        agent->pp_agent_tables.sort( [] (TableSymbol *a, TableSymbol *b) { return a->name < b->name ; } );
        agent->pp_link_agentvars.sort( [] (LinkAgentVarSymbol *a, LinkAgentVarSymbol *b) { return a->name < b->name ; } );
        agent->pp_multilink_members.sort( [] (AgentMultilinkSymbol *a, AgentMultilinkSymbol *b) { return a->name < b->name ; } );
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
        for ( auto *event : agent->pp_agent_events ) {
            all_event_names.insert( event->event_name );
        }
    }

    // For each event in the model, find the index in the sorted list, and assign it as event_id
    for (auto *agent : pp_all_agents) {
        for (auto *event : agent->pp_agent_events) {
            auto iter = all_event_names.find(event->event_name);
            event->pp_event_id = distance(all_event_names.begin(), iter);
        }
    }

    // Optionally sort symbol table so that self-scheduled attributes have identical numbers
    // and idnetical code injection order between ompp and Modgen.
    if (modgen_sort_option) {
        modgen_sort_pp_symbols();
    }

    // Assign numeric identifier to self-scheduling derived agentvars (used in trace output)
    for (auto pr : pp_symbols) {
        auto sym = pr.second;
        if (auto dav = dynamic_cast<DerivedAgentVarSymbol *>(sym)) {
            if (dav->is_self_scheduling()) {
                dav->numeric_id = dav->pp_agent->next_ss_id;
                ++(dav->pp_agent->next_ss_id);
            }
        }
    }

    // Pass 5: populate additional collections for subsequent code generation, e.g. for side_effect functions.
    // In this pass, symbols 'reach out' to dependent symbols and populate collections for implementing dependencies.
    // Symbols will be processed in lexicographical order.
    for (auto pr : pp_symbols) {
        pr.second->post_parse( ePopulateDependencies );
    }

    // Terminate the event time function body for the self-scheduling event (if present)
    for (auto *agent : pp_all_agents) {
        agent->finish_ss_event();
    }
}

void Symbol::process_cxx_comment(string cmt, yy::location loc)
{
    comment_map_value_type element(loc, cmt);
    cxx_comments.insert(element);
}

void Symbol::process_c_comment(string cmt, yy::location loc)
{
    comment_map_value_type element(loc, cmt);
    c_comments.insert(element);
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


const size_t Symbol::storage_size(token_type tok)
{
    switch (tok) {
    case token::TK_bool:
        return sizeof(bool);
    case token::TK_char:
        return sizeof(char);
    case token::TK_uchar:
        return sizeof(unsigned char);
    case token::TK_int:
        return sizeof(int);
    case token::TK_uint:
        return sizeof(unsigned int);
    case token::TK_long:
        return sizeof(long);
    case token::TK_ulong:
        return sizeof(unsigned long);
    case token::TK_float:
        return sizeof(float);
    case token::TK_double:
        return sizeof(double);
    default:
        // perhaps more complex logic here, for things like TK_Time, TK_counter, TK_big_counter, etc.
        return sizeof(double);
    }
}


/**
 * Helper macro for code in optimized_storage_type()
 *
 * @param TYPE The type.
 */

#define OST_HELPER(TYPE) \
if ( numeric_limits<TYPE>::min() <= min_value \
  && numeric_limits<TYPE>::max() >= max_value \
  && sizeof(TYPE) < best_size) { \
    best_type = token::TK_##TYPE; \
    best_size = sizeof(TYPE); \
}

// static
const token_type Symbol::optimized_storage_type(long long min_value, long long max_value)
{
    // typedef the om single-keyword types so that the above macro works
    // but within function scope to reduce name pollution
    typedef unsigned char uchar;
    typedef unsigned short ushort;
    typedef unsigned int uint;
    typedef unsigned long ulong;

    token_type best_type = token::TK_long;
    size_t best_size = sizeof(long);

    if (min_value >= 0) {
        // start with the largest unsigned type in om
        best_type = token::TK_ulong;
        best_size = sizeof(ulong);
    }
    else {  // min_value < 0
        // start with the largest signed type in om
        best_type = token::TK_long;
        best_size = sizeof(long);
    }

    // Signed typoes are preferred by the following order.
    OST_HELPER(long);
    OST_HELPER(int);
    OST_HELPER(short);
    OST_HELPER(char);
    OST_HELPER(ulong);
    OST_HELPER(uint);
    OST_HELPER(ushort);
    OST_HELPER(uchar);

    return best_type;
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
        result = token::TK_maximum;
        break;
    case token::TK_max_value_in:
        result = token::TK_maximum;
        break;
    case token::TK_max_value_out:
        result = token::TK_maximum;
        break;
    case token::TK_min_delta:
        result = token::TK_minimum;
        break;
    case token::TK_min_value_in:
        result = token::TK_minimum;
        break;
    case token::TK_min_value_out:
        result = token::TK_minimum;
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
        result = token::TK_nz_delta;
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



