/**
* @file    token.cpp
* Implements functions which encapsulate maps related to the bison-generated enum token.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <unordered_map>
#include "token.h"

using namespace std;

// Unfortunately, bison 2.7 with C++ does not expose yytname
// so we need to create hard-coded equivalent.

// Unfortunately, C++11 initializer lists not yet supported in VC11
// Otherwise it would have made sense to initialize one hash like:
//static unordered_map<token_type, const char*> token_string = {
//	{ token::TK_event, "event" },
//	{ token::TK_delta, "delta" },
//};
// Would still have needed initializer logic to create hash in other direction.

/**
* Internal hash used to map from a token to the string representation of that token.
*/
static unordered_map<token_type, string, std::hash<int> > token_string;


/**
* Internal hash used to map from a string to the token associated with that string.
*/
static unordered_map<string, token_type> string_token;

static bool initialization_done = false;

/**
* Initialize the maps used by string_to_token and token_to_string.
*/
static void Initialization()
{
    //TODO write tricky macro used like TOKEN_STRING_EMPLACE( languages ) to create these statements without fear

    // NB: There is an exact one-to-one correspondence with code in parser.y

    // top level om keywords, in alphabetic order
    token_string.emplace(token::TK_agent, "agent");
    token_string.emplace(token::TK_agent_set, "agent_set");
    token_string.emplace(token::TK_aggregation, "aggregation");
    token_string.emplace(token::TK_classification, "classification");
    token_string.emplace(token::TK_counter_type, "counter_type");
    token_string.emplace(token::TK_dependency, "dependency");
    token_string.emplace(token::TK_extend_parameter, "extend_parameter");
    token_string.emplace(token::TK_hide, "hide");
    token_string.emplace(token::TK_import, "import");
    token_string.emplace(token::TK_index_type, "index_type");
    token_string.emplace(token::TK_integer_type, "integer_type");
    token_string.emplace(token::TK_languages, "languages");
    token_string.emplace(token::TK_link, "link");
    token_string.emplace(token::TK_model_generated_parameter_group, "model_generated_parameter_group");
    token_string.emplace(token::TK_model_type, "model_type");
    token_string.emplace(token::TK_options, "options");
    token_string.emplace(token::TK_parameter_group, "parameter_group");
    token_string.emplace(token::TK_parameters, "parameters");
    token_string.emplace(token::TK_partition, "partition");
    token_string.emplace(token::TK_range, "range");
    token_string.emplace(token::TK_real_type, "real_type");
    token_string.emplace(token::TK_string, "string");
    token_string.emplace(token::TK_table, "table");
    token_string.emplace(token::TK_table_group, "table_group");
    token_string.emplace(token::TK_time_type, "time_type");
    token_string.emplace(token::TK_track, "track");
    token_string.emplace(token::TK_user_table, "user_table");
    token_string.emplace(token::TK_version, "version");


    // body level om keywords, in alphabetic order
    token_string.emplace(token::TK_active_spell_delta, "active_spell_delta");
    token_string.emplace(token::TK_active_spell_duration, "active_spell_duration");
    token_string.emplace(token::TK_active_spell_weighted_duration, "active_spell_weighted_duration");
    token_string.emplace(token::TK_agent_id, "agent_id");
    token_string.emplace(token::TK_aggregate, "aggregate");
    token_string.emplace(token::TK_all_base_states, "all_base_states");
    token_string.emplace(token::TK_all_derived_states, "all_derived_states");
    token_string.emplace(token::TK_all_internal_states, "all_internal_states");
    token_string.emplace(token::TK_all_links, "all_links");
    token_string.emplace(token::TK_bounds_errors, "bounds_errors");
    token_string.emplace(token::TK_case_based, "case_based");
    token_string.emplace(token::TK_case_checksum, "case_checksum");
    token_string.emplace(token::TK_cell_based, "cell_based");
    token_string.emplace(token::TK_changes, "changes");
    token_string.emplace(token::TK_completed_spell_delta, "completed_spell_delta");
    token_string.emplace(token::TK_completed_spell_duration, "completed_spell_duration");
    token_string.emplace(token::TK_completed_spell_weighted_duration, "completed_spell_weighted_duration");
    token_string.emplace(token::TK_count, "count");
    token_string.emplace(token::TK_counter, "counter");
    token_string.emplace(token::TK_cumrate, "cumrate");
    token_string.emplace(token::TK_delta, "delta");
    token_string.emplace(token::TK_delta2, "delta2");
    token_string.emplace(token::TK_duration, "duration");
    token_string.emplace(token::TK_duration_counter, "duration_counter");
    token_string.emplace(token::TK_duration_trigger, "duration_trigger");
    token_string.emplace(token::TK_entrances, "entrances");
    token_string.emplace(token::TK_event, "event");
    token_string.emplace(token::TK_event_trace, "event_trace");
    token_string.emplace(token::TK_exits, "exits");
    token_string.emplace(token::TK_file, "file");
    token_string.emplace(token::TK_filter, "filter");
    token_string.emplace(token::TK_fp_consistency, "fp_consistency");
    token_string.emplace(token::TK_haz1rate, "haz1rate");
    token_string.emplace(token::TK_haz2rate, "haz2rate");
    token_string.emplace(token::TK_hook, "hook");
    token_string.emplace(token::TK_IMPLEMENT_HOOK, "IMPLEMENT_HOOK");
    token_string.emplace(token::TK_index, "index");
    token_string.emplace(token::TK_index_errors, "index_errors");
    token_string.emplace(token::TK_integer, "integer");
    token_string.emplace(token::TK_interval, "interval");
    token_string.emplace(token::TK_just_in_time, "just_in_time");
    token_string.emplace(token::TK_max, "max");
    token_string.emplace(token::TK_max_delta, "max_delta");
    token_string.emplace(token::TK_max_over, "max_over");
    token_string.emplace(token::TK_max_value_in, "max_value_in");
    token_string.emplace(token::TK_max_value_out, "max_value_out");
    token_string.emplace(token::TK_min, "min");
    token_string.emplace(token::TK_min_delta, "min_delta");
    token_string.emplace(token::TK_min_over, "min_over");
    token_string.emplace(token::TK_min_value_in, "min_value_in");
    token_string.emplace(token::TK_min_value_out, "min_value_out");
    token_string.emplace(token::TK_model_generated, "model_generated");
    token_string.emplace(token::TK_nz_delta, "nz_delta");
    token_string.emplace(token::TK_nz_value_in, "nz_value_in");
    token_string.emplace(token::TK_nz_value_out, "nz_value_out");
    token_string.emplace(token::TK_off, "off");
    token_string.emplace(token::TK_on, "on");
    token_string.emplace(token::TK_order, "order");
    token_string.emplace(token::TK_packing_level, "packing_level");
    token_string.emplace(token::TK_permit_all_cus, "permit_all_cus");
    token_string.emplace(token::TK_piece_linear, "piece_linear");
    token_string.emplace(token::TK_RandomStream, "RandomStream");
    token_string.emplace(token::TK_rate, "rate");
    token_string.emplace(token::TK_real, "real");
    token_string.emplace(token::TK_self_scheduling_int, "self_scheduling_int");
    token_string.emplace(token::TK_self_scheduling_split, "self_scheduling_split");
    token_string.emplace(token::TK_sparse, "sparse");
    token_string.emplace(token::TK_split, "split");
    token_string.emplace(token::TK_sum, "sum");
    token_string.emplace(token::TK_sum_over, "sum_over");
    token_string.emplace(token::TK_Time, "Time");
    token_string.emplace(token::TK_time_based, "time_based");
    token_string.emplace(token::TK_time_infinite, "time_infinite");
    token_string.emplace(token::TK_time_keeping, "time_keeping");
    token_string.emplace(token::TK_time_undef, "time_undef");
    token_string.emplace(token::TK_transitions, "transitions");
    token_string.emplace(token::TK_trigger_changes, "trigger_changes");
    token_string.emplace(token::TK_trigger_entrances, "trigger_entrances");
    token_string.emplace(token::TK_trigger_exits, "trigger_exits");
    token_string.emplace(token::TK_trigger_transitions, "trigger_transitions");
    token_string.emplace(token::TK_uchar, "uchar");
    token_string.emplace(token::TK_uint, "uint");
    token_string.emplace(token::TK_ulong, "ulong");
    token_string.emplace(token::TK_undergone_change, "undergone_change");
    token_string.emplace(token::TK_undergone_entrance, "undergone_entrance");
    token_string.emplace(token::TK_undergone_exit, "undergone_exit");
    token_string.emplace(token::TK_undergone_transition, "undergone_transition");
    token_string.emplace(token::TK_unit, "unit");
    token_string.emplace(token::TK_ushort, "ushort");
    token_string.emplace(token::TK_value_at_changes, "value_at_changes");
    token_string.emplace(token::TK_value_at_entrances, "value_at_entrances");
    token_string.emplace(token::TK_value_at_exits, "value_at_exits");
    token_string.emplace(token::TK_value_at_first_change, "value_at_first_change");
    token_string.emplace(token::TK_value_at_first_entrance, "value_at_first_entrance");
    token_string.emplace(token::TK_value_at_first_exit, "value_at_first_exit");
    token_string.emplace(token::TK_value_at_first_transition, "value_at_first_transition");
    token_string.emplace(token::TK_value_at_latest_change, "value_at_latest_change");
    token_string.emplace(token::TK_value_at_latest_entrance, "value_at_latest_entrance");
    token_string.emplace(token::TK_value_at_latest_exit, "value_at_latest_exit");
    token_string.emplace(token::TK_value_at_latest_transition, "value_at_latest_transition");
    token_string.emplace(token::TK_value_at_transitions, "value_at_transitions");
    token_string.emplace(token::TK_value_in, "value_in");
    token_string.emplace(token::TK_value_in2, "value_in2");
    token_string.emplace(token::TK_value_out, "value_out");
    token_string.emplace(token::TK_value_out2, "value_out2");
    token_string.emplace(token::TK_weighted_cumulation, "weighted_cumulation");
    token_string.emplace(token::TK_weighted_duration, "weighted_duration");

    // C++ reserved words, in alphabetic order
    // Source: http://en.cppreference.com/w/cpp/keyword
    token_string.emplace(token::TK_alignas, "alignas");
    token_string.emplace(token::TK_alignof, "alignof");
    token_string.emplace(token::TK_and, "and");
    token_string.emplace(token::TK_and_eq, "and_eq");
    token_string.emplace(token::TK_asm, "asm");
    token_string.emplace(token::TK_auto, "auto");
    token_string.emplace(token::TK_bitand, "bitand");
    token_string.emplace(token::TK_bitor, "bitor");
    token_string.emplace(token::TK_bool, "bool");
    token_string.emplace(token::TK_break, "break");
    token_string.emplace(token::TK_case, "case");
    token_string.emplace(token::TK_catch, "catch");
    token_string.emplace(token::TK_char, "char");
    token_string.emplace(token::TK_char16_t, "char16_t");
    token_string.emplace(token::TK_char32_t, "char32_t");
    token_string.emplace(token::TK_class, "class");
    token_string.emplace(token::TK_compl, "compl");
    token_string.emplace(token::TK_const, "const");
    token_string.emplace(token::TK_constexpr, "constexpr");
    token_string.emplace(token::TK_const_cast, "const_cast");
    token_string.emplace(token::TK_continue, "continue");
    token_string.emplace(token::TK_decltype, "decltype");
    token_string.emplace(token::TK_default, "default");
    token_string.emplace(token::TK_delete, "delete");
    token_string.emplace(token::TK_do, "do");
    token_string.emplace(token::TK_double, "double");
    token_string.emplace(token::TK_dynamic_cast, "dynamic_cast");
    token_string.emplace(token::TK_else, "else");
    token_string.emplace(token::TK_enum, "enum");
    token_string.emplace(token::TK_explicit, "explicit");
    token_string.emplace(token::TK_export, "export");
    token_string.emplace(token::TK_extern, "extern");
    token_string.emplace(token::TK_false, "false");
    token_string.emplace(token::TK_float, "float");
    token_string.emplace(token::TK_for, "for");
    token_string.emplace(token::TK_friend, "friend");
    token_string.emplace(token::TK_goto, "goto");
    token_string.emplace(token::TK_if, "if");
    token_string.emplace(token::TK_inline, "inline");
    token_string.emplace(token::TK_int, "int");
    token_string.emplace(token::TK_long, "long");
    token_string.emplace(token::TK_mutable, "mutable");
    token_string.emplace(token::TK_namespace, "namespace");
    token_string.emplace(token::TK_new, "new");
    token_string.emplace(token::TK_noexcept, "noexcept");
    token_string.emplace(token::TK_not, "not");
    token_string.emplace(token::TK_not_eq, "not_eq");
    token_string.emplace(token::TK_nullptr, "nullptr");
    token_string.emplace(token::TK_operator, "operator");
    token_string.emplace(token::TK_or, "or");
    token_string.emplace(token::TK_or_eq, "or_eq");
    token_string.emplace(token::TK_private, "private");
    token_string.emplace(token::TK_protected, "protected");
    token_string.emplace(token::TK_public, "public");
    token_string.emplace(token::TK_register, "register");
    token_string.emplace(token::TK_reinterpret_cast, "reinterpret_cast");
    token_string.emplace(token::TK_return, "return");
    token_string.emplace(token::TK_short, "short");
    token_string.emplace(token::TK_signed, "signed");
    token_string.emplace(token::TK_sizeof, "sizeof");
    token_string.emplace(token::TK_static, "static");
    token_string.emplace(token::TK_static_assert, "static_assert");
    token_string.emplace(token::TK_static_cast, "static_cast");
    token_string.emplace(token::TK_struct, "struct");
    token_string.emplace(token::TK_switch, "switch");
    token_string.emplace(token::TK_template, "template");
    token_string.emplace(token::TK_this, "this");
    token_string.emplace(token::TK_thread_local, "thread_local");
    token_string.emplace(token::TK_throw, "throw");
    token_string.emplace(token::TK_true, "true");
    token_string.emplace(token::TK_try, "try");
    token_string.emplace(token::TK_typedef, "typedef");
    token_string.emplace(token::TK_typeid, "typeid");
    token_string.emplace(token::TK_typename, "typename");
    token_string.emplace(token::TK_union, "union");
    token_string.emplace(token::TK_unsigned, "unsigned");
    token_string.emplace(token::TK_using, "using");
    token_string.emplace(token::TK_virtual, "virtual");
    token_string.emplace(token::TK_void, "void");
    token_string.emplace(token::TK_volatile, "volatile");
    token_string.emplace(token::TK_wchar_t, "wchar_t");
    token_string.emplace(token::TK_while, "while");
    token_string.emplace(token::TK_xor, "xor");
    token_string.emplace(token::TK_xor_eq, "xor_eq");

    // C++ operators & symbols
    // Source: http://en.cppreference.com/w/cpp/keyword

    // assignment
    token_string.emplace(token::TK_ASSIGN, "=");
    token_string.emplace(token::TK_PLUS_ASSIGN, "+=");
    token_string.emplace(token::TK_MINUS_ASSIGN, "-=");
    token_string.emplace(token::TK_TIMES_ASSIGN, "*=");
    token_string.emplace(token::TK_DIV_ASSIGN, "/=");
    token_string.emplace(token::TK_MOD_ASSIGN, "%=");
    token_string.emplace(token::TK_BWAND_ASSIGN, "&=");
    token_string.emplace(token::TK_BWOR_ASSIGN, "|=");
    token_string.emplace(token::TK_BWXOR_ASSIGN, "^=");
    token_string.emplace(token::TK_LSHIFT_ASSIGN, "<<=");
    token_string.emplace(token::TK_RSHIFT_ASSIGN, ">>=");

    // increment / decrement
    token_string.emplace(token::TK_INCREMENT, "++");
    token_string.emplace(token::TK_DECREMENT, "--");

    // arithmetic
    token_string.emplace(token::TK_PLUS, "+");
    token_string.emplace(token::TK_MINUS, "-");
    token_string.emplace(token::TK_DIV, "/");
    token_string.emplace(token::TK_MOD, "%");
    token_string.emplace(token::TK_BW_NOT, "~");
    token_string.emplace(token::TK_BW_OR, "|");
    token_string.emplace(token::TK_BW_XOR, "^");
    token_string.emplace(token::TK_BW_LSHIFT, "<<");
    token_string.emplace(token::TK_BW_RSHIFT, ">>");

    // logical
    token_string.emplace(token::TK_LOGICAL_NOT, "!");
    token_string.emplace(token::TK_LOGICAL_AND, "&&");
    token_string.emplace(token::TK_LOGICAL_OR, "||");

    // comparison
    token_string.emplace(token::TK_EQ, "==");
    token_string.emplace(token::TK_NE, "!=");
    token_string.emplace(token::TK_LT, "<");
    token_string.emplace(token::TK_GT, ">");
    token_string.emplace(token::TK_LE, "<=");
    token_string.emplace(token::TK_GE, ">=");

    // member access
    token_string.emplace(token::TK_MEMBER_OF, ".");
    token_string.emplace(token::TK_MEMBER_OF_POINTER, "->");
    token_string.emplace(token::TK_POINTER_TO_MEMBER, ".*");
    token_string.emplace(token::TK_POINTER_TO_MEMBER_OF_POINTER, "->*");

    // multiple categories
    token_string.emplace(token::TK_STAR, "*");
    token_string.emplace(token::TK_AMPERSAND, "&");

    // other
    token_string.emplace(token::TK_SCOPE_RESOLUTION, "::");
    token_string.emplace(token::TK_QUESTION_MARK, "?");
    token_string.emplace(token::TK_COLON, ":");
    token_string.emplace(token::TK_COMMA, ",");
    token_string.emplace(token::TK_SEMICOLON, ";");

    // braces, brackets, parentheses
    token_string.emplace(token::TK_LEFT_BRACE, "{");
    token_string.emplace(token::TK_RIGHT_BRACE, "}");
    token_string.emplace(token::TK_LEFT_BRACKET, "[");
    token_string.emplace(token::TK_RIGHT_BRACKET, "]");
    token_string.emplace(token::TK_LEFT_PAREN, "(");
    token_string.emplace(token::TK_RIGHT_PAREN, ")");


    token_string.emplace(token::TK_error, "error");



    // initialize hash in other direction
    for (auto i : token_string) string_token.emplace(i.second, i.first);

    // Token synonyms follow.
    // These are unpreferred terms,
    // and are only in the map string->token, not in the map token->string.
    // The scanner will map each such word to the corresponding preferred token.
    // The map token->string has a unique key and maps the symbol enum to the preferred term.

    string_token.emplace("actor", token::TK_agent);
    string_token.emplace("actor_id", token::TK_agent_id);
    string_token.emplace("actor_set", token::TK_agent_set);
    string_token.emplace("logical", token::TK_bool);
    string_token.emplace("TRUE", token::TK_true);
    string_token.emplace("FALSE", token::TK_false);
    string_token.emplace("NULL", token::TK_nullptr);
    string_token.emplace("TIME", token::TK_Time);
    string_token.emplace("TIME_INFINITE", token::TK_time_infinite);
    string_token.emplace("TIME_UNDEF", token::TK_time_undef);

    initialization_done = true;
}

