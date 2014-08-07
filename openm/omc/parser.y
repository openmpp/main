%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "2.6"
%defines
%define parser_class_name "parser"

// The following code is written to the header file, not the implementation file
%code requires {

// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <list>

using namespace std;

// forward declarations
class Driver;
class ParseContext;
class Literal;
class IntegerLiteral;
class CharacterLiteral;
class FloatingPointLiteral;
class StringLiteral;
class Symbol;
class AgentMemberSymbol;
class ConstantSymbol;
class ExprForAgentVar;
class ExprForTable;

}

// The following code is written to the implementation file, not the header file
%code {
#include <stdarg.h>
#include <cassert>
#include <string>
#include "Driver.h"
#include "ParseContext.h"
#include "ast.h"
#include "libopenm/omCommon.h"
// last item found by flex.  Used in grammar to provide error information
// TODO check and eliminate use
extern char *yytext;

// Helper function to process terminal in table expressions
static ExprForTableAccumulator * table_expr_terminal(Symbol *agentvar, token_type acc, token_type incr, token_type table_op, ParseContext & pc);

}

// The parsing context.

%parse-param { Driver& drv }
%parse-param { ParseContext& pc }

%lex-param   { Driver& drv }
%lex-param   { ParseContext& pc }

%locations

%initial-action
{
    // Initialize location.
    @$.begin.filename = @$.end.filename = drv.pfile;
};

%debug

%error-verbose

%token-table

// Items in the bison symbol stack
// Naming conventions:
// keywords have prefix TK_ followed by keyword
// member of union have prefix val_ folowed by type for native C types, or pval_ for pointers to objects
%union
{
	int                   val_token;
    int                   val_int;
    Literal              *pval_Literal;
    IntegerLiteral       *pval_IntegerLiteral;
    CharacterLiteral     *pval_CharacterLiteral;
    FloatingPointLiteral *pval_FloatingPointLiteral;
    StringLiteral        *pval_StringLiteral;
	Symbol               *pval_Symbol;
	AgentMemberSymbol    *pval_AgentMemberSymbol;
	ConstantSymbol       *pval_ConstantSymbol;
	ExprForAgentVar      *pval_AgentVarExpr;
	ExprForTable         *pval_TableExpr;
    string               *pval_string;
    list<string *>       *pval_string_list;
};

%printer { yyoutput << "token " << Symbol::token_to_string((token_type)$$); } <val_token>
%printer { yyoutput << "literal " << $$->value(); } <pval_Literal>
%printer { yyoutput << "integer literal " << $$->value(); } <pval_IntegerLiteral>
%printer { yyoutput << "character literal " << $$->value(); } <pval_CharacterLiteral>
%printer { yyoutput << "floating point literal " << $$->value(); } <pval_FloatingPointLiteral>
%printer { yyoutput << "string literal " << $$->value(); } <pval_StringLiteral>
%printer { yyoutput << "constant " << $$->is_literal ? $$->literal->value() : (*($$->enumerator))->name; } <pval_ConstantSymbol>
%printer { yyoutput << "symbol " << '"' << $$->name << '"' << " type=" << typeid($$).name(); } <pval_Symbol>
%printer { yyoutput << "table expr "; } <pval_TableExpr>
%printer { yyoutput << "initial value type=" << Symbol::token_to_string($$->associated_token); } <pval_InitialValue>

%destructor { delete $$; } <pval_IntegerLiteral> <pval_CharacterLiteral> <pval_FloatingPointLiteral> <pval_StringLiteral>


// NB: There is an exact one-to-one correspondence with code in Symbol.cpp

// top-level om keywords, in alphabetic order
%token <val_token>    TK_aggregation    "aggregation"
%token <val_token>    TK_big_counter_type "big_counter_type"
%token <val_token>    TK_classification "classification"
%token <val_token>    TK_counter_type   "counter_type"
%token <val_token>    TK_dependency     "dependency"
%token <val_token>    TK_entity         "entity"
%token <val_token>    TK_entity_set     "entity_set"
%token <val_token>    TK_extend_parameter "extend_parameter"
%token <val_token>    TK_hide           "hide"
%token <val_token>    TK_import         "import"
%token <val_token>    TK_index_type     "index_type"
%token <val_token>    TK_integer_type   "integer_type"
%token <val_token>    TK_languages      "languages"
%token <val_token>    TK_link           "link"
%token <val_token>    TK_model_generated_parameter_group    "model_generated_parameter_group"
%token <val_token>    TK_model_type     "model_type"
%token <val_token>    TK_options        "options"
%token <val_token>    TK_parameters     "parameters"
%token <val_token>    TK_parameter_group "parameter_group"
%token <val_token>    TK_partition      "partition"
%token <val_token>    TK_range          "range"
%token <val_token>    TK_real_type      "real_type"
%token <val_token>    TK_string         "string"
%token <val_token>    TK_table          "table"
%token <val_token>    TK_table_group    "table_group"
%token <val_token>    TK_time_type      "time_type"
%token <val_token>    TK_track          "track"
%token <val_token>    TK_use            "use"
%token <val_token>    TK_user_table     "user_table"
%token <val_token>    TK_version        "version"

// body-level om keywords, in alphabetic order
%token <val_token>    TK_active_spell_delta        "active_spell_delta"
%token <val_token>    TK_active_spell_duration     "active_spell_duration"
%token <val_token>    TK_active_spell_weighted_duration  "active_spell_weighted_duration"
%token <val_token>    TK_aggregate                 "aggregate"
%token <val_token>    TK_all_base_states           "all_base_states"
%token <val_token>    TK_all_derived_states        "all_derived_states"
%token <val_token>    TK_all_internal_states       "all_internal_states"
%token <val_token>    TK_all_links                 "all_links"
%token <val_token>    TK_big_counter               "big_counter"
%token <val_token>    TK_bounds_errors             "bounds_errors"
%token <val_token>    TK_case_based                "case_based"
%token <val_token>    TK_case_checksum             "case_checksum"
%token <val_token>    TK_cell_based                "cell_based"
%token <val_token>    TK_changes                   "changes"
%token <val_token>    TK_completed_spell_delta     "completed_spell_delta"
%token <val_token>    TK_completed_spell_duration  "completed_spell_duration"
%token <val_token>    TK_completed_spell_weighted_duration  "completed_spell_weighted_duration"
%token <val_token>    TK_count                     "count"
%token <val_token>    TK_counter                   "counter"
%token <val_token>    TK_cumrate                   "cumrate"
%token <val_token>    TK_delta                     "delta"
%token <val_token>    TK_delta2                    "delta2"
%token <val_token>    TK_duration                  "duration"
%token <val_token>    TK_duration_counter          "duration_counter"
%token <val_token>    TK_duration_trigger          "duration_trigger"
%token <val_token>    TK_entrances                 "entrances"
%token <val_token>    TK_event                     "event"
%token <val_token>    TK_event_trace               "event_trace"
%token <val_token>    TK_exits                     "exits"
%token <val_token>    TK_file                      "file"
%token <val_token>    TK_filter                    "filter"
%token <val_token>    TK_fp_consistency            "fp_consistency"
%token <val_token>    TK_haz1rate                  "haz1rate"
%token <val_token>    TK_haz2rate                  "haz2rate"
%token <val_token>    TK_hook                      "hook"
%token <val_token>    TK_IMPLEMENT_HOOK            "IMPLEMENT_HOOK"
%token <val_token>    TK_index                     "index"
%token <val_token>    TK_index_errors              "index_errors"
%token <val_token>    TK_integer                   "integer"
%token <val_token>    TK_interval                  "interval"
%token <val_token>    TK_just_in_time              "just_in_time"
%token <val_token>    TK_ldouble                   "ldouble"
%token <val_token>    TK_llong                     "llong"
%token <val_token>    TK_max                       "max"
%token <val_token>    TK_max_delta                 "max_delta"
%token <val_token>    TK_max_over                  "max_over"
%token <val_token>    TK_max_value_in              "max_value_in"
%token <val_token>    TK_max_value_out             "max_value_out"
%token <val_token>    TK_min                       "min"
%token <val_token>    TK_min_delta                 "min_delta"
%token <val_token>    TK_min_over                  "min_over"
%token <val_token>    TK_min_value_in              "min_value_in"
%token <val_token>    TK_min_value_out             "min_value_out"
%token <val_token>    TK_model_generated           "model_generated"
%token <val_token>    TK_nz_delta                  "nz_delta"
%token <val_token>    TK_nz_value_in               "nz_value_in"
%token <val_token>    TK_nz_value_out              "nz_value_out"
%token <val_token>    TK_off                       "off"
%token <val_token>    TK_on                        "on"
%token <val_token>    TK_order                     "order"
%token <val_token>    TK_packing_level             "packing_level"
%token <val_token>    TK_permit_all_cus            "permit_all_cus"
%token <val_token>    TK_piece_linear              "piece_linear"
%token <val_token>    TK_RandomStream              "RandomStream"
%token <val_token>    TK_rate                      "rate"
%token <val_token>    TK_real                      "real"
%token <val_token>    TK_schar                     "schar"
%token <val_token>    TK_self_scheduling_int       "self_scheduling_int"
%token <val_token>    TK_self_scheduling_split     "self_scheduling_split"
%token <val_token>    TK_sparse                    "sparse"
%token <val_token>    TK_split                     "split"
%token <val_token>    TK_sum                       "sum"
%token <val_token>    TK_sum_over                  "sum_over"
%token <val_token>    TK_Time                      "Time"
%token <val_token>    TK_time_based                "time_based"
%token <val_token>    TK_time_infinite             "time_infinite"
%token <val_token>    TK_time_keeping              "time_keeping"
%token <val_token>    TK_time_undef                "time_undef"
%token <val_token>    TK_transitions               "transitions"
%token <val_token>    TK_trigger_changes           "trigger_changes"
%token <val_token>    TK_trigger_entrances         "trigger_entrances"
%token <val_token>    TK_trigger_exits             "trigger_exits"
%token <val_token>    TK_trigger_transitions       "trigger_transitions"
%token <val_token>    TK_uchar                     "uchar"
%token <val_token>    TK_uint                      "uint"
%token <val_token>    TK_ullong                    "ullong"
%token <val_token>    TK_ulong                     "ulong"
%token <val_token>    TK_undergone_change          "undergone_change"
%token <val_token>    TK_undergone_entrance        "undergone_entrance"
%token <val_token>    TK_undergone_exit            "undergone_exit"
%token <val_token>    TK_undergone_transition      "undergone_transition"
%token <val_token>    TK_unit                      "unit"
%token <val_token>    TK_ushort                    "ushort"
%token <val_token>    TK_value_at_changes          "value_at_changes"
%token <val_token>    TK_value_at_entrances        "value_at_entrances"
%token <val_token>    TK_value_at_exits            "value_at_exits"
%token <val_token>    TK_value_at_first_change     "value_at_first_change"
%token <val_token>    TK_value_at_first_entrance   "value_at_first_entrance"
%token <val_token>    TK_value_at_first_exit       "value_at_first_exit"
%token <val_token>    TK_value_at_first_transition "value_at_first_transition"
%token <val_token>    TK_value_at_latest_change    "value_at_latest_change"
%token <val_token>    TK_value_at_latest_entrance  "value_at_latest_entrance"
%token <val_token>    TK_value_at_latest_exit      "value_at_latest_exit"
%token <val_token>    TK_value_at_latest_transition "value_at_latest_transition"
%token <val_token>    TK_value_at_transitions       "value_at_transitions"
%token <val_token>    TK_value_in                   "value_in"
%token <val_token>    TK_value_in2                  "value_in2"
%token <val_token>    TK_value_out                  "value_out"
%token <val_token>    TK_value_out2                 "value_out2"
%token <val_token>    TK_weighted_cumulation        "weighted_cumulation"
%token <val_token>    TK_weighted_duration          "weighted_duration"

