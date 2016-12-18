/**
 * @file    Symbol.cpp
 * Definitions for the Symbol class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include <limits>
#include <algorithm>
#include <typeinfo>
#include <unordered_map>
#include <set>
#include "location.hh"
#include "libopenm/omLog.h"
#include "libopenm/db/modelBuilder.h"   // for OM_MAX_BUILTIN_TYPE_ID
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
#include "StringTypeSymbol.h"
#include "UnknownTypeSymbol.h"
#include "TypeOfLinkSymbol.h"
#include "ParameterSymbol.h"
#include "EntitySymbol.h"
#include "EntityDataMemberSymbol.h"
#include "EntityDataMemberSymbol.h"
#include "EntityEventSymbol.h"
#include "EntityFuncSymbol.h"
#include "AttributeSymbol.h"
#include "DerivedAttributeSymbol.h"
#include "LinkAttributeSymbol.h"
#include "EntityMultilinkSymbol.h"
#include "IdentityAttributeSymbol.h"
#include "EntityTableMeasureSymbol.h"
#include "EntityTableAccumulatorSymbol.h"
#include "EntityTableMeasureAttributeSymbol.h"
#include "EntitySetSymbol.h"
#include "TableSymbol.h"
#include "EntityTableSymbol.h"
#include "DerivedTableSymbol.h"
#include "GroupSymbol.h"
#include "ParameterGroupSymbol.h"
#include "TableGroupSymbol.h"
#include "HideGroupSymbol.h"
#include "DependencyGroupSymbol.h"
#include "GlobalFuncSymbol.h"
#include "AggregationSymbol.h"
#include "ModelSymbol.h"
#include "ScenarioSymbol.h"

using namespace std;
using namespace openm;

vector<string> Symbol::use_folders;

list<string> Symbol::all_source_files;

symbol_map_type Symbol::symbols;

list<symbol_map_value_type> Symbol::pp_symbols;

unordered_set<string> Symbol::pp_ignore_pass1;

unordered_set<string> Symbol::identifiers_in_model_source;

list<TypeSymbol *> Symbol::pp_all_types0;

list<TypeSymbol *> Symbol::pp_all_types1;

list<EnumerationSymbol *> Symbol::pp_all_enumerations;

list<EnumerationWithEnumeratorsSymbol *> Symbol::pp_all_enumerations_with_enumerators;

list<LanguageSymbol *> Symbol::pp_all_languages;

list<StringSymbol *> Symbol::pp_all_strings;

list<EntitySymbol *> Symbol::pp_all_agents;

list<EntitySetSymbol *> Symbol::pp_all_entity_sets;

list<TableSymbol *> Symbol::pp_all_tables;

list<EntityTableSymbol *> Symbol::pp_all_entity_tables;

list<DerivedTableSymbol *> Symbol::pp_all_derived_tables;

list<ParameterSymbol *> Symbol::pp_all_parameters;

list<ParameterGroupSymbol *> Symbol::pp_all_parameter_groups;

list<TableGroupSymbol *> Symbol::pp_all_table_groups;

list<HideGroupSymbol *> Symbol::pp_all_hide_groups;

list<DependencyGroupSymbol *> Symbol::pp_all_dependency_groups;

list<GlobalFuncSymbol *> Symbol::pp_all_global_funcs;

list<AggregationSymbol *> Symbol::pp_all_aggregations;

multimap<string, string> Symbol::memfunc_bodyids;

map<string, vector<string> > Symbol::memfunc_parmlist;

map<string, yy::location> Symbol::memfunc_defn_loc;

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
    { token::TK_derived_table, "derived_table" },
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
    { token::TK_case_based, "case_based" },
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
    { token::TK_gini, "gini" },
    { token::TK_haz1rate, "haz1rate" },
    { token::TK_haz2rate, "haz2rate" },
    { token::TK_hook, "hook" },
    { token::TK_IMPLEMENT_HOOK, "IMPLEMENT_HOOK" },
    { token::TK_index, "index" },
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
    { token::TK_order, "order" },
    { token::TK_P1, "P1" },
    { token::TK_P2, "P2" },
    { token::TK_P5, "P5" },
    { token::TK_P10, "P10" },
    { token::TK_P20, "P20" },
    { token::TK_P25, "P25" },
    { token::TK_P30, "P30" },
    { token::TK_P40, "P40" },
    { token::TK_P50, "P50" },
    { token::TK_P60, "P60" },
    { token::TK_P70, "P70" },
    { token::TK_P75, "P75" },
    { token::TK_P80, "P80" },
    { token::TK_P90, "P90" },
    { token::TK_P95, "P95" },
    { token::TK_P98, "P98" },
    { token::TK_P99, "P99" },
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
    { token::TK_ELLIPSIS, "..." },

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
    token::TK_derived_table,
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
    { "user_table", token::TK_derived_table },
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

unordered_map<string, string> Symbol::explicit_labels;

unordered_map<string, string> Symbol::notes_source;

unordered_map<string, string> Symbol::notes_input;

unordered_set<string> Symbol::tran_funcs;

unordered_set<string> Symbol::tran_strings;

SpecialGlobal Symbol::pre_simulation("PreSimulation");

SpecialGlobal Symbol::post_simulation("PostSimulation");

SpecialGlobal Symbol::derived_tables("UserTables");

int Symbol::type_changes = 0;

int Symbol::post_parse_errors = 0;

int Symbol::post_parse_warnings = 0;

unordered_map<string, string> Symbol::options;

bool Symbol::option_event_trace = false;

bool Symbol::option_case_checksum = false;

string Symbol::code_page;

bool Symbol::no_line_directives = false;

bool Symbol::trace_parsing = false;

bool Symbol::trace_scanning = false;

bool Symbol::modgen_sort_option = true;

void Symbol::post_parse(int pass)
{
    // This class is the top of the post_parse hierarchical calling chain.
    //super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignLabel:
    {
        // Create default values of symbol labels and notes for all languages
        for (int j = 0; j < LanguageSymbol::number_of_languages(); j++) {
            auto lang_sym = LanguageSymbol::id_to_sym[j];
            auto def_lab = default_label(*lang_sym);
            assert(def_lab.length() > 0);
            pp_labels.push_back(def_lab);
            pp_notes.push_back("");
        }

        // Check for presence of a comment label on the same lines as the symbol declaration.
        if (decl_loc != yy::location()) {
            // This symbol has a declaration location.
            // Check all lines of the declaration for a valid comment label
            int line_count = decl_loc.end.line - decl_loc.begin.line + 1;
            for (int j = 0; j < line_count; ++j) {
                // Construct key for lookup in map of all // comments
                yy::position pos(decl_loc.begin.filename, decl_loc.begin.line + j, 0);
                process_symbol_label(pos);
            }
        }

        // Check for an explicit label specified using //LABEL, for each language
        for (int j = 0; j < LanguageSymbol::number_of_languages(); j++) {
            auto lang_sym = LanguageSymbol::id_to_sym[j];
            string key = unique_name + "," + lang_sym->name;
            auto search = explicit_labels.find(key);
            if (search != explicit_labels.end()) {
                pp_labels[j] = trim(search->second);
            }
        }

        // Check for a note specified using NOTE comment, for each language
        for (int j = 0; j < LanguageSymbol::number_of_languages(); j++) {
            auto lang_sym = LanguageSymbol::id_to_sym[j];
            string key = unique_name + "," + lang_sym->name;
            auto search = notes_source.find(key);
            if (search != notes_source.end()) {
                pp_notes[j] = search->second;
            }
        }

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
                    pp_warning(LT("warning : '") + name + LT("' was never declared"));
                    // OK to continue
                }
            }
            else {
                // I don't think we should get here.  All symbols should be derived symbols at this point.
                // A syntax error should have been detected earlier.
                pp_error(LT("error : unresolved symbol '" )+ name + LT("'"));
                throw HelperException(LT("Finish omc"));
            }
        }
        break;
    }
    case eAssignMembers:
    {
        // Check for presence of a comment label on the *previous* line of the symbol declaration.
        // This is a second pass for associating language labels which are not already associated
        // with a symbol declared on the same line as the label.
        if (decl_loc != yy::location() && decl_loc.begin.line > 0) {
            // This symbol has a declaration location.
            // Construct key for lookup in map of all // comments
            yy::position pos(decl_loc.begin.filename, decl_loc.begin.line - 1, 0);
            process_symbol_label(pos);
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

string Symbol::pretty_name() const
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
    pp_log_message(msg);
    if (post_parse_errors >= max_error_count) {
        string msg = LT("error : ") + to_string(post_parse_errors) + LT(" errors encountered");
        theLog->logFormatted(msg.c_str());
        throw HelperException(LT("error : unsafe to continue, stopping post parse processing"));
    }
}

void Symbol::pp_fatal(const string& msg)
{
    post_parse_errors++;
    pp_log_message(msg);
    throw HelperException(LT("error : unsafe to continue, stopping post parse processing"));
}

void Symbol::pp_warning(const string& msg)
{
    post_parse_warnings++;
    pp_log_message(msg);
}

void Symbol::pp_log_message(const string& msg)
{
    yy::location l = decl_loc;
    if (l.begin.filename) {
        // The symbol has a declaration location
        theLog->logFormatted("%s(%d): %s", l.begin.filename->c_str(), l.begin.line, msg.c_str());
    }
    else {
        // This Symbol has no declaration location, so show symbol name
        string nm = pretty_name();
        theLog->logFormatted("%s in symbol '%s'", msg.c_str(), nm.c_str());
    }
}

bool Symbol::process_symbol_label(const yy::position& pos)
{
    auto cmt_search = cxx_comments.find(pos);
    if (cmt_search != cxx_comments.end()) {
        // There was a // comment on the given line.
        string cmt = cmt_search->second;
        // Extract a possible leading language code.
        string lang_code;
        for (auto ch : cmt) {
            if (isspace(ch)) break;
            lang_code += ch;
        }
        if (lang_code.size() > 0) {
            auto lang_search = LanguageSymbol::name_to_id.find(lang_code);
            if (lang_search != LanguageSymbol::name_to_id.end()) {
                // The comment starts with a valid language code
                // Remove matched label comment from map to allow subsequent
                // identification of dangling labels on line immediately preceding a declaration.
                cxx_comments.erase(cmt_search);
                string lbl = trim(cmt.substr(lang_code.size() + 1));
                if (lbl.length() > 0) {
                    pp_labels[lang_search->second] = lbl;
                }
                else {
                    // ignore empty label
                }
                return true;
            }
        }
    }
    return false;
}

string Symbol::default_label(const LanguageSymbol& language) const
{
    return name;
}


string Symbol::label(const LanguageSymbol& language) const
{
    return pp_labels[language.language_id];
}

string Symbol::label() const
{
    return pp_labels[0];
}

string Symbol::note(const LanguageSymbol & language) const
{
    // placeholder implementation
    //return name + " note (" + language.name + ")";
    return pp_notes[language.language_id];
}

void Symbol::populate_default_symbols(const string &model_name, const string &scenario_name)
{
    Symbol *sym = nullptr;
    sym = new VersionSymbol( 1, 0, 0, 0 );
    assert(sym);
    sym = new ModelTypeSymbol(token::TK_case_based, false);
    assert(sym);
    // ModelSymbol has fixed name "model" for associating label and note
    sym = new ModelSymbol("model", model_name);
    assert(sym);
    // ScenarioSymbol has fixed name "scenario" for associating label and note
    sym = new ScenarioSymbol("scenario", scenario_name);
    assert(sym);


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
    assert(sym);

    // C++ signed integral types
    sym = new NumericSymbol(token::TK_schar, token::TK_signed, token::TK_char, "0");
    assert(sym);
    sym = new NumericSymbol(token::TK_short, "0");
    assert(sym);
    sym = new NumericSymbol(token::TK_int, "0");
    assert(sym);
    sym = new NumericSymbol(token::TK_long, "0");
    assert(sym);
    sym = new NumericSymbol(token::TK_llong, token::TK_long, token::TK_long, "0");
    assert(sym);

    // C++ unsigned integral types (including bool)
    sym = new BoolSymbol();
    assert(sym);
    sym = new NumericSymbol(token::TK_uchar, token::TK_unsigned, token::TK_char, "0");
    assert(sym);
    sym = new NumericSymbol(token::TK_ushort, token::TK_unsigned, token::TK_short, "0");
    assert(sym);
    sym = new NumericSymbol(token::TK_uint, token::TK_unsigned, token::TK_int, "0");
    assert(sym);
    sym = new NumericSymbol(token::TK_ulong, token::TK_unsigned, token::TK_long, "0");
    assert(sym);
    sym = new NumericSymbol(token::TK_ullong, token::TK_unsigned, token::TK_long, token::TK_long, "0");
    assert(sym);

    // C++ floating point types
    sym = new NumericSymbol(token::TK_float, "0.0");
    assert(sym);
    sym = new NumericSymbol(token::TK_double, "0.0");
    assert(sym);
    sym = new NumericSymbol(token::TK_ldouble, token::TK_long, token::TK_double, "0");
    assert(sym);
 
    // Changeable numeric types
    sym = new TimeSymbol(token::TK_double); // Time
    assert(sym);
    sym = new RealSymbol(token::TK_double); // real
    assert(sym);
    sym = new NumericSymbol(token::TK_integer, token::TK_int, "0"); // integer
    assert(sym);
    sym = new NumericSymbol(token::TK_counter, token::TK_int, "0"); // counter
    assert(sym);
    sym = new NumericSymbol(token::TK_big_counter, token::TK_unsigned, token::TK_long, token::TK_long, "0"); // counter
    assert(sym);

    // Other types
    sym = new StringTypeSymbol();
    assert(sym);
    sym = new UnknownTypeSymbol();
    assert(sym);

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
                sym->pp_warning(LT("warning : '") + sym->name + LT("' was never declared"));
                // OK to continue
            }
        }
        else {
            // I don't think we should get here.  All symbols should be derived symbols at this point.
            // A syntax error should have been detected earlier.
            pp_error(sym->decl_loc, LT("error : unresolved symbol '") + sym->name + LT("'"));
            throw HelperException(LT("Finish omc"));
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

void Symbol::modgen_sort_pp_symbols1()
{
    pp_symbols.sort(
        [](symbol_map_value_type a, symbol_map_value_type b)
        {
            return a.second->pp_modgen_name() < b.second->pp_modgen_name();
        }
    );
}

void Symbol::modgen_sort_pp_symbols2()
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
    if (l.begin.filename) {
        theLog->logFormatted("%s(%d): %s", l.begin.filename->c_str(), l.begin.line, msg.c_str());
    }
    else {
        theLog->logFormatted("%s", msg.c_str());
    }
    if (post_parse_errors >= max_error_count) {
        string msg = LT("error : ") + to_string(post_parse_errors) + LT(" errors encountered");
        theLog->logFormatted(msg.c_str());
        throw HelperException(LT("error : unsafe to continue, stopping post parse processing"));
    }
}

// static
void Symbol::pp_logmsg(const yy::location& loc, const string& msg)
{
    yy::location l = loc;
    if (l.begin.filename) {
        theLog->logFormatted("%s(%d): %s", l.begin.filename->c_str(), l.begin.line, msg.c_str());
    }
    else {
        theLog->logFormatted("%s", msg.c_str());
    }
}

void Symbol::post_parse_all()
{
    // Parse all options encountered in model code.
    // This is done before any other post-parse operations
    // to allow options to affect post-parse processing as well as code generation.
    parse_options();

    if (LanguageSymbol::number_of_languages() == 0) {
        pp_error(yy::location(), LT("error : no languages specified"));
    }

    // Create pp_symbols now to easily find Symbols while debugging.
    populate_pp_symbols();

    // pass 1: create additional symbols not created during parse phase
    // symbols will be processed in lexicographical order within sorting group
    for (auto pr : pp_symbols) {
        if (pp_ignore_pass1.count(pr.first) != 0) {
            // This is a freshly morphed pass #1 symbol, so ignore it.
            // Note that pr.second will be invalid for symbols morphed in pass #1.
            continue;
        }
        //theLog->logFormatted("pass #1 %d %s", pr.second->sorting_group, pr.second->unique_name.c_str());
        pr.second->post_parse( eCreateMissingSymbols );
    }

    // Recreate pp_symbols because symbols may have changed or been added.
    populate_pp_symbols();

    // pass 2: assign label using default or from comment on same lines as symbol declaration
    // Symbols will be processed in lexicographical order within sorting group.
    for (auto pr : pp_symbols) {
        pr.second->post_parse( eAssignLabel );
    }

    // pass 3: create pp_ members
    // Symbols will be processed in lexicographical order within sorting group.
    for (auto pr : pp_symbols) {
        pr.second->post_parse( eAssignMembers );
    }

    // pass 4: resolve derived agentvar data types
    // Symbols will be processed in lexicographical order within sorting group.
    type_changes = 1;
    int type_change_passes = 0;
    while (type_changes != 0) {
        type_changes = 0;
        for (auto pr : pp_symbols) {
            pr.second->post_parse( eResolveDataTypes );
        }
        ++type_change_passes;
        if (type_change_passes > 20) {
            theLog->logMsg("error : more than 20 post-parse type change passes.");
            throw HelperException(LT("Finish omc"));
        }
    }

    // pass 5: create pp_ collections
    // Symbols will be processed in lexicographical order within sorting group.
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
    pp_all_agents.sort([](EntitySymbol *a, EntitySymbol *b) { return a->name < b->name; });
    pp_all_parameters.sort( [] (ParameterSymbol *a, ParameterSymbol *b) { return a->name < b->name ; } );
    pp_all_entity_sets.sort( [] (EntitySetSymbol *a, EntitySetSymbol *b) { return a->name < b->name ; } );
    pp_all_tables.sort( [] (TableSymbol *a, TableSymbol *b) { return a->name < b->name ; } );
    pp_all_entity_tables.sort( [] (EntityTableSymbol *a, EntityTableSymbol *b) { return a->name < b->name ; } );
    pp_all_derived_tables.sort( [] (DerivedTableSymbol *a, DerivedTableSymbol *b) { return a->name < b->name ; } );
    pp_all_parameter_groups.sort( [] (ParameterGroupSymbol *a, ParameterGroupSymbol *b) { return a->name < b->name ; } );
    pp_all_table_groups.sort( [] (TableGroupSymbol *a, TableGroupSymbol *b) { return a->name < b->name ; } );
    pp_all_hide_groups.sort( [] (HideGroupSymbol *a, HideGroupSymbol *b) { return a->name < b->name ; } );
    pp_all_dependency_groups.sort( [] (DependencyGroupSymbol *a, DependencyGroupSymbol *b) { return a->name < b->name ; } );
    pp_all_global_funcs.sort( [] (GlobalFuncSymbol *a, GlobalFuncSymbol *b) { return a->name < b->name ; } );
    pp_all_aggregations.sort([](AggregationSymbol *a, AggregationSymbol *b) { return a->name < b->name; });

    // Assign numeric identifiers to symbols in selected collections
    // These numeric id's are used to communicate with the meta-data API.
    {
        int id = 0;
        for (auto parameter : pp_all_parameters) {
            parameter->pp_parameter_id = id;
            ++id;
        }
    }

    {
        int id = 0;
        for (auto table : pp_all_tables) {
            table->pp_table_id = id;
            ++id;
        }
    }

    // Sort collection of enumerators in each enumeration in ordinal order
    for (auto enumeration : pp_all_enumerations_with_enumerators) {
        enumeration->pp_enumerators.sort([](EnumeratorSymbol *a, EnumeratorSymbol *b) { return a->ordinal < b->ordinal; });
    }

    // Sort collections in agents in lexicographic order
    for ( auto agent : pp_all_agents ) {
        agent->pp_agent_data_members.sort( [] (EntityDataMemberSymbol *a, EntityDataMemberSymbol *b) { return a->name < b->name ; } );
        agent->pp_callback_members.sort( [] (EntityMemberSymbol *a, EntityMemberSymbol *b) { return a->name < b->name ; } );
        agent->pp_identity_agentvars.sort( [] (IdentityAttributeSymbol *a, IdentityAttributeSymbol *b) { return a->name < b->name ; } );
        agent->pp_agent_events.sort( [] (EntityEventSymbol *a, EntityEventSymbol *b) { return a->event_name < b->event_name ; } );
        agent->pp_agent_funcs.sort( [] (EntityFuncSymbol *a, EntityFuncSymbol *b) { return a->name < b->name ; } );
        agent->pp_entity_tables.sort( [] (EntityTableSymbol *a, EntityTableSymbol *b) { return a->name < b->name ; } );
        agent->pp_link_agentvars.sort( [] (LinkAttributeSymbol *a, LinkAttributeSymbol *b) { return a->name < b->name ; } );
        agent->pp_multilink_members.sort( [] (EntityMultilinkSymbol *a, EntityMultilinkSymbol *b) { return a->name < b->name ; } );
    }

    // Sort collections in tables
    for ( auto table : pp_all_tables ) {
        // Sort measures in sequence order
        table->pp_measures.sort( [] (TableMeasureSymbol *a, TableMeasureSymbol *b) { return a->index < b->index; } );
    }

    // Sort collections in entity tables
    for ( auto table : pp_all_entity_tables ) {
        // Sort accumulators in sequence order
        table->pp_accumulators.sort( [] (EntityTableAccumulatorSymbol *a, EntityTableAccumulatorSymbol *b) { return a->index < b->index; } );
        // Sort measure attributes in sequence order
        table->pp_measure_attributes.sort( [] (EntityTableMeasureAttributeSymbol *a, EntityTableMeasureAttributeSymbol *b) { return a->index < b->index; } );
    }

    {
        // Create an amalgamated set of event names, in all agents, sorted lexicographically.
        // Note that the set contains no duplicates, but event names can be duplicates in different agents.
        set<string> all_event_names;
        for (auto *agent : pp_all_agents) {
            for (auto *event : agent->pp_agent_events) {
                string str = event->event_name;
                all_event_names.insert(str);
            }
        }

        // For each event in the model, find the index in the sorted list, and assign it as event_id
        for (auto *agent : pp_all_agents) {
            for (auto *event : agent->pp_agent_events) {
                string str = event->event_name;
                auto iter = all_event_names.find(str);
                event->pp_event_id = distance(all_event_names.begin(), iter);
            }
        }
    }

    // For each event in the model, create the modgen-compatible event number for event checksum generation.
    // Modgen event numbers are numbered sequentially starting at 0 within each actor
    // by Modgen event name in lexicographic order.
    // The event which implements self-scheduling events is assigned the last number
    for (auto *agent : pp_all_agents) {
        set<string> agent_event_names;
        for ( auto *event : agent->pp_agent_events ) {
            if (event == agent->ss_event) {
                // do not insert self-scheduling event into the set of event names
                continue;
            }
            string str = event->pp_modgen_name();
            agent_event_names.insert( str );
        }
        for (auto *event : agent->pp_agent_events) {
            if (event == agent->ss_event) {
                // special case for self-scheduling event: assign 1 greater
                // than the highest numbered event
                event->pp_modgen_event_num = agent_event_names.size();
                continue;
            }
            string str = event->pp_modgen_name();
            auto iter = agent_event_names.find(str);
            event->pp_modgen_event_num = distance(agent_event_names.begin(), iter);
        }
    }

    // Optionally sort symbol table so that self-scheduled attributes have identical numbers
    // and identical code injection order between ompp and Modgen.
    if (modgen_sort_option) {
        // This first sort does not respect sorting group.
        // It is used temporarily to reproduce the Modgen self-scheduling
        // numeric identifiers in the for loop below.
        modgen_sort_pp_symbols1();
    }

    // Assign numeric identifier to self-scheduling derived agentvars (used in trace output)
    for (auto pr : pp_symbols) {
        auto sym = pr.second;
        if (auto dav = dynamic_cast<DerivedAttributeSymbol *>(sym)) {
            if (dav->is_self_scheduling()) {
                dav->numeric_id = dav->pp_agent->next_ss_id;
                ++(dav->pp_agent->next_ss_id);
            }
        }
    }

    if (modgen_sort_option) {
        // This second sort restores sorting group
        modgen_sort_pp_symbols2();
    }

    // Pass 6: populate additional collections for subsequent code generation, e.g. for side_effect functions.
    // In this pass, symbols 'reach out' to dependent symbols and populate collections for implementing dependencies.
    // Symbols will be processed in lexicographical order within sorting group.
    for (auto pr : pp_symbols) {
        pr.second->post_parse( ePopulateDependencies );
    }

    // Terminate the event time function body for the self-scheduling event (if present)
    for (auto *agent : pp_all_agents) {
        agent->finish_ss_event();
    }

    // Process PreSimulation, PostSimulation, UserTables

    // For special global functions, sort and check for duplicates
    for (auto sg : {&pre_simulation, &post_simulation, &derived_tables}) {
        bool dups_found = false;
        if (sg->ambiguous_count > 1) {
            dups_found = true;
            post_parse_warnings++;
            string msg = "omc : warning : Multiple " + sg->prefix + " functions with no suffix";
            theLog->logMsg(msg.c_str());
        }
        sg->suffixes.sort(); // lexicographic

        // check for duplicates and suffix validity (numeric, no extraneous leading '0')
        list<int> valid_suffixes;
        for (auto cur = sg->suffixes.begin(); cur != sg->suffixes.end();) {
            auto suffix = *cur;
            assert(suffix.length() > 0);
            auto range = equal_range(cur, sg->suffixes.end(), suffix);
            if (distance(range.first, range.second) > 1) {
                dups_found = true;
                post_parse_errors++;
                string msg = LT("error : duplicate definition of function ") + sg->prefix + suffix;
                theLog->logMsg(msg.c_str());
            }
            if ((suffix[0] == '0' && suffix.length() > 1) || std::string::npos != suffix.find_first_not_of("0123456789")) {
                post_parse_warnings++;
                string msg = LT("warning : unrecognised suffix in ") + sg->prefix + LT(" function ") + sg->prefix + suffix;
                theLog->logMsg(msg.c_str());
            }
            else {
                valid_suffixes.push_back(stoi(suffix));
            }
            // skip any duplicates
            cur = range.second;
        }

        // Recreate the list of suffixes, in numeric order
        valid_suffixes.sort();
        sg->suffixes.clear();
        for (auto id : valid_suffixes) {
            sg->suffixes.push_back(to_string(id));
        }

        // Output helpful list of used suffixes, if dups were found
        if (dups_found) {
            string msg = LT("To disambiguate multiple ") + sg->prefix + LT(" functions, append a numeric suffix");
            theLog->logMsg(msg.c_str());
            if (sg->suffixes.size() > 0) {
                string used;
                for (auto suffix : sg->suffixes) {
                    used += " " + suffix;
                }
                string msg = LT("List of ") + sg->prefix + LT(" suffixes used in model:") + used;
                theLog->logMsg(msg.c_str());
            }
        }
    }
}

void Symbol::parse_options()
{
    {
        string key = "event_trace";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_event_trace = true;
            }
            else if (value == "off") {
                option_event_trace = false;
            }
        }
    }

    {
        string key = "case_checksum";
        auto iter = options.find(key);
        if (iter != options.end()) {
            string value = iter->second;
            if (value == "on") {
                option_case_checksum = true;
            }
            else if (value == "off") {
                option_case_checksum = false;
            }
        }
    }

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

    // The following order picks signed types over unsigned types
    OST_HELPER(long);
    OST_HELPER(int);
    OST_HELPER(short);
    OST_HELPER(char);
    // Windows 64-bit memory model is LLP64, where unsigned long is 32-bit
    // Linux 64-bit memory model is LP64, where unsigned long is 64 bits.
    // So unsigned long is not a storage candidate for 64-bit Linux.
    // Eliminate as a storage candidate for both.
    // (in any case, the context is storage type for classifications, ranges and partitions).
    //OST_HELPER(ulong);
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