/**
* Return the string corresponding to a token.
*
* @param   e   The token value, e.g. TK_agent
*
* @return  The string representation of the token, e.g. "agent"
*/

const string token_to_string(const token_type& e)
{
    if (!initialization_done) Initialization();
    auto i = token_string.find(e);
    return i->second;
}

/**
* Return the token corresponding to a string.
*
* @param   s   A string with an associated token, e.g. "agent"
*
* @return   The token associated with the string, e.g. token::TK:agent.
*
*           If the string is not a token, the special value token::TK_error is returned.
*/

const token_type string_to_token(const char * s)
{
    if (!initialization_done) Initialization();
    auto i = string_token.find(s);
    if (i == string_token.end()) return token::TK_error;
    else return i->second;
}

/**
* Extract accumulator from Modgen cumulation operator
*
* @param   e   The Modgen cumulation operator, e.g. token::TK_max_value_in
*
* @return  The associated accumulator, e.g. token::TK_max
*/

const token_type modgen_cumulation_operator_to_acc(const token_type& e)
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

/**
* Extract increment from Modgen cumulation operator
*
* @param   e   The Modgen cumulation operator, e.g. token::TK_max_value_in
*
* @return  The associated increment, e.g. token::TK_value_in
*/

const token_type modgen_cumulation_operator_to_incr(const token_type& e)
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