// C++ keywords, in alphabetic order
// Source: http://en.cppreference.com/w/cpp/keyword
%token <val_token>    TK_alignas       "alignas"
%token <val_token>    TK_alignof       "alignof"
%token <val_token>    TK_and           "and"
%token <val_token>    TK_and_eq        "and_eq"
%token <val_token>    TK_asm           "asm"
%token <val_token>    TK_auto          "auto"
%token <val_token>    TK_bitand        "bitand"
%token <val_token>    TK_bitor         "bitor"
%token <val_token>    TK_bool          "bool"
%token <val_token>    TK_break         "break"
%token <val_token>    TK_case          "case"
%token <val_token>    TK_catch         "catch"
%token <val_token>    TK_char          "char"
%token <val_token>    TK_char16_t      "char16_t"
%token <val_token>    TK_char32_t      "char32_t"
%token <val_token>    TK_class         "class"
%token <val_token>    TK_compl         "compl"
%token <val_token>    TK_const         "const"
%token <val_token>    TK_constexpr     "constexpr"
%token <val_token>    TK_const_cast    "const_cast"
%token <val_token>    TK_continue      "continue"
%token <val_token>    TK_decltype      "decltype"
%token <val_token>    TK_default       "default"
%token <val_token>    TK_delete        "delete"
%token <val_token>    TK_do            "do"
%token <val_token>    TK_double        "double"
%token <val_token>    TK_dynamic_cast  "dynamic_cast"
%token <val_token>    TK_else          "else"
%token <val_token>    TK_enum          "enum"
%token <val_token>    TK_explicit      "explicit"
%token <val_token>    TK_export        "export"
%token <val_token>    TK_extern        "extern"
%token <val_token>    TK_false         "false"
%token <val_token>    TK_float         "float"
%token <val_token>    TK_for           "for"
%token <val_token>    TK_friend        "friend"
%token <val_token>    TK_goto          "goto"
%token <val_token>    TK_if            "if"
%token <val_token>    TK_inline        "inline"
%token <val_token>    TK_int           "int"
%token <val_token>    TK_long          "long"
%token <val_token>    TK_mutable       "mutable"
%token <val_token>    TK_namespace     "namespace"
%token <val_token>    TK_new           "new"
%token <val_token>    TK_noexcept      "noexcept"
%token <val_token>    TK_not           "not"
%token <val_token>    TK_not_eq        "not_eq"
%token <val_token>    TK_nullptr       "nullptr"
%token <val_token>    TK_operator      "operator"
%token <val_token>    TK_or            "or"
%token <val_token>    TK_or_eq         "or_eq"
%token <val_token>    TK_private       "private"
%token <val_token>    TK_protected     "protected"
%token <val_token>    TK_public        "public"
%token <val_token>    TK_register      "register"
%token <val_token>    TK_reinterpret_cast "reinterpret_cast"
%token <val_token>    TK_return        "return"
%token <val_token>    TK_short         "short"
%token <val_token>    TK_signed        "signed"
%token <val_token>    TK_sizeof        "sizeof"
%token <val_token>    TK_static        "static"
%token <val_token>    TK_static_assert "static_assert"
%token <val_token>    TK_static_cast   "static_cast"
%token <val_token>    TK_struct        "struct"
%token <val_token>    TK_switch        "switch"
%token <val_token>    TK_template      "template"
%token <val_token>    TK_this          "this"
%token <val_token>    TK_thread_local  "thread_local"
%token <val_token>    TK_throw         "throw"
%token <val_token>    TK_true          "true"
%token <val_token>    TK_try           "try"
%token <val_token>    TK_typedef       "typedef"
%token <val_token>    TK_typeid        "typeid"
%token <val_token>    TK_typename      "typename"
%token <val_token>    TK_union         "union"
%token <val_token>    TK_unsigned      "unsigned"
%token <val_token>    TK_using         "using"
%token <val_token>    TK_virtual       "virtual"
%token <val_token>    TK_void          "void"
%token <val_token>    TK_volatile      "volatile"
%token <val_token>    TK_wchar_t       "wchar_t"
%token <val_token>    TK_while         "while"
%token <val_token>    TK_xor           "xor"
%token <val_token>    TK_xor_eq        "xor_eq"

// C++ operators
// Source: http://en.cppreference.com/w/cpp/keyword

// assignment
%token <val_token>    TK_ASSIGN        "="
%token <val_token>    TK_PLUS_ASSIGN   "+="
%token <val_token>    TK_MINUS_ASSIGN  "-="
%token <val_token>    TK_TIMES_ASSIGN  "*="
%token <val_token>    TK_DIV_ASSIGN    "/="
%token <val_token>    TK_MOD_ASSIGN    "%="
%token <val_token>    TK_BWAND_ASSIGN  "&="
%token <val_token>    TK_BWOR_ASSIGN   "|="
%token <val_token>    TK_BWXOR_ASSIGN  "^="
%token <val_token>    TK_LSHIFT_ASSIGN "<<="
%token <val_token>    TK_RSHIFT_ASSIGN ">>="

// increment / decrement
%token <val_token>    TK_INCREMENT     "++"
%token <val_token>    TK_DECREMENT     "--"

// arithmetic
%token <val_token>    TK_PLUS          "+"
%token <val_token>    TK_MINUS         "-"
%token <val_token>    TK_DIV           "/"
%token <val_token>    TK_MOD           "%"
%token <val_token>    TK_BW_NOT        "~"
%token <val_token>    TK_BW_OR         "|"
%token <val_token>    TK_BW_XOR        "^"
%token <val_token>    TK_BW_LSHIFT     "<<"
%token <val_token>    TK_BW_RSHIFT     ">>"

// logical
%token <val_token>    TK_LOGICAL_NOT  "!"
%token <val_token>    TK_LOGICAL_AND  "&&"
%token <val_token>    TK_LOGICAL_OR   "||"

// comparison
%token <val_token>    TK_EQ           "=="
%token <val_token>    TK_NE           "!="
%token <val_token>    TK_LT           "<"
%token <val_token>    TK_GT           ">"
%token <val_token>    TK_LE           "<="
%token <val_token>    TK_GE           ">="

// member access
%token <val_token>    TK_MEMBER_OF    "."
%token <val_token>    TK_MEMBER_OF_POINTER "->"
%token <val_token>    TK_POINTER_TO_MEMBER ".*"
%token <val_token>    TK_POINTER_TO_MEMBER_OF_POINTER "->*"

// multiple categories
%token <val_token>    TK_STAR          "*"
%token <val_token>    TK_AMPERSAND     "&"

// other
%token <val_token>    TK_SCOPE_RESOLUTION "::"
%token <val_token>    TK_QUESTION_MARK "?"
%token <val_token>    TK_COLON         ":"
%token <val_token>    TK_COMMA         ","
%token <val_token>    TK_SEMICOLON     ";"

// braces, brackets, parentheses
%token <val_token>    TK_LEFT_BRACE    "{"
%token <val_token>    TK_RIGHT_BRACE   "}"
%token <val_token>    TK_LEFT_BRACKET  "["
%token <val_token>    TK_RIGHT_BRACKET "]"
%token <val_token>    TK_LEFT_PAREN    "("
%token <val_token>    TK_RIGHT_PAREN   ")"


%token <val_token>    TK_error         "error"
%token <val_token>    TK_unused        "unused"

%token <pval_Symbol>               SYMBOL
%token <pval_string>               STRING
%token <pval_Literal>              LITERAL
%token <pval_IntegerLiteral>       INTEGER_LITERAL
%token <pval_CharacterLiteral>     CHARACTER_LITERAL
%token <pval_FloatingPointLiteral> FLOATING_POINT_LITERAL
%token <pval_StringLiteral>        STRING_LITERAL

// operator precedence & associativity
// Source: http://en.cppreference.com/w/cpp/language/operator_precedence
// Note that bison orders in ascending precedence, opposite to the source table
// The comment following each line is the precedence in the source table
%left  ","                // precedence 17
%right  "throw"           // precedence 16
%right  "?" ":" "=" "+=" "-=" "*=" "/=" "%=" "<<=" ">>=" "&=" "^=" "|=" // precedence 15
%left  "||"               // precedence 14
%left  "&&"               // precedence 13
%left  "|"                // precedence 12
%left  "^"                // precedence 11
%left  "&"                // precedence 10
%left  "==" "!="          // precedence 9
%left  "<" "<=" ">" ">="  // precedence 8
%left  "<<" ">>"          // precedence 7
%left  "+" "-"            // precedence 6
%left  "*" "/" "%"        // precedence 5
%left  ".*" "->*"         // precedence 4
%right PREFIX_INCREMENT PREFIX_DECREMENT UNARY_PLUS UNARY_MINUS "!" "~" TYPE_CAST INDIRECTION ADDRESS_OF "sizeof" "new" "delete" // precedence 3
%left  POSTFIX_INCREMENT POSTFIX_DECREMENT FUNCTION_CALL ARRAY_SUBSCRIPTING "." "->"  // precedence 2
%left  "::"              // precedence 1

%type  <val_int>        option_value
%type  <val_token>      option_keyword

%type  <val_token>      model_type
%type  <val_token>      modgen_cumulation_operator
%type  <val_token>      table_accumulator
%type  <val_token>      table_increment
%type  <val_token>      table_operator

%type  <pval_Literal>   bool_literal
%type  <pval_Literal>   numeric_literal
%type  <pval_Literal>   signed_integer_literal
%type  <pval_Literal>   signed_floating_point_literal
%type  <pval_Literal>   signed_numeric_literal
%type  <pval_Literal>   non_numeric_literal
%type  <pval_Literal>   literal
%type  <pval_Literal>   signed_literal

%type  <pval_Symbol>    decl_type_part
%type  <pval_Symbol>    agentvar
%type  <pval_Symbol>    derived_agentvar
%type  <pval_Symbol>    link_to_agentvar
%type  <pval_Literal>   event_priority_opt
%type  <pval_ConstantSymbol>  constant

%type  <pval_AgentMemberSymbol> link_symbol

%type  <pval_TableExpr> expr_for_table
%type  <pval_TableExpr> table_expression_list

%type  <pval_AgentVarExpr> expr_for_agentvar

%type  <val_token> aggregate_multilink_function

%type  <val_token>      numeric_type
%type  <val_token>      changeable_numeric_type
%type  <val_token>      cxx_numeric_type
%type  <val_token>      cxx_signed_integral_type
%type  <val_token>      cxx_unsigned_integral_type
%type  <val_token>      cxx_floating_point_type

%type <pval_string>      parameter_initializer_element
%type <pval_string_list> parameter_initializer_list

%type  <val_token>      uchar_synonym
%type  <val_token>      schar_synonym
%type  <val_token>      short_synonym
%type  <val_token>      ushort_synonym
%type  <val_token>      int_synonym
%type  <val_token>      uint_synonym
%type  <val_token>      long_synonym
%type  <val_token>      ulong_synonym
%type  <val_token>      llong_synonym
%type  <val_token>      ullong_synonym
%type  <val_token>      ldouble_synonym

%token END      0 "end of file"



%%

%start ompp_declarative_islands;

ompp_declarative_islands:
ompp_declarative_islands ompp_declarative_island
	| /* nothing */
	;

ompp_declarative_island:
	  decl_use              { pc.InitializeForCxx(); }
	| decl_languages        { pc.InitializeForCxx(); }
	| decl_string           { pc.InitializeForCxx(); }
	| decl_options          { pc.InitializeForCxx(); }
	| decl_model_type       { pc.InitializeForCxx(); }
	| decl_time_type        { pc.InitializeForCxx(); }
	| decl_real_type        { pc.InitializeForCxx(); }
	| decl_counter_type     { pc.InitializeForCxx(); }
	| decl_big_counter_type { pc.InitializeForCxx(); }
	| decl_integer_type     { pc.InitializeForCxx(); }
	| decl_index_type       { pc.InitializeForCxx(); }
	| decl_version          { pc.InitializeForCxx(); }
    | decl_classification   { pc.InitializeForCxx(); }
    | decl_partition        { pc.InitializeForCxx(); }
    | decl_range            { pc.InitializeForCxx(); }
    | decl_parameters       { pc.InitializeForCxx(); }
	| decl_entity           { pc.InitializeForCxx(); }
	| decl_link             { pc.InitializeForCxx(); }
	| decl_entity_set       { pc.InitializeForCxx(); }
	| decl_table            { pc.InitializeForCxx(); }
	;

/*
 * use
 */

decl_use:
          "use" STRING_LITERAL[path] ";"
                        {
                            string use_file = $path->value();
                            // remove leading and trailing double quote characters
                            use_file = use_file.substr(1, use_file.length() - 2);
                            // prepend the fixed 'use' folder location
                            use_file = Symbol::use_folder + use_file;
                            Symbol::all_source_files.push_back(use_file);
                            delete $path;
                        }
        | "use" error ";"
        ;

/*
 * languages
 */

decl_languages:
          "languages" "{" language_list "}" ";"
        | "languages" "{" error "}" ";"
        | "languages" error ";"
        ;

language_list:
      SYMBOL
                        {
                            // morph existing symbol to LanguageSymbol
                            auto *sym = new LanguageSymbol( $SYMBOL, @SYMBOL );
                        }
	| language_list "," SYMBOL
                        {
                            // morph existing symbol to LanguageSymbol
                            auto *sym = new LanguageSymbol( $SYMBOL, @SYMBOL );
                        }
	;

/*
 * string
 */

decl_string:
          "string" SYMBOL[string] ";"
                        {
                            // morph existing symbol to StringSymbol
                            auto *sym = new StringSymbol( $string, @string );
                        }
        | "string" error ";"
        ;

/*
 * options
 */

decl_options:
      "options" options_list ";"
    | "options" error ";"
    ;

options_list:
      option
	| options_list option
	;

option:
        option_keyword[kw] "=" option_value[value]
                        {
                            // process recognized option / value pair
                            token_type kw = (token_type)$kw;
                            int value = $value;

                            if (kw == token::TK_event_trace) {
                                Symbol::option_event_trace = (value == 1) ? true : false;
                            }
                        }
	;

option_keyword:
      "bounds_errors"
    | "index_errors"
    | "case_checksum"
    | "event_trace"
    | "fp_consistency"
    | "packing_level"
	;

option_value[value]:
      "on"
                        {
                            $value = 1;
                        }
    | "off"
                        {
                            $value = 0;
                        }
    | INTEGER_LITERAL[integer_value]
                        {
                            $value = stoi( $integer_value->value() );
                            delete $integer_value;
                        }
    ;

/*
 * changeable types
 */

decl_time_type:
	  "time_type" cxx_numeric_type[type_to_use] ";"
                        {
                            // Change properties of existing unitary TimeSymbol
                            auto *sym = TimeSymbol::find();
                            assert(sym);  // Initialization guarantee
                            sym->time_type = (token_type)$type_to_use;
                            sym->decl_loc = @$;
                        }
    | "time_type" error ";"
    ;

decl_real_type:
      "real_type" cxx_floating_point_type[type_to_use] ";"
                        {
                            // Change properties of existing NumericSymbol
                            auto *sym = NumericSymbol::find(token::TK_real);
                            assert(sym);  // Initialization guarantee
                            sym->Set_keywords((token_type)$type_to_use);
                            sym->decl_loc = @$;
                        }
    | "real_type" error ";"
    ;

decl_counter_type:
	  "counter_type" cxx_unsigned_integral_type[type_to_use] ";"
						{
                            // Change properties of existing NumericSymbol
                            auto *sym = NumericSymbol::find(token::TK_counter);
                            assert(sym);  // Initialization guarantee
                            sym->Set_keywords((token_type)$type_to_use);
                            sym->decl_loc = @$;
                        }
    | "counter_type" error ";"
    ;

decl_big_counter_type:
	  "big_counter_type" cxx_unsigned_integral_type[type_to_use] ";"
						{
                            // Change properties of existing NumericSymbol
                            auto *sym = NumericSymbol::find(token::TK_big_counter);
                            assert(sym);  // Initialization guarantee
                            sym->Set_keywords((token_type)$type_to_use);
                            sym->decl_loc = @$;
                        }
    | "big_counter_type" error ";"
    ;

decl_integer_type:
      "integer_type" cxx_signed_integral_type[type_to_use] ";"
                        {
                            // Change properties of existing NumericSymbol
                            auto *sym = NumericSymbol::find(token::TK_integer);
                            assert(sym);  // Initialization guarantee
                            sym->Set_keywords((token_type)$type_to_use);
                            sym->decl_loc = @$;
                        }
    | "integer_type" error ";"
    ;

decl_index_type:
      "index_type" cxx_unsigned_integral_type[type_to_use] ";"
                        {
                            // Recognize in grammar but ignore
                        }
    | "index_type" error ";"
    ;

decl_model_type:
          "model_type" model_type[type_to_use] ";"
                        {
                            auto *sym = new ModelTypeSymbol((token_type) $type_to_use, false, @$);
                        }
        | "model_type" model_type[type_to_use] "," "just_in_time" ";"
                        {
                            auto *sym = new ModelTypeSymbol((token_type)$type_to_use, true, @$);
                        }
        | "model_type" error ";"
      ;

model_type:
	  "case_based"
	| "time_based"
	;

decl_version:
        "version" INTEGER_LITERAL[major] "," INTEGER_LITERAL[minor] "," INTEGER_LITERAL[sub_minor] "," INTEGER_LITERAL[sub_sub_minor] ";"
                        {
                            int major = stoi( $major->value() );
                            delete $major;

                            int minor = stoi($minor->value());
                            delete $minor;

                            int sub_minor = stoi($sub_minor->value());
                            delete $sub_minor;

                            int sub_sub_minor = stoi($sub_sub_minor->value());
                            delete $sub_sub_minor;

                            auto *sym = new VersionSymbol( major, minor, sub_minor, sub_sub_minor, @$ );
                        }
      | "version" error ";"
    ;

/*
 * classification
 */

decl_classification:
      "classification" SYMBOL[classification]
                        {
                            // Morph Symbol to ClassificationSymbol
                            $classification = new ClassificationSymbol( $classification, @classification );
                            // Set classification context for body of classification declaration
                            pc.set_classification_context( $classification );
                            // initialize working counter used for classification levels
                            pc.counter1 = 0;
                        }
            "{" classification_levels "}" ";"
                        {
                            // No valid classification context
                            pc.set_classification_context( nullptr );
                        }
            | "classification" "{" error "}" ";"
            ;

classification_levels:
      SYMBOL
                        {
                            // morph existing symbol to EnumeratorSymbol
                            auto *sym = new EnumeratorSymbol($SYMBOL, pc.get_classification_context(), pc.counter1, @SYMBOL);
                            pc.counter1++;  // counter for classification levels
                        }
      | classification_levels "," SYMBOL
                        {
                            // morph existing symbol to EnumeratorSymbol
                            auto *sym = new EnumeratorSymbol($SYMBOL, pc.get_classification_context(), pc.counter1, @SYMBOL);
                            pc.counter1++;  // counter for classification levels
                        }
	;

/*
 * partition
 */

decl_partition:
      "partition" SYMBOL[partition]
                        {
                            // Morph Symbol to PartitionSymbol
                            $partition = new PartitionSymbol( $partition, @partition );
                            // Set partition context for body of partition declaration
                            pc.set_partition_context($partition);
                            // initialize working counter used for partition split points
                            pc.counter1 = 0;
                        }
            "{" partition_splitpoints "}"[last] ";"
                        {
                            // create PartitionEnumeratorSymbol for upper partition interval
                            Symbol *enum_symbol = pc.get_partition_context();
                            string enumerator_name = enum_symbol->name + "_om_" + to_string(pc.counter1);
                            string upper_split_point = "max";
                            auto *sym = new PartitionEnumeratorSymbol(enumerator_name, enum_symbol, pc.counter1, upper_split_point, @last);

                            // No valid partition context
                            pc.set_partition_context( nullptr );
                        }
            | "partition" "{" error "}" ";"
            ;

partition_splitpoints:
      signed_numeric_literal
                        {
                            // create PartitionEnumeratorSymbol for interval which ends at this split point
                            Symbol *enum_symbol = pc.get_partition_context();
                            string enumerator_name = enum_symbol->name + "_om_" + to_string(pc.counter1);
                            string upper_split_point = $signed_numeric_literal->value();
                            auto *sym = new PartitionEnumeratorSymbol(enumerator_name, enum_symbol, pc.counter1, upper_split_point, @signed_numeric_literal);
                            pc.counter1++;  // counter for partition split points
                        }
      | partition_splitpoints "," signed_numeric_literal
                        {
                            // create PartitionEnumeratorSymbol for interval which ends at this split point
                            Symbol *enum_symbol = pc.get_partition_context();
                            string enumerator_name = enum_symbol->name + "_om_" + to_string(pc.counter1);
                            string upper_split_point = $signed_numeric_literal->value();
                            auto *sym = new PartitionEnumeratorSymbol(enumerator_name, enum_symbol, pc.counter1, upper_split_point, @signed_numeric_literal);
                            pc.counter1++;  // counter for partition split points
                        }
	;

/*
 * range
 */

decl_range:
      "range" SYMBOL[range] "{" signed_integer_literal[lower_bound] "," signed_integer_literal[upper_bound] "}" ";"
                        {
                            int lower_bound = stoi( $lower_bound->value() );
                            delete $lower_bound;

                            int upper_bound = stoi($upper_bound->value());
                            delete $upper_bound;

                            // Morph Symbol to RangeSymbol
                            $range = new RangeSymbol( $range, lower_bound, upper_bound, @range );
                        }
      | "range" "{" error "}" ";"
    ;

/*
 * parameter
 */

decl_parameters:
	  "parameters" "{" parameter_list "}" ";"
	| "parameters" "{" error "}" ";"
      ;

parameter_list: 
	  decl_parameter
	| parameter_list decl_parameter
	;

decl_parameter:
      decl_type_part[type_symbol] SYMBOL[parm]
                        {
                            ParameterSymbol *parm = nullptr;

                            if ($parm->is_base_symbol()) {
                                // parameter declaration
                                pc.redeclaration = false;
                                // Morph Symbol to ParameterSymbol
                                parm = new ParameterSymbol( $parm, $type_symbol, @parm );
                                assert(parm);
                                $parm = parm;
                            }
                            else {
                                // parameter re-declaration
                                pc.redeclaration = true;
                                parm = dynamic_cast<ParameterSymbol *>($parm);
                                assert(parm); // grammar/logic guarantee
                            }
                            // Set parameter context for gathering the dimension specification (if present)
                            // and initializer (if present).
                            pc.set_parameter_context( parm );
                        }
            decl_dim_list parameter_initializer_expr ";"
                        {
                            // No longer in parameter context
                            pc.set_parameter_context( nullptr );
                        }
    |  "cumrate" SYMBOL[parm]
                        {
                            ParameterSymbol *parm = nullptr;

                            if ($parm->is_base_symbol()) {
                                // parameter declaration
                                pc.redeclaration = false;
                                // Morph Symbol to ParameterSymbol
                                auto *type_symbol = Symbol::get_symbol("double");
                                assert(type_symbol); // grammar/initialization guarantee
                                parm = new ParameterSymbol( $parm, type_symbol, @parm );
                                assert(parm);
                                $parm = parm;
                            }
                            else {
                                // parameter re-declaration
                                pc.redeclaration = true;
                                parm = dynamic_cast<ParameterSymbol *>($parm);
                                assert(parm); // grammar/logic guarantee
                            }
                            // Set parameter context for gathering the dimension specification (if present)
                            // and initializer (if present).
                            pc.set_parameter_context( parm );
                        }
            decl_dim_list parameter_initializer_expr ";"
                        {
                            // No longer in parameter context
                            pc.set_parameter_context( nullptr );
                        }
    | error ";"
                        {
                            // Error recovery: Prepare to parse another parameter in the 'parameters' declarative island
                            pc.brace_level = 1;
                            pc.parenthesis_level = 0;
                            pc.bracket_level = 0;
                            pc.redeclaration = false;
                            // No valid parameter context
                            pc.set_parameter_context( nullptr );
                        }
	;

decl_dim_list:
      decl_dim_list "[" SYMBOL[dim] "]"
                        {
                            if (!pc.redeclaration) {
                                // add $dim to parameter's dimension_list
                                pc.get_parameter_context()->dimension_list.push_back($dim->stable_pp());
                            }
                            else {
                                // keep track of dimension list of redelcaration for subsequent semantic check
                                pc.get_parameter_context()->dimension_list2.push_back($dim->stable_pp());
                            }
                        }
    | /* Nothing */
    ;

parameter_initializer_expr:
      "=" parameter_initializer_element
                        {
                            // add first (and only) element to the initializer list
                            auto parm = pc.get_parameter_context();
                            assert(parm); // grammar guarantee
                            if (pc.is_scenario_parameter_value) {
                                parm->source = ParameterSymbol::scenario_parameter;
                            }
                            else if (pc.is_fixed_parameter_value) {
                                parm->source = ParameterSymbol::fixed_parameter;
                            }
                            else {
                                // error - parameter value specified in model source
                                assert(false); // model developer error
                            }
                            parm->initializer_list.push_back($parameter_initializer_element);
                        }
    | "=" "{" parameter_initializer_list[wrk] "}"
                        {
                            // splice the gathered initializer list into the parameter's list
                            auto parm = pc.get_parameter_context();
                            assert(parm); // grammar guarantee
                            if (pc.is_scenario_parameter_value) {
                                parm->source = ParameterSymbol::scenario_parameter;
                            }
                            else if (pc.is_fixed_parameter_value) {
                                parm->source = ParameterSymbol::fixed_parameter;
                            }
                            else {
                                // error - parameter value specified in model source
                                assert(false); // model developer error
                            }
                            auto wrk = $wrk; // to see it in the debugger
                            parm->initializer_list.splice(parm->initializer_list.end(), *wrk);
                            delete wrk;
                        }
    | /* Nothing */
    ;


parameter_initializer_list[result]:
    parameter_initializer_element[elem]
                        {
                            // create a new empty string list,
                            auto result = new list<string *>;
                            assert(result);
                            // and add the element to it
                            auto elem = $elem;
                            (*result).push_back(elem);
                            $result = result;
                        }
      | "{" parameter_initializer_list[wrk] "}"
                        {
                            $result = $wrk;
                        }
      // Allow a superfluous trailing comma in comma-separated list of values
      | parameter_initializer_list[wrk] ","
                        {
                            $result = $wrk;
                        }
      // The %prec below makes the bracketed repeater bind tightly to what follows, taking precedence over any following ","
      | "(" INTEGER_LITERAL[rpt] ")" parameter_initializer_list[wrk] %prec UNARY_MINUS
                        {
                            // create a new empty string list,
                            auto result = new list<string *>;
                            assert(result);
                            auto wrk = $wrk;
                            // and append the second list repeatedly to it
                            for (int j = 0; j < stoi($rpt->value()); j++) {
                                (*result).insert((*result).end(), (*wrk).begin(), (*wrk).end());
                            }
                            (*wrk).clear();
                            delete wrk;
                            $result = result;
                        }
      | parameter_initializer_list[wrk1] "," parameter_initializer_list[wrk2]
                        {
                            // splice the two lists together
                            auto wrk1 = $wrk1;
                            auto wrk2 = $wrk2;
                            (*wrk1).splice((*wrk1).end(), *wrk2);
                            delete wrk2;
                            $result = $wrk1;
                        }
    ;

parameter_initializer_element:
      signed_literal
                        {
                            // Create a new string to hold the initializer element
                            // TODO - reuse string storage using pointer
                            string *pstr = new string();
                            // The initializer is the literal, copy it
                            *pstr = $signed_literal->value();
                            // Rule returns a pointer to the string.
                            $parameter_initializer_element = pstr;

                            // delete the Literal object
                            delete $signed_literal;
                        }
    | SYMBOL[enum]
                        {
                            // Create a new string to hold the initializer element
                            // TODO - reuse string storage using pointer?  - not safe!
                            string *pstr = new string();
                            // The initializer is the enum name, copy it
                            *pstr = $enum->name;
                            // Rule returns a pointer to the string.
                            $parameter_initializer_element = pstr;
                        }
    ;


/*
 * entity
 */

decl_entity:
      "entity" SYMBOL[agent]
                        {
                            if ($agent->is_base_symbol()) {
                                // Morph Symbol to AgentSymbol
                                $agent = new AgentSymbol( $agent, @agent );
                            }
                            else {
                                pc.redeclaration = true;
                            }
                            // Set agent context for body of agent declaration
                            pc.set_agent_context( $agent );
                        }
            "{" agent_member_list "}" ";"
                        {
                            // No valid agent context
                            pc.set_agent_context( nullptr );
                        }
            | "agent" "{" error "}" ";"
            ;

agent_member_list:
	  agent_member
	| agent_member_list agent_member
	;

agent_member:
	  decl_simple_agentvar
	| decl_expr_agentvar
	| decl_agent_function
	| decl_agent_event
    | error ";"
                        {
                            // Error recovery: Prepare to parse another member in an agent declarative island
                            pc.brace_level = 1;
                            pc.parenthesis_level = 0;
                            pc.bracket_level = 0;
                        }
    ;

decl_simple_agentvar:
        decl_type_part[type_symbol] SYMBOL[agentvar] ";"
                        {
                            auto *sym = new SimpleAgentVarSymbol( $agentvar, pc.get_agent_context(), $type_symbol, nullptr, @agentvar );
                        }
      | decl_type_part[type_symbol] SYMBOL[agentvar] "=" "{" signed_literal "}" ";"
                        {
                            auto *sym = new SimpleAgentVarSymbol( $agentvar, pc.get_agent_context(), $type_symbol, $signed_literal, @agentvar );
                        }
      | decl_type_part[type_symbol] SYMBOL[agentvar] "=" "{" SYMBOL[enumerator] "}" ";"
                        {
                            auto *sym = new SimpleAgentVarEnumSymbol( $agentvar, pc.get_agent_context(), $type_symbol, $enumerator, @agentvar );
                        }
    ;

decl_expr_agentvar:
        decl_type_part[type_symbol] SYMBOL[agentvar] "=" expr_for_agentvar ";"
                        {
                            auto *sym = new IdentityAgentVarSymbol( $agentvar, pc.get_agent_context(), $type_symbol, $expr_for_agentvar, @agentvar );
                        }
    ;

decl_agent_function:
      "void" SYMBOL "(" ")" ";"
                        {
                            if ($SYMBOL->is_base_symbol()) {
                                // silent suppress redeclaration, esp. of Start() and Finish()
                                // argument 5 suppress_defn=true tells omc that the function definition is developer-supplied
                                auto sym = new AgentFuncSymbol( $SYMBOL, pc.get_agent_context(), "void", "", true, @SYMBOL );
                            }
                        }
    ;

decl_agent_event:
      "event" SYMBOL[time_func] "," SYMBOL[implement_func] event_priority_opt[priority] ";"
                        {
                            int event_priority = stoi($priority->value());
                            delete $priority;
                            auto *agent = pc.get_agent_context();
                            // Create agent event symbol
                            // Ex. "om_time_BirthdayEvent"
                            string event_name = "om_" + $implement_func->name + "_time";
                            auto *sym = new AgentEventSymbol(event_name, agent, $time_func, $implement_func, event_priority, @decl_agent_event);
                        }
    ;

event_priority_opt:
      "," INTEGER_LITERAL
                        {
                            $event_priority_opt = $INTEGER_LITERAL;
                        }
    | "," "time_keeping"
                        {
                            // time keeping priority is just less than internal event for self-scheduling attributes
                            $event_priority_opt = new IntegerLiteral( "254" );
                        }
    | /* nothing */
                        {
                            // default event priority is zero
                            $event_priority_opt = new IntegerLiteral( "0" );
                        }
    ;


/*
 * agentvar expression
 */

expr_for_agentvar[result]:
      agentvar[sym]
                        {
	                        $result = new ExprForAgentVarSymbol( $sym );
                        }
    | literal
                        {
	                        $result = new ExprForAgentVarLiteral( $literal );
                        }
    | "+"[op] expr_for_agentvar[right] %prec UNARY_PLUS
                        {
	                        $result = new ExprForAgentVarUnaryOp( (token_type) $op, $right );
                        }
    | "-"[op] expr_for_agentvar[right] %prec UNARY_MINUS
                        {
	                        $result = new ExprForAgentVarUnaryOp( (token_type) $op, $right );
                        }
    | expr_for_agentvar[left] "+"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "-"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "*"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "/"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "%"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "~"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "|"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "&"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "^"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "<<"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] ">>"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | "!"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarUnaryOp( (token_type) $op, $right );
                        }
    | expr_for_agentvar[left] "&&"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "||"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "=="[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "!="[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "<"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] ">"[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] "<="[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[left] ">="[op] expr_for_agentvar[right]
                        {
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_agentvar[cond] "?" expr_for_agentvar[first] ":" expr_for_agentvar[second] 
                        {
	                        $result = new ExprForAgentVarTernaryOp( $cond, $first, $second );
                        }
    | decl_type_part[type] "("[op] expr_for_agentvar[arg] ")"
                        {
                            // function-style cast
	                        auto type = new ExprForAgentVarSymbol( $type );
	                        $result = new ExprForAgentVarBinaryOp( (token_type) $op, type, $arg );
                        }
    | "(" expr_for_agentvar[expr] ")"
                        {
	                        $result = $expr;
                        }
	;


/*
 * the 'type' part of a parameter or agentvar declaration
 */

decl_type_part:
       numeric_type
                        {
                            // convert token for type to corresponding symbol in symbol table
                            auto *type_symbol = Symbol::get_symbol(Symbol::token_to_string((token_type)$numeric_type));
                            assert(type_symbol); // grammar/initialization guarantee
                            $decl_type_part = type_symbol;
                        }
    | SYMBOL[type_symbol]
                        {
                            $decl_type_part = $type_symbol;
                        }
    ;

/*
 * link
 */

decl_link:
          "link" link_symbol[link] ";"
                        {
                            // reciprocal one-to-one link

                            // The containing agent of the link
                            auto agent = $link->agent;
                            // get the type of the link
                            auto type = TypeOfLinkSymbol::get_single($link->agent);
                            // morph AgentMemberSymbol $link to a LinkAgentVarSymbol
                            auto lavs = new LinkAgentVarSymbol($link, agent, type, $link->decl_loc);
                            lavs->reciprocal_link = lavs; // special case
                        }
        | "link" link_symbol[link1] link_symbol[link2] ";"
                        {
                            // one-to-one link (different agents and/or different link agentvars)

                            // The containing agent of each link
                            auto agent1 = $link1->agent;
                            auto agent2 = $link2->agent;
                            // The type of each link, e.g. the type of link1 will point to agent2, like <link agent2>
                            auto type1 = TypeOfLinkSymbol::get_single(agent2);
                            auto type2 = TypeOfLinkSymbol::get_single(agent1);

                            // morph AgentMemberSymbol's to LinkAgentVarSymbol's
                            auto lavs1 = new LinkAgentVarSymbol($link1, agent1, type1, $link1->decl_loc);
                            auto lavs2 = new LinkAgentVarSymbol($link2, agent2, type2, $link2->decl_loc);

                            lavs1->reciprocal_link = lavs2;
                            lavs2->reciprocal_link = lavs1;
                        }
        | "link" link_symbol[link_single] link_symbol[link_multi] "[" "]" ";"
                        {
                            // one-to-many link
                            // The containing agent of each link
                            auto agent_single = $link_single->agent;
                            auto agent_multi = $link_multi->agent;
                            // The type of each link
                            auto type_single = TypeOfLinkSymbol::get_single(agent_multi);
                            auto type_multi = TypeOfLinkSymbol::get_single(agent_single);

                            // morph AgentMemberSymbol's to appropriate link types
                            auto single = new LinkAgentVarSymbol($link_single, agent_single, type_single, $link_single->decl_loc);
                            auto multi = new AgentMultilinkSymbol($link_multi, agent_multi, type_multi, $link_multi->decl_loc);

                            // match them up
                            single->reciprocal_multilink = multi;
                            multi->reciprocal_link = single;
                        }
        | "link" link_symbol[link_multi] "[" "]" link_symbol[link_single] ";"
                        {
                            // many-to-one link (identical code to previous rule).
                            // The containing agent of each link
                            auto agent_single = $link_single->agent;
                            auto agent_multi = $link_multi->agent;
                            // The type of each link
                            auto type_single = TypeOfLinkSymbol::get_single(agent_multi);
                            auto type_multi = TypeOfLinkSymbol::get_single(agent_single);

                            // morph AgentMemberSymbol's to appropriate link types
                            auto single = new LinkAgentVarSymbol($link_single, agent_single, type_single, $link_single->decl_loc);
                            auto multi = new AgentMultilinkSymbol($link_multi, agent_multi, type_multi, $link_multi->decl_loc);

                            // match them up
                            single->reciprocal_multilink = multi;
                            multi->reciprocal_link = single;
                        }
        | "link" link_symbol[link_multi1] "[" "]" link_symbol[link_multi2] "[" "]" ";"
                        {
                            // many-to-many link
                            // The containing agent of each link
                            auto agent_multi1 = $link_multi1->agent;
                            auto agent_multi2 = $link_multi2->agent;
                            // The type of each link
                            auto type_multi1 = TypeOfLinkSymbol::get_single(agent_multi2);
                            auto type_multi2 = TypeOfLinkSymbol::get_single(agent_multi1);

                            // morph AgentMemberSymbol's to appropriate link types
                            auto multi1 = new AgentMultilinkSymbol($link_multi1, agent_multi1, type_multi1, $link_multi1->decl_loc);
                            auto multi2 = new AgentMultilinkSymbol($link_multi2, agent_multi2, type_multi2, $link_multi2->decl_loc);

                            // match them up
                            multi1->reciprocal_multilink = multi2;
                            multi2->reciprocal_multilink = multi1;
                        }
        | "link" error ";"
        ;

link_symbol:
            SYMBOL[agent] "."
                        {
                            // Set agent context for scanner creation of the immediately following link agentvar
                            pc.set_agent_context( $agent );
                        }
            SYMBOL[link]
                        {
                            // Morph Symbol to an AgentMemberSymbol, which will be morphed subsequently to a link symbol
                            auto ams = new AgentMemberSymbol( $link, $agent, @link);
                            $link_symbol = ams;
                        }
        ;

/*
 * entity_set
 */

decl_entity_set: // Some code for decl_entity_set and decl_table is nearly identical
      "entity_set" SYMBOL[agent] SYMBOL[entity_set]
                        {
                            EntitySetSymbol *entity_set = nullptr;

                            if ($entity_set->is_base_symbol()) {
                                // Morph Symbol to EntitySetSymbol
                                entity_set = new EntitySetSymbol( $entity_set, $agent, @entity_set );
                                assert(entity_set);
                                $entity_set = entity_set;
                            }
                            else {
                                // re-declaration
                                pc.redeclaration = true;
                                entity_set = dynamic_cast<EntitySetSymbol *>($entity_set);
                                assert(entity_set); // grammar/logic guarantee
                                // TODO Raise error?
                            }
                            // Set agent context and entity set context for body of entity set declaration
                            pc.set_agent_context( $agent );
                            pc.set_entity_set_context( entity_set );
                        }
            entity_set_dimension_list_opt entity_set_filter_opt ";"
    | "entity_set" error ";"
    ;

entity_set_dimension_list_opt:
      entity_set_dimension_list
    | /* nothing */
    ;

entity_set_dimension_list:
      entity_set_dimension
    | entity_set_dimension_list entity_set_dimension
    ;

entity_set_dimension:
      "[" SYMBOL[agentvar] "]"
                        {
                            // add $agentvar to entity set's dimension_list
                            pc.get_entity_set_context()->dimension_list.push_back($agentvar->stable_pp());
                        }
    ;

entity_set_filter_opt:
    "filter" expr_for_agentvar[root]
                        {
                            EntitySetSymbol *entity_set = pc.get_entity_set_context();
                            // create an expression agentvar for the filter
                            auto eav = new IdentityAgentVarSymbol("om_" + entity_set->name + "_filter", entity_set->agent, BoolSymbol::find(), $root, @root);
                            assert(eav); // out of memory check
                            // note expression agentvar in entity set
                            entity_set->filter = eav;
                        }
    | /* nothing */
    ;

/*
 * table
 */

decl_table: // Some code for decl_entity_set and decl_table is nearly identical
      "table" SYMBOL[agent] SYMBOL[table] // Note that the symbol 'table' is not created in agent context
                        {
                            TableSymbol *table = nullptr;

                            if ($table->is_base_symbol()) {
                                // Morph Symbol to TableSymbol
                                table = new TableSymbol( $table, $agent, @table );
                                assert(table);
                                $table = table;
                            }
                            else {
                                // re-declaration
                                pc.redeclaration = true;
                                table = dynamic_cast<TableSymbol *>($table);
                                assert(table); // grammar/logic guarantee
                                // TODO Raise error?
                            }
                            // Set agent context and table context for body of table declaration
                            pc.set_agent_context( $agent );
                            pc.set_table_context( table );
                            // initialize working counter used for table expressions
                            pc.counter1 = 0;
                            // initialize working counter used for table accumulators
                            pc.counter2 = 0;
                            // initialize working counter used for table agentvars
                            pc.counter3 = 0;
                        }
            table_filter_opt
            "{" table_dimension_list "}" ";"
                        {
                            // No valid agent or table context
                            pc.set_table_context( nullptr );
                            pc.set_agent_context( nullptr );
                            // Reset working counters
                            pc.counter1 = 0;
                            pc.counter2 = 0;
                            pc.counter3 = 0;
                        }
    | "table" error ";"
    ;

table_filter_opt:
    "[" expr_for_agentvar[root] "]"
                        {
                            TableSymbol *table = pc.get_table_context();
                            // create an expression agentvar for the filter
                            auto eav = new IdentityAgentVarSymbol("om_" + table->name + "_filter", table->agent, BoolSymbol::find(), $root, @root);
                            assert(eav); // out of memory check
                            // note expression agentvar in table
                            table->filter = eav;
                        }
    | /* nothing */
    ;

table_dimension_list:
    table_dimension
  | table_dimension_list "*" table_dimension
  ;

table_dimension:
    SYMBOL[agentvar]
                        {
                            // add $agentvar to table's dimension_list
                            pc.get_table_context()->dimension_list.push_back($agentvar->stable_pp());
                        }
    | "{" table_expression_list "}"
    ;

table_expression_list:
      expr_for_table[root]
                        {
                            auto sym = new TableExpressionSymbol( pc.get_table_context(), $root, pc.counter1, @root );
                            pc.counter1++;  // counter for expressions
                        }
    | table_expression_list "," expr_for_table[root]
                        {
                            auto sym = new TableExpressionSymbol( pc.get_table_context(), $root, pc.counter1, @root );
                            pc.counter1++;  // counter for expressions
                        }
	;

/*
 * table expression
 */

expr_for_table[result]:
      // Ex. income
      agentvar
                        {
                            Symbol *agentvar = $agentvar;
                            // Defaults are accumulator=sum, increment=delta, table operator=interval
                            token_type acc = token::TK_sum;
                            token_type incr = token::TK_delta;
                            token_type tabop = token::TK_interval;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(agentvar, acc, incr, tabop, pc);
                        }
      // Ex. max_value_in(income)
    | modgen_cumulation_operator "(" agentvar ")"
                        {
                            Symbol *agentvar = $agentvar;
                            // Ex. token::TK_max
                            token_type acc = Symbol::modgen_cumulation_operator_to_acc( (token_type) $modgen_cumulation_operator );
                            // Ex. token::TK_value_in
                            token_type incr = Symbol::modgen_cumulation_operator_to_incr( (token_type) $modgen_cumulation_operator );
                            token_type tabop = token::TK_interval;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(agentvar, acc, incr, tabop, pc);
                        }
      // Ex. sum(delta(income))
    | table_accumulator[acc] "(" table_increment[incr] "(" agentvar ")" ")"
                        {
                            Symbol *agentvar = $agentvar;
                            token_type acc = (token_type) $acc;
                            token_type incr = (token_type) $incr;
                            token_type tabop = token::TK_interval;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(agentvar, acc, incr, tabop, pc);
                        }
      // Ex. event(income)
    | table_operator[tabop] "(" agentvar ")"
                        {
                            Symbol *agentvar = $agentvar;
                            token_type acc = token::TK_sum;
                            token_type incr = token::TK_delta;
                            token_type tabop = (token_type) $tabop;
                            // The following static helper function is defined in the final section of parser.y (below)
                            $result = table_expr_terminal(agentvar, acc, incr, tabop, pc);
                        }
      // Ex. max_value_in(event(income))
    | modgen_cumulation_operator "(" table_operator[tabop] "(" agentvar ")" ")"
                        {
                            Symbol *agentvar = $agentvar;
                            // Ex. token::TK_max
                            token_type acc = Symbol::modgen_cumulation_operator_to_acc( (token_type) $modgen_cumulation_operator );
                            // Ex. token::TK_value_in
                            token_type incr = Symbol::modgen_cumulation_operator_to_incr( (token_type) $modgen_cumulation_operator );
                            token_type tabop = (token_type) $tabop;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(agentvar, acc, incr, tabop, pc);
                        }
      // Ex. sum(delta(event(income)))
    | table_accumulator[acc] "(" table_increment[incr] "(" table_operator[tabop] "(" agentvar ")" ")" ")"
                        {
                            Symbol *agentvar = $agentvar;
                            token_type acc = (token_type) $acc;
                            token_type incr = (token_type) $incr;
                            token_type tabop = (token_type) $tabop;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(agentvar, acc, incr, tabop, pc);
                        }
    | numeric_literal
                        {
	                        $result = new ExprForTableLiteral( $numeric_literal );
                        }
    | expr_for_table[left] "+"[op] expr_for_table[right]
                        {
	                        $result = new ExprForTableOp( (token_type) $op, $left, $right );
                        }
    | expr_for_table[left] "-"[op] expr_for_table[right]
                        {
	                        $result = new ExprForTableOp( (token_type) $op, $left, $right );
                        }
    | expr_for_table[left] "*"[op] expr_for_table[right]
                        {
	                        $result = new ExprForTableOp( (token_type) $op, $left, $right );
                        }
    | expr_for_table[left] "/"[op] expr_for_table[right]
                        {
	                        $result = new ExprForTableOp( (token_type) $op, $left, $right );
                        }
    | "-"[op] expr_for_table[right] %prec UNARY_MINUS
                        {
	                        $result = new ExprForTableOp( (token_type) $op, nullptr, $right );
                        }
    | "+"[op] expr_for_table[right] %prec UNARY_PLUS
                        {
	                        $result = new ExprForTableOp( (token_type) $op, nullptr, $right );
                        }
    | "(" expr_for_table[expr] ")"
                        {
	                        $result = $expr;
                        }
	;

modgen_cumulation_operator:
      TK_delta
    | TK_delta2
    | TK_nz_delta
    | TK_value_in
    | TK_value_in2
    | TK_nz_value_in
    | TK_value_out
    | TK_value_out2
    | TK_nz_value_out
    | TK_max_delta
    | TK_max_value_in
    | TK_max_value_out
    | TK_min_delta
    | TK_min_value_in
    | TK_min_value_out
	;

table_accumulator:
      TK_sum
    | TK_min
    | TK_max
    ;

table_increment:
      TK_delta
    | TK_delta2
    | TK_nz_delta
    | TK_value_in
    | TK_value_in2
    | TK_nz_value_in
    | TK_value_out
    | TK_value_out2
    | TK_nz_value_out
    ;

table_operator:
      TK_interval
    | TK_event
    ;

/*
 * agentvar in an expression
 */

agentvar:
      SYMBOL
    | derived_agentvar
    | link_to_agentvar
	;

link_to_agentvar:
      SYMBOL[link] "->"
                        {
                            // Tell the scanner not to apply agent scope resolution to the following 'word'
                            // in the input stream, but just return a STRING instead.  That's because the agent context
                            // depends on the nature of the symbol on the "->", whose declaration may not yet have been encountered.
                            pc.next_word_is_string = true;
                        }
            STRING[agentvar]
                        {
                            // Create LinkToAgentVarSymbol.  Store the r.h.s of pointer operator as a string for
                            // subsequent post-parse resolution.
                            $link_to_agentvar = LinkToAgentVarSymbol::create_symbol(pc.get_agent_context(), $link, *$agentvar);
                            delete $agentvar; // delete the string created using new in scanner
                        }
        ;

/*
 * derived agentvars
 */

derived_agentvar:
    /*
     * derived agentvars - duration family
     */
      TK_duration[kw] "(" ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, @kw );
                        }
    | TK_duration[kw] "(" SYMBOL[observed] "," constant ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, @kw );
                        }
    | TK_weighted_duration[kw] "(" SYMBOL[weight] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, static_cast<Symbol *>(nullptr), static_cast<ConstantSymbol *>(nullptr), $weight, @kw );
                        }
    | TK_weighted_duration[kw] "(" SYMBOL[observed] "," constant "," SYMBOL[weight] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, $weight, @kw );
                        }
    | TK_weighted_cumulation[kw] "(" SYMBOL[observed] "," SYMBOL[weight] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $weight, @kw );
                        }

    /*
     * derived agentvars - spell family
     */
    | TK_active_spell_duration[kw] "(" SYMBOL[spell] "," constant ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $spell, $constant, @kw );
                        }
    | TK_completed_spell_duration[kw] "(" SYMBOL[spell] "," constant ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $spell, $constant, @kw );
                        }
    | TK_active_spell_weighted_duration[kw] "(" SYMBOL[spell] "," constant "," SYMBOL[weight]  ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $spell, $constant, $weight, @kw );
                        }
    | TK_completed_spell_weighted_duration[kw] "(" SYMBOL[spell] "," constant "," SYMBOL[weight]  ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $spell, $constant, $weight, @kw );
                        }
    | TK_active_spell_delta[kw] "(" SYMBOL[spell] "," constant "," SYMBOL[delta]  ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $spell, $constant, $delta, @kw );
                        }
    | TK_completed_spell_delta[kw] "(" SYMBOL[spell] "," constant "," SYMBOL[delta]  ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $spell, $constant, $delta, @kw );
                        }

    /*
     * derived agentvars - transition occurrence family
     */
    | TK_undergone_entrance[kw] "(" SYMBOL[observed] "," constant ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, @kw );
                        }
    | TK_undergone_exit[kw] "(" SYMBOL[observed] "," constant ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, @kw );
                        }
    | TK_undergone_transition[kw] "(" SYMBOL[observed] "," constant[from] "," constant[to] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $from, $to, @kw );
                        }
    | TK_undergone_change[kw] "(" SYMBOL[observed] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, @kw );
                        }

    /*
     * derived agentvars - transition count family
     */
    | TK_entrances[kw] "(" SYMBOL[observed] "," constant ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, @kw );
                        }
    | TK_exits[kw] "(" SYMBOL[observed] "," constant ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, @kw );
                        }
    | TK_transitions[kw] "(" SYMBOL[observed] "," constant[from] "," constant[to] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $from, $to, @kw );
                        }
    | TK_changes[kw] "(" SYMBOL[observed] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, @kw );
                        }

    /*
     * derived agentvars - transition observer family - value
     */
    | TK_value_at_first_entrance[kw] "(" SYMBOL[observed] "," constant "," SYMBOL[value] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, $value, @kw );
                        }
    | TK_value_at_latest_entrance[kw] "(" SYMBOL[observed] "," constant "," SYMBOL[value] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, $value, @kw );
                        }
    | TK_value_at_first_exit[kw] "(" SYMBOL[observed] "," constant "," SYMBOL[value] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, $value, @kw );
                        }
    | TK_value_at_latest_exit[kw] "(" SYMBOL[observed] "," constant "," SYMBOL[value] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, $value, @kw );
                        }
    | TK_value_at_first_transition[kw] "(" SYMBOL[observed] "," constant[from] "," constant[to] "," SYMBOL[value]  ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $from, $to, $value, @kw );
                        }
    | TK_value_at_latest_transition[kw] "(" SYMBOL[observed] "," constant[from] "," constant[to] "," SYMBOL[value]  ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $from, $to, $value, @kw );
                        }
    | TK_value_at_first_change[kw] "(" SYMBOL[observed] "," SYMBOL[value] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $value, @kw );
                        }
    | TK_value_at_latest_change[kw] "(" SYMBOL[observed] "," SYMBOL[value] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $value, @kw );
                        }

    /*
     * derived agentvars - transition observer family - sum
     */
    | TK_value_at_entrances[kw] "(" SYMBOL[observed] "," constant "," SYMBOL[value] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, $value, @kw );
                        }
    | TK_value_at_exits[kw] "(" SYMBOL[observed] "," constant "," SYMBOL[value] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, $value, @kw );
                        }
    | TK_value_at_transitions[kw] "(" SYMBOL[observed] "," constant[from] "," constant[to] "," SYMBOL[value]  ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $from, $to, $value, @kw );
                        }
    | TK_value_at_changes[kw] "(" SYMBOL[observed] "," SYMBOL[value] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $value, @kw );
                        }

    /*
     * derived agentvars - transformation family
     */
    | TK_split[kw] "(" SYMBOL[observed] "," SYMBOL[partition] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, static_cast<Symbol *>(nullptr), $partition, @kw );
                        }
    | TK_aggregate[kw] "(" SYMBOL[observed] "," SYMBOL[classification] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, static_cast<Symbol *>(nullptr), static_cast<Symbol *>(nullptr), $classification, @kw );
                        }

    /*
     * derived agentvars - trigger family
     */
    | TK_trigger_entrances[kw] "(" SYMBOL[observed] "," constant ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, @kw );
                        }
    | TK_trigger_exits[kw] "(" SYMBOL[observed] "," constant ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constant, @kw );
                        }
    | TK_trigger_transitions[kw] "(" SYMBOL[observed] "," constant[from] "," constant[to] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $from, $to, @kw );
                        }
    | TK_trigger_changes[kw] "(" SYMBOL[observed] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, @kw );
                        }

    /*
     * derived agentvars - self-scheduling family
     */
    | TK_duration_counter[kw] "(" SYMBOL[observed] "," constant[constnt] "," constant[interval] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constnt, $interval, @kw );
                        }
    | TK_duration_counter[kw] "(" SYMBOL[observed] "," constant[constnt] "," constant[interval]  "," constant[maxcount] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constnt, $interval, $maxcount, @kw );
                        }
    | TK_duration_trigger[kw] "(" SYMBOL[observed] "," constant[constnt] "," constant[delay] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, $constnt, $delay, @kw );
                        }
    | TK_self_scheduling_int[kw] "(" SYMBOL[observed] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, @kw );
                        }
    | TK_self_scheduling_split[kw] "(" SYMBOL[observed] "," SYMBOL[partition] ")"
                        {
                            $derived_agentvar = DerivedAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$kw, $observed, static_cast<Symbol *>(nullptr), $partition, @kw );
                        }

    /*
     * derived agentvars - self-scheduling family - integer durations
     */
    //TODO TK_self_scheduling_int - self_scheduling_int(duration())
    //TODO TK_self_scheduling_int - self_scheduling_int(active_spell_duration())
    //TODO TK_self_scheduling_int - self_scheduling_int(duration(weighted_duration()))
    //TODO TK_self_scheduling_int - self_scheduling_int(duration(active_spell_weighted_duration()))
    //TODO TK_self_scheduling_int - self_scheduling_int(age)
    //TODO TK_self_scheduling_int - self_scheduling_int(time)

    /*
     * derived agentvars - self-scheduling family - split durations
     */
    //TODO TK_self_scheduling_split - self_scheduling_split(duration(), partition)
    //TODO TK_self_scheduling_split - self_scheduling_split(active_spell_duration(), partition)
    //TODO TK_self_scheduling_split - self_scheduling_split(duration(weighted_duration()), partition)
    //TODO TK_self_scheduling_split - self_scheduling_split(duration(active_spell_wegihted_duration()), parition)
    //TODO TK_self_scheduling_split - self_scheduling_split(age, partition)
    //TODO TK_self_scheduling_split - self_scheduling_split(time, partition)

    /*
     * derived agentvars - multilink family
     */
    | TK_count[function] "(" SYMBOL[multilink] ")"
                        {
                            $derived_agentvar = MultilinkAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$function, $multilink, "" );
                        }
    | aggregate_multilink_function[function] "(" SYMBOL[multilink] ","
                        {
                            // Tell the scanner not to apply agent scope resolution to the following 'word'
                            // in the input stream, but just return a STRING instead.  That's because the agent context
                            // depends on the nature of the preceding multilink symbol, whose declaration may not yet have been encountered.
                            pc.next_word_is_string = true;
                        }
        STRING[agentvar] ")"
                        {
                            $derived_agentvar = MultilinkAgentVarSymbol::create_symbol( pc.get_agent_context(), (token_type)$function, $multilink, *$agentvar );
                            delete $agentvar; // delete the string created using new in scanner
                        }
	;

aggregate_multilink_function:
      TK_sum_over
    | TK_min_over
    | TK_max_over
    ;

/*
 * Literals & constant
 */

constant:
      signed_literal[literal]
                        {
                            // Create a constant representing a literal
                            auto cs = new ConstantSymbol($literal, @literal);
                            assert(cs);
                            $constant = cs;
                        }
    | SYMBOL[enumerator]
                        {
                            // Create a constant representing an enumerator
                            auto cs = new ConstantSymbol($enumerator, @enumerator);
                            assert(cs);
                            $constant = cs;
                        }
    ;


bool_literal:
      "true"[tok]
                        {
                            $bool_literal = new BooleanLiteral( Symbol::token_to_string( (token_type) $tok ) );
                        }
    | "false"[tok]
                        {
                            $bool_literal = new BooleanLiteral( Symbol::token_to_string( (token_type) $tok ) );
                        }
	;

numeric_literal: // Note that numeric_literal will never have a leading -
      INTEGER_LITERAL
                        {
                            $numeric_literal = $INTEGER_LITERAL;
                        }
    | FLOATING_POINT_LITERAL
                        {
                            $numeric_literal = $FLOATING_POINT_LITERAL;
                        }
    ;

signed_integer_literal:
      INTEGER_LITERAL
                        {
                            $signed_integer_literal = $INTEGER_LITERAL;
                        }
    | "-" INTEGER_LITERAL
                        {
                            $signed_integer_literal = new IntegerLiteral( "-" + $INTEGER_LITERAL->value() );
                            delete $INTEGER_LITERAL;
                        }
    ;

signed_floating_point_literal:
      FLOATING_POINT_LITERAL
                        {
                            $signed_floating_point_literal = $FLOATING_POINT_LITERAL;
                        }
    | "-" FLOATING_POINT_LITERAL
                        {
                            $signed_floating_point_literal = new FloatingPointLiteral( "-" + $FLOATING_POINT_LITERAL->value() );
                            delete $FLOATING_POINT_LITERAL;
                        }
    ;

signed_numeric_literal:
      signed_integer_literal
    | signed_floating_point_literal
    ;

non_numeric_literal:
      bool_literal
                        {
                            $non_numeric_literal = $bool_literal;
                        }
    | CHARACTER_LITERAL
                        {
                            $non_numeric_literal = $CHARACTER_LITERAL;
                        }
    | STRING_LITERAL
                        {
                            $non_numeric_literal = $STRING_LITERAL;
                        }
    | "nullptr"
                        {
                            $non_numeric_literal = new PointerLiteral( Symbol::token_to_string( token::TK_nullptr ) );
                        }
    | "time_infinite"
                        {
                            $non_numeric_literal = new TimeLiteral( Symbol::token_to_string( token::TK_time_infinite ) );
                        }
    | "time_undef"
                        {
                            $non_numeric_literal = new TimeLiteral( Symbol::token_to_string( token::TK_time_undef ) );
                        }
    ;

literal:
      numeric_literal
                        {
                            $literal = $numeric_literal;
                        }
    | non_numeric_literal
                        {
                            $literal = $non_numeric_literal;
                        }
    ;

signed_literal:
      signed_numeric_literal
                        {
                            $signed_literal = $signed_numeric_literal;
                        }
    | non_numeric_literal
                        {
                            $signed_literal = $non_numeric_literal;
                        }
    ;

/*
 * Numeric types
 */

numeric_type:
      cxx_numeric_type
    | changeable_numeric_type
	;

cxx_numeric_type:
      "char"  // in C/C++, the signedness of char is not specified
    | cxx_signed_integral_type
    | cxx_unsigned_integral_type
    | cxx_floating_point_type
	;

cxx_signed_integral_type:
      schar_synonym
    | short_synonym
    | int_synonym
    | long_synonym
    | llong_synonym
	;

cxx_unsigned_integral_type:
    "bool"  // in C/C++, false=0 and true=1, so this is an unsigned numeric type and can be used as such, e.g. for counting
    | uchar_synonym
    | ushort_synonym
    | uint_synonym
    | ulong_synonym
    | ullong_synonym
	;

cxx_floating_point_type:
      "float"
    | "double"
    | ldouble_synonym
	;

changeable_numeric_type:
      "Time"
    | "real"
    | "integer"
    | "counter"
	;

/*
 * Synonyms for fundamental C++ types 
 */

uchar_synonym:
      "uchar"
    | "unsigned" "char"           { $uchar_synonym = token::TK_uchar; }
    ;

schar_synonym:
      "schar"
    | "signed" "char"             { $schar_synonym = token::TK_schar; }
    ;

short_synonym:
      "short"
    | "short" "int"               { $short_synonym = token::TK_short; }
    | "signed" "short"            { $short_synonym = token::TK_short; }
    | "signed" "short" "int"      { $short_synonym = token::TK_short; }
    ;

ushort_synonym:
      "ushort"
    | "unsigned" "short"          { $ushort_synonym = token::TK_ushort; }
    | "unsigned" "short" "int"    { $ushort_synonym = token::TK_ushort; }
    ;

int_synonym:
      "int"
    | "signed"                    { $int_synonym = token::TK_int; }
    | "signed" "int"              { $int_synonym = token::TK_int; }
    ;

uint_synonym:
      "uint"
    | "unsigned"                  { $uint_synonym = token::TK_uint; }
    | "unsigned" "int"            { $uint_synonym = token::TK_uint; }
    ;

long_synonym:
      "long"
    | "long" "int"                { $long_synonym = token::TK_long; }
    | "signed" "long"             { $long_synonym = token::TK_long; }
    | "signed" "long" "int"       { $long_synonym = token::TK_long; }
    ;

ulong_synonym:
      "ulong"
    | "unsigned" "long"           { $ulong_synonym = token::TK_ulong; }
    | "unsigned" "long" "int"     { $ulong_synonym = token::TK_ulong; }
    ;

llong_synonym:
      "llong"
    | "long" "long"                 { $llong_synonym = token::TK_llong; }
    | "long" "long" "int"           { $llong_synonym = token::TK_llong; }
    | "signed" "long" "long"        { $llong_synonym = token::TK_llong; }
    | "signed" "long" "long" "int"  { $llong_synonym = token::TK_llong; }
    ;

ullong_synonym:
      "ullong"
    | "unsigned" "long" "long"       { $ullong_synonym = token::TK_ullong; }
    | "unsigned" "long" "long" "int" { $ullong_synonym = token::TK_ullong; }
    ;

ldouble_synonym:
      "ldouble"
    | "long" "double"             { $ldouble_synonym = token::TK_ldouble; }
    ;

%%

// Helper function to process terminal in table expressions
static ExprForTableAccumulator * table_expr_terminal(Symbol * agentvar, token_type acc, token_type incr, token_type table_op, ParseContext & pc)
{
    Symbol *table = pc.get_table_context();
    // Also create symbol for associated analysis agentvar if not already present
    TableAnalysisAgentVarSymbol *analysis_agentvar = nullptr;
    if ( TableAnalysisAgentVarSymbol::exists( table, agentvar) ) {
        string unique_name = TableAnalysisAgentVarSymbol::symbol_name( table, agentvar);
        analysis_agentvar = dynamic_cast<TableAnalysisAgentVarSymbol *>(Symbol::get_symbol( unique_name ));
        assert( analysis_agentvar );
    }
    else {
        analysis_agentvar = new TableAnalysisAgentVarSymbol( table, agentvar, pc.counter3);
        pc.counter3++;
    }
    // determine if the increment requires the creation & maintenance of an associated 'in' agent member.
    if ( analysis_agentvar->need_value_in == false ) {
        if (   incr == token::TK_delta
            || incr == token::TK_delta2
            || incr == token::TK_nz_delta
            || incr == token::TK_value_in
            || incr == token::TK_value_in2
            || incr == token::TK_nz_value_in
            ) analysis_agentvar->need_value_in = true;
    }
    // Also create symbol for associated accumulator if not already present
    TableAccumulatorSymbol *accumulator = nullptr;
    if ( TableAccumulatorSymbol::exists( table, acc, incr, table_op, agentvar) ) {
        string unique_name = TableAccumulatorSymbol::symbol_name( table, acc, incr, table_op, agentvar );
        accumulator = dynamic_cast<TableAccumulatorSymbol *>(Symbol::get_symbol( unique_name ));
        assert( accumulator );
    }
    else {
        accumulator = new TableAccumulatorSymbol( table, acc, incr, table_op, agentvar, analysis_agentvar, pc.counter2);
        pc.counter2++;
    }
	auto result = new ExprForTableAccumulator( accumulator );
    assert(result);
    return result;
}

void
yy::parser::error (const yy::parser::location_type& l,
                          const std::string& m)
{
    drv.error (l, m);
}


