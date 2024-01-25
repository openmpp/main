%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "2.6"
%defines

//%define parser_class_name "parser"
%define api.parser.class { parser }

// The following code is written to the header file, not the implementation file
%code requires {

// Copyright (c) 2013-2022 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <sstream>
#include <list>
#include <typeinfo>

using namespace std;

// forward declarations
class Driver;
class ParseContext;
class Literal;
class Constant;
class IntegerLiteral;
class CharacterLiteral;
class FloatingPointLiteral;
class StringLiteral;
class Symbol;
class EntityMemberSymbol;
class ConstantSymbol;
class ExprForAttribute;
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
#include "libopenm/common/omFile.h"
	// last item found by flex.  Used in grammar to provide error information
// TODO check and eliminate use
extern char *yytext;

// Helper function to process terminal in table expressions
static ExprForTableAccumulator * table_expr_terminal(Symbol *attribute, token_type acc, token_type incr, token_type table_op, ParseContext & pc);

// Helper function to handle prohibited redeclaration
static bool check_undeclared(Symbol* sym, const yy::parser::location_type& loc, Driver &drv);

}

// The parsing context.

%parse-param { Driver& drv }
%parse-param { ParseContext& pc }

%lex-param   { Driver& drv }
%lex-param   { ParseContext& pc }

%locations
//%define api.filename.type "std::string"
%define api.location.type{ omc::location }
%code requires { #include "omc_location.hh" }

%initial-action
{
    // Initialize location.
    @$.begin.filename = @$.end.filename = drv.pfile;
};

%debug

//%error-verbose
%define parse.error verbose

%token-table

// Items in the bison symbol stack
// Naming conventions:
// keywords have prefix TK_ followed by keyword
// member of union have prefix val_ folowed by type for native C types, or pval_ for pointers to objects
%union
{
	int                   val_token;
    int                   val_int;
    bool                  val_bool;
    Literal              *pval_Literal;
    IntegerLiteral       *pval_IntegerLiteral;
    CharacterLiteral     *pval_CharacterLiteral;
    FloatingPointLiteral *pval_FloatingPointLiteral;
    StringLiteral        *pval_StringLiteral;
	Symbol               *pval_Symbol;
	EntityMemberSymbol   *pval_entityMemberSymbol;
	ConstantSymbol       *pval_ConstantSymbol;
	Constant             *pval_Constant;
    list<Constant *>     *pval_Constant_list;
	ExprForAttribute     *pval_AttributeExpr;
	ExprForTable         *pval_TableExpr;
    string               *pval_string;
    list<string *>       *pval_string_list;
    list<Symbol *>       *pval_Symbol_list;
};

%printer { yyoutput << "token " << Symbol::token_to_string((token_type)$$); } <val_token>
%printer { yyoutput << "literal " << ($$ ? $$->value() : "nullptr"); } <pval_Literal>
%printer { yyoutput << "integer literal " << ($$ ? $$->value() : "nullptr"); } <pval_IntegerLiteral>
%printer { yyoutput << "character literal " << ($$ ? $$->value() : "nullptr"); } <pval_CharacterLiteral>
%printer { yyoutput << "floating point literal " << ($$ ? $$->value() : "nullptr"); } <pval_FloatingPointLiteral>
%printer { yyoutput << "string literal " << ($$ ? $$->value() : "nullptr"); } <pval_StringLiteral>
%printer { yyoutput << "constant " << ($$ ? ($$->is_literal ? $$->literal->value() : (*($$->enumerator))->name) : "nullptr"); } <pval_ConstantSymbol>
// special code in line below to handle decl_type_part "void", whose value is nullptr
%printer { yyoutput << "symbol " << '"' << ($$ ? $$->name : "void") << '"' << " type=" << ($$ ? typeid($$).name() : "void"); } <pval_Symbol>
%printer { yyoutput << "table expr "; } <pval_TableExpr>

%destructor { delete $$; } <pval_IntegerLiteral> <pval_CharacterLiteral> <pval_FloatingPointLiteral> <pval_StringLiteral>


// NB: There is an exact one-to-one correspondence with code in Symbol.cpp

// top level om keywords, in alphabetic order
%token <val_token>    TK_aggregation    "aggregation"
%token <val_token>    TK_big_counter_type "big_counter_type"
%token <val_token>    TK_classification "classification"
%token <val_token>    TK_counter_type   "counter_type"
%token <val_token>    TK_dependency     "dependency"
%token <val_token>    TK_derived_table "derived_table"
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
%token <val_token>    TK_parameters_suppress  "parameters_suppress"
%token <val_token>    TK_parameters_to_tables "parameters_to_tables"
%token <val_token>    TK_parameters_retain    "parameters_retain"
%token <val_token>    TK_parameter_group "parameter_group"
%token <val_token>    TK_partition      "partition"
%token <val_token>    TK_pull           "pull"
%token <val_token>    TK_range          "range"
%token <val_token>    TK_real_type      "real_type"
%token <val_token>    TK_show         "show"
%token <val_token>    TK_string         "string"
%token <val_token>    TK_table          "table"
%token <val_token>    TK_tables_suppress "tables_suppress"
%token <val_token>    TK_tables_retain   "tables_retain"
%token <val_token>    TK_table_group    "table_group"
%token <val_token>    TK_time_type      "time_type"
%token <val_token>    TK_track          "track"
%token <val_token>    TK_use            "use"
%token <val_token>    TK_user_table     "user_table"
%token <val_token>    TK_version        "version"

// body level om keywords, in alphabetic order
%token <val_token>    TK_active_spell_delta        "active_spell_delta"
%token <val_token>    TK_active_spell_duration     "active_spell_duration"
%token <val_token>    TK_active_spell_weighted_duration  "active_spell_weighted_duration"
%token <val_token>    TK_aggregate                 "aggregate"
%token <val_token>    TK_all_base_states           "all_base_states"
%token <val_token>    TK_all_derived_states        "all_derived_states"
%token <val_token>    TK_all_internal_states       "all_internal_states"
%token <val_token>    TK_all_links                 "all_links"
%token <val_token>    TK_big_counter               "big_counter"
%token <val_token>    TK_case_based                "case_based"
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
%token <val_token>    TK_derived                   "derived"
%token <val_token>    TK_duration                  "duration"
%token <val_token>    TK_duration_counter          "duration_counter"
%token <val_token>    TK_duration_trigger          "duration_trigger"
%token <val_token>    TK_entrances                 "entrances"
%token <val_token>    TK_event                     "event"
%token <val_token>    TK_event_trace               "event_trace"
%token <val_token>    TK_exits                     "exits"
%token <val_token>    TK_file                      "file"
%token <val_token>    TK_filter                    "filter"
%token <val_token>    TK_gini                      "gini"
%token <val_token>    TK_haz1rate                  "haz1rate"
%token <val_token>    TK_haz2rate                  "haz2rate"
%token <val_token>    TK_hook                      "hook"
%token <val_token>    TK_IMPLEMENT_HOOK            "IMPLEMENT_HOOK"
%token <val_token>    TK_index                     "index"
%token <val_token>    TK_integer                   "integer"
%token <val_token>    TK_interval                  "interval"
%token <val_token>    TK_just_in_time              "just_in_time"
%token <val_token>    TK_ldouble                   "ldouble"
%token <val_token>    TK_llong                     "llong"
%token <val_token>    TK_maximum                   "maximum"
%token <val_token>    TK_max_delta                 "max_delta"
%token <val_token>    TK_max_over                  "max_over"
%token <val_token>    TK_max_value_in              "max_value_in"
%token <val_token>    TK_max_value_out             "max_value_out"
%token <val_token>    TK_minimum                   "minimum"
%token <val_token>    TK_min_delta                 "min_delta"
%token <val_token>    TK_min_over                  "min_over"
%token <val_token>    TK_min_value_in              "min_value_in"
%token <val_token>    TK_min_value_out             "min_value_out"
%token <val_token>    TK_model_generated           "model_generated"
%token <val_token>    TK_nz_delta                  "nz_delta"
%token <val_token>    TK_nz_value_in               "nz_value_in"
%token <val_token>    TK_nz_value_out              "nz_value_out"
%token <val_token>    TK_order                     "order"
%token <val_token>    TK_P1                        "P1"
%token <val_token>    TK_P2                        "P2"
%token <val_token>    TK_P5                        "P5"
%token <val_token>    TK_P10                       "P10"
%token <val_token>    TK_P20                       "P20"
%token <val_token>    TK_P25                       "P25"
%token <val_token>    TK_P30                       "P30"
%token <val_token>    TK_P40                       "P40"
%token <val_token>    TK_P50                       "P50"
%token <val_token>    TK_P60                       "P60"
%token <val_token>    TK_P70                       "P70"
%token <val_token>    TK_P75                       "P75"
%token <val_token>    TK_P80                       "P80"
%token <val_token>    TK_P90                       "P90"
%token <val_token>    TK_P95                       "P95"
%token <val_token>    TK_P98                       "P98"
%token <val_token>    TK_P99                       "P99"
%token <val_token>    TK_piece_linear              "piece_linear"
%token <val_token>    TK_RandomStream              "RandomStream"
%token <val_token>    TK_rate                      "rate"
%token <val_token>    TK_real                      "real"
%token <val_token>    TK_schar                     "schar"
%token <val_token>    TK_self_scheduling_int       "self_scheduling_int"
%token <val_token>    TK_self_scheduling_split     "self_scheduling_split"
%token <val_token>    TK_sparse                    "sparse"
%token <val_token>    TK_split                     "split"
%token <val_token>    TK_sqrt                      "sqrt"
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
%token <val_token>    TK_double_arrow               "=>"

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
// Source: http://en.cppreference.com/w/cpp/language/expressions#Operators

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
%token <val_token>    TK_ELLIPSIS      "..."

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
%left  POSTFIX_INCREMENT POSTFIX_DECREMENT '(' FUNCTION_CALL "[" ARRAY_SUBSCRIPTING "." "->"  // precedence 2
%left  "::"              // precedence 1

%type  <val_token>      model_type
%type  <val_token>      modgen_cumulation_operator
%type  <val_token>      table_accumulator
%type  <val_token>      table_increment
%type  <val_token>      table_operator
%type  <val_token>      parameter_modifier_opt
%type  <val_token>      table_margin_opt
%type  <val_token>      decl_func_arg_token
%type  <val_token>      parameter_group_token
%type  <val_token>      anon_group_kw
%type  <pval_IntegerLiteral> cumrate_dimensions_opt

%type  <pval_Literal>   bool_literal
%type  <pval_Literal>   numeric_literal
%type  <pval_Literal>   signed_integer_literal
%type  <pval_Literal>   signed_floating_point_literal
%type  <pval_Literal>   signed_numeric_literal
%type  <pval_Literal>   non_numeric_literal
%type  <pval_Literal>   literal
%type  <pval_Literal>   signed_literal

%type  <pval_Symbol>    decl_type_part
%type  <pval_Symbol>    symbol_in_expr
%type  <pval_Symbol>    symbol_in_table
%type  <pval_Symbol>    derived_attribute
%type  <pval_Symbol>    derived_attribute_self_scheduling
%type  <pval_Symbol>    derived_attribute_trigger
%type  <pval_Symbol>    derived_attribute_other
%type  <pval_Symbol>    any_attribute
%type  <pval_Symbol>    link_to_attribute
%type  <pval_Symbol>    link_to_attribute_shadowed
%type  <pval_Symbol>    hook_to_symbol
%type  <pval_Literal>   event_priority_opt
%type  <pval_Literal>   hook_order_opt
%type  <pval_ConstantSymbol>  constant
%type  <pval_Constant>  initializer_constant

%type  <pval_entityMemberSymbol> link_symbol

%type  <pval_TableExpr> expr_for_table
%type  <pval_TableExpr> table_expression_list

%type  <pval_AttributeExpr> expr_for_attribute

%type  <val_token> aggregate_multilink_function

%type  <val_token>      numeric_type
%type  <val_token>      changeable_numeric_type
%type  <val_token>      cxx_numeric_type
%type  <val_token>      cxx_signed_integral_type
%type  <val_token>      cxx_unsigned_integral_type
%type  <val_token>      cxx_floating_point_type

%type <pval_Constant>  parameter_initializer_element
%type <pval_Constant_list> parameter_initializer_list
%type <pval_string>      decl_func_arg_element
%type <pval_string>      decl_func_arg_string
%type <pval_string>      option_rhs
%type <pval_Symbol_list> array_decl_dimension_list
%type <pval_Symbol_list> symbol_list

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

%type  <pval_string>    name_opt
%type  <val_bool>       sample_dimension_opt
;

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
	| decl_import           { pc.InitializeForCxx(); }
	| decl_pull             { pc.InitializeForCxx(); }
	| decl_model_type       { pc.InitializeForCxx(); }
	| decl_time_type        { pc.InitializeForCxx(); }
	| decl_real_type        { pc.InitializeForCxx(); }
	| decl_counter_type     { pc.InitializeForCxx(); }
	| decl_big_counter_type { pc.InitializeForCxx(); }
	| decl_integer_type     { pc.InitializeForCxx(); }
	| decl_index_type       { pc.InitializeForCxx(); }
	| decl_version          { pc.InitializeForCxx(); }
    | decl_classification   { pc.InitializeForCxx(); }
    | decl_aggregation      { pc.InitializeForCxx(); }
    | decl_partition        { pc.InitializeForCxx(); }
    | decl_range            { pc.InitializeForCxx(); }
    | decl_parameter_group  { pc.InitializeForCxx(); }
    | decl_table_group      { pc.InitializeForCxx(); }
    | decl_hide             { pc.InitializeForCxx(); }
    | decl_anon_group       { pc.InitializeForCxx(); }
    | decl_dependency       { pc.InitializeForCxx(); }
    | decl_track            { pc.InitializeForCxx(); }
    | decl_parameters       { pc.InitializeForCxx(); }
	| decl_entity           { pc.InitializeForCxx(); }
	| decl_link             { pc.InitializeForCxx(); }
	| decl_entity_set       { pc.InitializeForCxx(); }
	| decl_table            { pc.InitializeForCxx(); }
	| decl_derived_table    { pc.InitializeForCxx(); }
    | decl_extend_parameter { pc.InitializeForCxx(); }
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
							bool found = false;
							for (auto use_dir : Symbol::use_folders) {
								// prepend a 'use' folder location
								string use_file_full = use_dir + use_file;
								if (openm::isFileExists(use_file_full.c_str())) {
									found = true;
									Symbol::all_source_files.push_back(use_file_full);
									Symbol::use_source_files.push_back(use_file_full);
									break;
								}
							}
							if (!found) {
								error(@path, LT("error: file '") + use_file + LT("' not found in any use folder."));
							}
                            delete $path;
                            $path = nullptr;
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
                            assert(sym);
                        }
	| language_list "," SYMBOL
                        {
                            // morph existing symbol to LanguageSymbol
                            auto *sym = new LanguageSymbol( $SYMBOL, @SYMBOL );
                            assert(sym);
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
                            assert(sym);
                        }
        | "string" error ";"
        ;

/*
 * options
 */

decl_options:
          "options" 
                        {
                            // Actually, it's too late to do this here because the look-ahead token
                            // has already been read.  So "options" is detected and handled as a special
                            // case in the scanner.
                            pc.next_word_is_string = true;
                        }
              options_list ";"
                        {
                            pc.next_word_is_string = false;
                        }
       | "options"
                        {
                            pc.next_word_is_string = true;
                        }
              error ";"
                        {
                            pc.next_word_is_string = false;
                        }
    ;

options_list:
          option
	    | options_list option
	;

option_rhs[result]:
            STRING[value]
                        {
                            $result = $value;
                        }
         | numeric_literal[value]
                        {
                            // create a new string from the numeric_literal
                            $result = new string($value->value());
                            // finished with the StringLiteral
                            delete $value;
                            $value = nullptr;
                        }
    ;

option:
          STRING[key] "=" 
                        {
                            pc.next_word_is_string = true;
                        }
              option_rhs[value]
                        {
                            // check for possible clash in model source specifying same key
                            auto srch = Symbol::options.find(*$key);
                            if (srch != Symbol::options.end()) {
                                auto& opt_pair = srch->second; // opt_pair is option value, option location
                                if (opt_pair.first != *$value) {
                                    // key already specified in model source with different value
                                    if (*$key == "local_random_streams") {
                                        // special case, allow multiple options statements
                                    }
                                    else {
                                        error(@value, LT("error: option '") + (*$key) + LT("' specified elsewhere as '") + (opt_pair.first) + LT("'"));
                                    }
                                }
                            }
                            // place key-value pair in options collection
                            //Symbol::options[*$key] = *$value;
                            string opt_val = *$value;
                            omc::location opt_loc = @key;
                            auto opt_both = make_pair(opt_val, opt_loc);
                            string opt_key = *$key;
                            auto opt_thing = make_pair(opt_key, opt_both);
                            Symbol::options.insert(Symbol::options.begin(), opt_thing);
                            // prepare for another possible key-value pair
                            pc.next_word_is_string = true;
                        }
	;

/*
 * import
 */

decl_import:    // DEPRECATED: import is a reserved keyword in c++20
          "import" do_decl_import
        | "import" error ";"
        ;

decl_pull:      // "pull" is a synonym for "import" keyword to avoid c++20 conflict
          "pull" do_decl_import
        | "pull" error ";"
        ;

do_decl_import: // just parse old-style syntax, and assume a single table name, not a list
              SYMBOL[param] "("
                        {
                            pc.next_word_is_string = true;
                        }
              STRING[model] "." 
                        {
                            pc.next_word_is_string = true;
                        }
              STRING[table] ")"
                        {
                            pc.next_word_is_string = true;
                        }
              sample_dimension_opt ";"
                        {
                            // create new symbol for import statement
                            auto *imp = new ImportSymbol($param, *$model, *$table, $sample_dimension_opt, @param);
                            assert(imp);

                            // cleanup
                            delete $model;
                            $model = nullptr;
                            delete $table;
                            $table = nullptr;

                            // finished import statement, return to normal symbol processing
                            pc.next_word_is_string = false;
                        }
        ;

sample_dimension_opt:
          STRING[key] "="
                        {
                            // only allowed key is sample_dimension
                            if (*$key != "sample_dimension") {
                                error(@key, LT("error: invalid option in import statement (must be 'sample_dimension')."));
                            }
                            delete $key;
                            $key = nullptr;
                            pc.next_word_is_string = true;
                        }
              STRING[value]
                        {
                            if (*$value == "on") {
                                $$ = true;
                            }
                            else if (*$value == "off") {
                                $$ = false;
                            }
                            else {
                                error(@value, LT("error: invalid option value in import statement (must be 'on' or 'off')."));
                            }
                            delete $value;
                            $value = nullptr;
                        }
                  | /* nothing */
                        {
                            // default if sample_dimension is not specified
                            $$ = false;
                        }
      ;


/*
 * mutable types
 */

decl_time_type:
	  "time_type" cxx_numeric_type[type_to_use] ";"
                        {
                            Symbol::Time_ctype = (token_type)$type_to_use;
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
                            Symbol::real_ctype = (token_type)$type_to_use;
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
                            Symbol::counter_ctype = (token_type)$type_to_use;
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
                            Symbol::big_counter_ctype = (token_type)$type_to_use;
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
                            Symbol::integer_ctype = (token_type)$type_to_use;
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
                            // Instantiation of new instance will replace existing instance (in constructor for Symbol)
                            auto *sym = new ModelTypeSymbol((token_type) $type_to_use, false, @$);
                            assert(sym);
                        }
        | "model_type" model_type[type_to_use] "," "just_in_time" ";"
                        {
                            // Instantiation of new instance will replace existing instance (in constructor for Symbol)
                            auto *sym = new ModelTypeSymbol((token_type)$type_to_use, true, @$);
                            assert(sym);
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
                            $major = nullptr;

                            int minor = stoi($minor->value());
                            delete $minor;
                            $minor = nullptr;

                            int sub_minor = stoi($sub_minor->value());
                            delete $sub_minor;
                            $sub_minor = nullptr;

                            int sub_sub_minor = stoi($sub_sub_minor->value());
                            delete $sub_sub_minor;
                            $sub_sub_minor = nullptr;

                            auto *sym = new VersionSymbol( major, minor, sub_minor, sub_sub_minor, @$ );
                            assert(sym);
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
                            pc.reset_working_counters();
                        }
            "{" classification_levels "}" ";"[decl_end]
                        {
                            // Leaving classification context
                            // Update the end position of the declaration.
                            auto cl = pc.get_classification_context();
                            cl->decl_loc.end = @decl_end.end;
                            // clean up
                            pc.set_classification_context( nullptr );
                            pc.reset_working_counters();
                        }
            | "classification" "{" error "}" ";"
                        {
                            // Leaving classification context
                            pc.set_classification_context(nullptr);
                            pc.reset_working_counters();
                        }
            ;

classification_levels:
      name_opt[short_name] SYMBOL
                        {
                            // morph existing symbol to EnumeratorSymbol
                            auto *sym = new ClassificationEnumeratorSymbol($SYMBOL, $short_name, pc.get_classification_context(), pc.counter1, @SYMBOL);
                            assert(sym);
                            pc.counter1++;  // counter for classification levels
                        }
      | classification_levels "," name_opt[short_name] SYMBOL
                        {
                            // morph existing symbol to EnumeratorSymbol
                            auto *sym = new ClassificationEnumeratorSymbol($SYMBOL, $short_name, pc.get_classification_context(), pc.counter1, @SYMBOL);
                            assert(sym);
                            pc.counter1++;  // counter for classification levels
                        }
	;

/*
 * aggregation
 */

decl_aggregation:
      "aggregation"[kw] SYMBOL[to_class] "," SYMBOL[from_class]
            "{" symbol_list "}" ";"
                        {
                            if (AggregationSymbol::exists($from_class, $to_class)) {
                                // redeclaration not allowed
                                error(@kw, LT("error: an aggregation from '") + $from_class->name + LT("' to '") + $to_class->name + LT("' already exists"));
                            }
                            // create new AggregationSymbol
                            auto *agg = new AggregationSymbol( $from_class, $to_class, @kw );
                            assert(agg);
                            list<Symbol *> *pls = $symbol_list;
                            // move symbol list to group (transform elements to stable **)
                            for (auto sym : *pls) agg->symbol_list.push_back(sym->stable_pp());
                            pls->clear();
                            delete pls;
                            $symbol_list = nullptr;
                        }
    | "aggregation" "{" error "}" ";"
	| "aggregation" error ";"
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
                            pc.reset_working_counters();
                        }
            "{" partition_splitpoints "}"[last] ";"[decl_end]
                        {
                            // create PartitionEnumeratorSymbol for upper partition interval
                            Symbol *enum_symbol = pc.get_partition_context();
                            string enumerator_name = enum_symbol->name + "_om_" + to_string(pc.counter1);
                            string upper_split_point = "max";
                            auto *sym = new PartitionEnumeratorSymbol(enumerator_name, enum_symbol, pc.counter1, upper_split_point, @last);
                            assert(sym);

                            // Leaving partition context
                            // Update the end position of the declaration.
                            auto part = pc.get_partition_context();
                            part->decl_loc.end = @decl_end.end;
                            // clean up
                            pc.set_partition_context( nullptr );
                            pc.reset_working_counters();
                        }
            | "partition" "{" error "}" ";"
                        {
                            // Leaving partition context
                            pc.set_partition_context(nullptr);
                            pc.reset_working_counters();
                        }
            ;

partition_splitpoints:
      signed_numeric_literal
                        {
                            // create PartitionEnumeratorSymbol for interval which ends at this split point
                            Symbol *enum_symbol = pc.get_partition_context();
                            string enumerator_name = enum_symbol->name + "_om_" + to_string(pc.counter1);
                            string upper_split_point = $signed_numeric_literal->value();
                            auto *sym = new PartitionEnumeratorSymbol(enumerator_name, enum_symbol, pc.counter1, upper_split_point, @signed_numeric_literal);
                            assert(sym);
                            pc.counter1++;  // counter for partition split points
                        }
      | partition_splitpoints "," signed_numeric_literal
                        {
                            // create PartitionEnumeratorSymbol for interval which ends at this split point
                            Symbol *enum_symbol = pc.get_partition_context();
                            string enumerator_name = enum_symbol->name + "_om_" + to_string(pc.counter1);
                            string upper_split_point = $signed_numeric_literal->value();
                            auto *sym = new PartitionEnumeratorSymbol(enumerator_name, enum_symbol, pc.counter1, upper_split_point, @signed_numeric_literal);
                            assert(sym);
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
                            $lower_bound = nullptr;

                            int upper_bound = stoi($upper_bound->value());
                            delete $upper_bound;
                            $upper_bound = nullptr;

                            // Morph Symbol to RangeSymbol
                            $range = new RangeSymbol( $range, lower_bound, upper_bound, @range );
                        }
      | "range" "{" error "}" ";"
    ;

/*
 * parameter_group
 */

parameter_group_token:
          "parameter_group"
        | "model_generated_parameter_group"
        ;

decl_parameter_group:
	  parameter_group_token[tok] SYMBOL[group] "{" symbol_list "}" ";"
                        {
                            // morph existing symbol to ParameterGroupSymbol
                            auto *grp = new ParameterGroupSymbol( $group, @group );
                            assert(grp);
                            list<Symbol *> *pls = $symbol_list;
                            // move symbol list to group (transform elements to stable **)
                            for (auto sym : *pls) grp->symbol_list.push_back(sym->stable_pp());
                            pls->clear();
                            delete pls;
                            $symbol_list = nullptr;
                        }
	| parameter_group_token "{" error "}" ";"
	| parameter_group_token error ";"
      ;

// A non-empty list of comma-separated symbols.
symbol_list:
      SYMBOL
                        {
                            // start a new symbol list
                            auto * pls = new list<Symbol *>;
                            pls->push_back($SYMBOL);
                            $$ = pls;
                        }
    | symbol_list[pls] "," SYMBOL
                        {
                            // append to existing symbol list
                            $pls->push_back($SYMBOL);
                            $$ = $pls;
                        }
	;


/*
 * table_group
 */

decl_table_group:
	  "table_group" SYMBOL[group] "{" symbol_list "}" ";"
                        {
                            // morph existing symbol to TableGroupSymbol
                            auto *grp = new TableGroupSymbol( $group, @group );
                            assert(grp);
                            list<Symbol *> *pls = $symbol_list;
                            // move symbol list to group (transform elements to stable **)
                            for (auto sym : *pls) grp->symbol_list.push_back(sym->stable_pp());
                            pls->clear();
                            delete pls;
                            $symbol_list = nullptr;
                        }
	| "table_group" "{" error "}" ";"
	| "table_group" error ";"
      ;


/*
 * hide, show, parameters_suppress, parameters_retain, tables_suppress, tables_retain
 */

decl_hide:
	  "hide"[tok] "(" symbol_list ")" ";"  // deprecated Modgen hide statement using parentheses
                        {
                            // create new AnonGroupSymbol, of provenance 'hide'
                            auto *grp = new AnonGroupSymbol(AnonGroupSymbol::eKind::hide, @tok );
                            assert(grp);
                            list<Symbol *> *pls = $symbol_list;
                            // move symbol list to group (transform elements to stable **)
                            for (auto sym : *pls) grp->symbol_list.push_back(sym->stable_pp());
                            pls->clear();
                            delete pls;
                            $symbol_list = nullptr;
                        }
    | "hide" "(" error ")" ";"
    ;

decl_anon_group:
    anon_group_kw[tok] symbol_list ";"
                        {
                            // the originating anonymous group statement, eg parameters_suppress
                            AnonGroupSymbol::eKind anon_kind;

                            // assign anon_kind by translating token to enum
                            switch ($tok) {
                            case token::TK_hide:                 { anon_kind = AnonGroupSymbol::eKind::hide; break; }
                            case token::TK_show:               { anon_kind = AnonGroupSymbol::eKind::show; break; }
                            case token::TK_parameters_suppress:  { anon_kind = AnonGroupSymbol::eKind::parameters_suppress; break; }
                            case token::TK_parameters_retain:    { anon_kind = AnonGroupSymbol::eKind::parameters_retain; break; }
                            case token::TK_tables_suppress:      { anon_kind = AnonGroupSymbol::eKind::tables_suppress; break; }
                            case token::TK_tables_retain:        { anon_kind = AnonGroupSymbol::eKind::tables_retain; break; }
                            case token::TK_parameters_to_tables: { anon_kind = AnonGroupSymbol::eKind::parameters_to_tables; break; }
                            default: assert(false); // logic guarantee
                            }

                            // create new AnonGroupSymbol, first argument of constructor gives the provenance
                            auto* grp = new AnonGroupSymbol(anon_kind, @tok);
                            assert(grp);
                            list<Symbol*>* pls = $symbol_list;
                            // move symbol list to group (transform elements to stable **)
                            for (auto sym : *pls) grp->symbol_list.push_back(sym->stable_pp());
                            pls->clear();
                            delete pls;
                            $symbol_list = nullptr;
                        }
    | anon_group_kw error ";"
      ;

anon_group_kw:
      TK_hide
    | TK_show
    | TK_parameters_suppress
    | TK_parameters_retain
    | TK_tables_suppress
    | TK_tables_retain
    | TK_parameters_to_tables
      ;


/*
 * dependency
 */

decl_dependency:
	  "dependency"[tok] "(" symbol_list ")" ";"
                        {
                            // morph existing symbol to DependencyGroupSymbol
                            auto *grp = new DependencyGroupSymbol(@tok );
                            assert(grp);
                            list<Symbol *> *pls = $symbol_list;
                            // move symbol list to group (transform elements to stable **)
                            for (auto sym : *pls) grp->symbol_list.push_back(sym->stable_pp());
                            pls->clear();
                            delete pls;
                            $symbol_list = nullptr;
                        }
	| "dependency" "(" error ")" ";"
	| "dependency" error ";"
      ;


/*
 * track
 */

decl_track:
	  "track" SYMBOL[entity] 
                        {
                            // Set entity context for body of track declaration
                            pc.set_entity_context( $entity);
                        }
          track_filter_opt "{" symbol_list "}" ";"
                        {
                            list<Symbol *> *pls = $symbol_list;
                            // move symbol list to group (transform elements to stable **)
                            //for (auto sym : *pls) grp->symbol_list.push_back(sym->stable_pp());
                            pls->clear();
                            delete pls;
                            $symbol_list = nullptr;
                            // Leaving entity context
                            pc.set_entity_context( nullptr );
                        }
	| "track" error ";"
      ;


track_filter_opt:
	  "[" expr_for_attribute[root] "]"
                        {
                            //TODO
                            // if topmost $root node is comma-operator "," drill down a level to find 
                            // the ongoing filter (left) and the final filter (right)
                            // else $root is the ongoing filter and there is no final filter
                            // store filter(s) in IdentityAttributeSymbol members of EntitySymbol
                        }
	| /* nothing */
      ;

/*
* naming operator =>
*/

name_opt:
    STRING[name] "=>"
    {
        // if specified, is pointer to string
        $$ = $name;
    }
    | /* nothing */
    {
        // if not specified, is null
        $$ = nullptr;
    }
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

parameter_modifier_opt:
      "derived"[kw]
                        {
                            $parameter_modifier_opt = $kw;
                        }
    | /* nothing */
                        {
                            $parameter_modifier_opt = token::TK_unused;
                        }
    ;

cumrate_dimensions_opt:
      "[" INTEGER_LITERAL[dims] "]"
                        {
                            $cumrate_dimensions_opt = $dims;
                        }
    | /* nothing */
                        {
                            $cumrate_dimensions_opt = nullptr;
                        }
    ;

decl_parameter:
      parameter_modifier_opt[pm_opt] decl_type_part[type_symbol] SYMBOL[parm]
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
                                parm->redecl_loc = @parm; // note redeclaration location
                                // note type in redeclaration for subsequent semantic check
                                parm->datatype2 = ($type_symbol)->stable_pp();
                            }
                            // update provenance if derived
                            if ($pm_opt == token::TK_derived) {
                                parm->source = ParameterSymbol::parameter_source::derived_parameter;
                            }
                            // Set parameter context for gathering the dimension specification (if present)
                            // and initializer (if present).
                            pc.set_parameter_context( parm );
                            pc.reset_working_counters();
                        }
            decl_dim_list parameter_initializer_expr ";"
                        {
                            // Leaving parameter context
                            pc.set_parameter_context(nullptr);
                            pc.reset_working_counters();
                        }
    | parameter_modifier_opt[pm_opt] "haz1rate" SYMBOL[parm]
                        {
                            // haz1rate is deprecated and undocumented, but still present in older models, so support it, silently.
                            ParameterSymbol *parm = nullptr;

                            if ($parm->is_base_symbol()) {
                                // parameter declaration
                                pc.redeclaration = false;
                                // storage type of haz1rate parameters is double
                                auto *type_symbol = Symbol::get_symbol("double");
                                assert(type_symbol); // grammar/initialization guarantee
                                // Morph Symbol to ParameterSymbol
                                parm = new ParameterSymbol( $parm, type_symbol, @parm );
                                assert(parm);
                                parm->haz1rate = true;
                                $parm = parm;
                            }
                            else {
                                // parameter re-declaration
                                pc.redeclaration = true;
                                parm = dynamic_cast<ParameterSymbol *>($parm);
                                assert(parm); // grammar/logic guarantee
                                parm->redecl_loc = @parm; // note redeclaration location
                            }
                            // update provenance if derived
                            if ($pm_opt == token::TK_derived) {
                                parm->source = ParameterSymbol::parameter_source::derived_parameter;
                            }
                            // Set parameter context for gathering the dimension specification (if present)
                            // and initializer (if present).
                            pc.set_parameter_context( parm );
                            pc.reset_working_counters();
                        }
            decl_dim_list parameter_initializer_expr ";"
                        {
                            // Leaving parameter context
                            pc.set_parameter_context(nullptr);
                            pc.reset_working_counters();
                        }
    | parameter_modifier_opt[pm_opt] "cumrate" cumrate_dimensions_opt[cumrate_dims] SYMBOL[parm]
                        {
                            ParameterSymbol *parm = nullptr;

                            if ($parm->is_base_symbol()) {
                                // parameter declaration
                                pc.redeclaration = false;
                                // storage type of cumrate parameters is double
                                auto *type_symbol = Symbol::get_symbol("double");
                                assert(type_symbol); // grammar/initialization guarantee
                                // Morph Symbol to ParameterSymbol
                                parm = new ParameterSymbol( $parm, type_symbol, @parm );
                                assert(parm);
                                $parm = parm;
                            }
                            else {
                                // parameter re-declaration
                                pc.redeclaration = true;
                                parm = dynamic_cast<ParameterSymbol *>($parm);
                                assert(parm); // grammar/logic guarantee
                                parm->redecl_loc = @parm; // note redeclaration location
                            }
                            // update provenance if model_generated
                            if ($pm_opt == token::TK_derived) {
                                parm->source = ParameterSymbol::parameter_source::derived_parameter;
                            }
                            // record cumrate information in parameter
                            parm->cumrate = true;
                            // record number of non-conditioning trailing dimensions
                            if ($cumrate_dims) {
                                parm->cumrate_dims = stoi($cumrate_dims->value());
                                delete $cumrate_dims;
                                $cumrate_dims = nullptr;
                            }
                            else {
                                // default value if not specified explcitily
                                parm->cumrate_dims = 1;
                            }
                            // Set parameter context for gathering the dimension specification (if present)
                            // and initializer (if present).
                            pc.set_parameter_context( parm );
                            pc.reset_working_counters();
                        }
            decl_dim_list parameter_initializer_expr ";"
                        {
                            // Leaving parameter context
                            pc.set_parameter_context(nullptr);
                            pc.reset_working_counters();
                        }
    | "file" SYMBOL[parm]
                        {
                            ParameterSymbol *parm = nullptr;

                            if ($parm->is_base_symbol()) {
                                // parameter declaration
                                pc.redeclaration = false;
                                // Morph Symbol to ParameterSymbol
                                TypeSymbol *ts = StringTypeSymbol::find();
                                assert(ts);
                                parm = new ParameterSymbol( $parm, ts, @parm );
                                assert(parm);
                                $parm = parm;
                            }
                            else {
                                // parameter re-declaration
                                pc.redeclaration = true;
                                parm = dynamic_cast<ParameterSymbol *>($parm);
                                assert(parm); // grammar/logic guarantee
                                parm->redecl_loc = @parm; // note redeclaration location
                            }
                            // Set parameter context for gathering the initializer (if present).
                            pc.set_parameter_context( parm );
                            pc.reset_working_counters();
                        }
            parameter_initializer_expr ";"
                        {
                            // Leaving parameter context
                            pc.set_parameter_context( nullptr );
                            pc.reset_working_counters();
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
                            pc.reset_working_counters();
                        }
	;

decl_dim_list:
      decl_dim_list name_opt[short_name] "[" SYMBOL[enumeration] "]"
                        {
                            Symbol *enumeration = $enumeration;
                            assert(enumeration);
                            if (!pc.redeclaration) {
                                // add enumeration to parameter's enumeration list
                                pc.get_parameter_context()->enumeration_list.push_back(enumeration->stable_pp());

                                auto sym = new DimensionSymbol(pc.get_parameter_context(), $short_name, pc.counter4, false, nullptr, enumeration, false, @enumeration);
                                assert(sym);
                                // add dimension symbol to parameter's dimension_list
                                pc.get_parameter_context()->dimension_list.push_back(sym);
                                // Increment the counter used for the number of dimensions.
                                // This is the same as the 0-based ordinal of the next dimension in the declaration.
                                pc.counter4++;
                            }
                            else {
                                // keep track of dimension list of redeclaration for subsequent semantic check
                                pc.get_parameter_context()->enumeration_list2.push_back(enumeration->stable_pp());
                            }
                        }
    | /* Nothing */
    ;

parameter_initializer_expr:
      "="[eq] parameter_initializer_element
                        {
                            // add first (and only) element to the initializer list
                            auto parm = pc.get_parameter_context();
                            assert(parm); // grammar guarantee
                            if (parm->is_derived()) {
                                ostringstream msg;
                                msg << LT("warning : ignoring initializer for derived parameter ") << parm->name;
                                drv.warning(@eq, msg.str());
                                //TODO should not fall through to initialization code after if/elseif/else below
                            }
                            else if (pc.is_scenario_parameter_value) {
                                parm->initializations_count++;
                                parm->source = ParameterSymbol::scenario_parameter;
                            }
                            else if (pc.is_fixed_parameter_value) {
                                parm->initializations_count++;
                                parm->source = ParameterSymbol::fixed_parameter;
                            }
                            else {
                                ostringstream msg;
                                msg << LT("error : initializer for parameter ") << parm->name << LT(" not allowed in model source code");
                                error(@eq, msg.str());
                            }
                            parm->initializer_list.push_back($parameter_initializer_element);
                        }
    | "="[eq] "{" parameter_initializer_list[wrk] "}"
                        {
                            // splice the gathered initializer list into the parameter's list
                            auto parm = pc.get_parameter_context();
                            assert(parm); // grammar guarantee
                            if (parm->is_derived()) {
                                ostringstream msg;
                                msg << LT("warning : ignoring initializer for derived parameter ") << parm->name;
                                drv.warning(@eq, msg.str());
                                //TODO should not fall through to initialization code after if/elseif/else below
                            }
                            else if (pc.is_scenario_parameter_value) {
                                parm->initializations_count++;
                                parm->source = ParameterSymbol::scenario_parameter;
                            }
                            else if (pc.is_fixed_parameter_value) {
                                parm->initializations_count++;
                                parm->source = ParameterSymbol::fixed_parameter;
                            }
                            else {
                                ostringstream msg;
                                msg << LT("error : initializer for parameter ") << parm->name << LT(" not allowed in model source code");
                                error(@eq, msg.str());
                            }
                            auto wrk = $wrk; // to see it in the debugger
                            parm->initializer_list.splice(parm->initializer_list.end(), *wrk);
                            delete wrk;
                            $wrk = nullptr;
                        }
    | /* Nothing */
                        {
                            auto parm = pc.get_parameter_context();
                            assert(parm); // grammar guarantee
                            // no initializer, so parameter was declared in model source.
                            parm->is_declared = true;
                        }
    ;


parameter_initializer_list[result]:
    parameter_initializer_element[elem]
                        {
                            // create a new empty constant list,
                            auto result = new list<Constant *>;
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
                            // create a new empty constant list,
                            auto result = new list<Constant *>;
                            assert(result);
                            auto wrk = $wrk;
                            // and append the second list repeatedly to it
                            for (int j = 0; j < stoi($rpt->value()); j++) {
                                (*result).insert((*result).end(), (*wrk).begin(), (*wrk).end());
                            }
                            (*wrk).clear();
                            delete wrk;
                            $wrk = nullptr;
                            $result = result;
                        }
      | parameter_initializer_list[wrk1] "," parameter_initializer_list[wrk2]
                        {
                            // splice the two lists together
                            auto wrk1 = $wrk1;
                            auto wrk2 = $wrk2;
                            (*wrk1).splice((*wrk1).end(), *wrk2);
                            delete wrk2;
                            $wrk2 = nullptr;
                            $result = $wrk1;
                        }
    ;

parameter_initializer_element:
      initializer_constant
                        {
                            // Rule returns a pointer to the constant.
                            $parameter_initializer_element = $initializer_constant;
                        }
    ;

/*
 * entity
 */

decl_entity:
      "entity" SYMBOL[entity]
                        {
                            if ($entity->is_base_symbol()) {
                                // Morph Symbol to EntitySymbol
                                $entity = new EntitySymbol( $entity, @entity);
                            }
                            else {
                                pc.redeclaration = true;
                            }
                            // Set entity context for body of entity declaration
                            pc.set_entity_context( $entity);
                        }
            "{" entity_member_list "}" ";"
                        {
                            // No valid entity context
                            pc.set_entity_context( nullptr );
                        }
            | "entity" "{" error "}" ";"
            ;

entity_member_list:
	  entity_member
	| entity_member_list entity_member
	;

entity_member:
	  decl_simple_attribute
	| decl_entity_array
	| decl_identity_attribute
	| decl_entity_function
	| decl_entity_event
	| decl_hook
    | error ";"
                        {
                            // Error recovery: Prepare to parse another member in an entity declarative island
                            pc.brace_level = 1;
                            pc.parenthesis_level = 0;
                            pc.bracket_level = 0;
                        }
    ;

decl_simple_attribute:
        decl_type_part[type_symbol] SYMBOL[attribute] ";"
                        {
                            if (check_undeclared($attribute, @attribute, drv)) {
                                auto* sym = new SimpleAttributeSymbol($attribute, pc.get_entity_context(), $type_symbol, nullptr, @attribute);
                                assert(sym);
                            }
                        }
      | decl_type_part[type_symbol] SYMBOL[attribute] "=" "{" signed_literal "}" ";"
                        {
                            if (check_undeclared($attribute, @attribute, drv)) {
                                auto* sym = new SimpleAttributeSymbol($attribute, pc.get_entity_context(), $type_symbol, $signed_literal, @attribute);
                                assert(sym);
                            }
                        }
      | decl_type_part[type_symbol] SYMBOL[attribute] "=" "{" SYMBOL[enumerator] "}" ";"
                        {
                            if (check_undeclared($attribute, @attribute, drv)) {
                                auto *sym = new SimpleAttributeEnumSymbol( $attribute, pc.get_entity_context(), $type_symbol, $enumerator, @attribute );
                                assert(sym);
                            }
                        }
    ;

decl_entity_array:
        decl_type_part[type_symbol] SYMBOL[attribute] array_decl_dimension_list ";"
                        {
                            if (check_undeclared($attribute, @attribute, drv)) {
                                // Morph symbol to entity array member symbol
                                auto aam = new EntityArrayMemberSymbol($attribute, pc.get_entity_context(), $type_symbol, @attribute);
                                list<Symbol *> *pls = $array_decl_dimension_list;
                                // Move dimension list to EntityArrayMemberSymbol (transform elements to stable **).
                                for (auto sym : *pls) aam->dimension_list.push_back(sym->stable_pp());
                                pls->clear();
                                delete pls;
                                $array_decl_dimension_list = nullptr;
                            }
                        }
    ;

array_decl_dimension_list: // A non-empty list of dimensions enclosed by []
        "[" SYMBOL[dim] "]"
                        {
                            // start a new symbol list
                            auto * pls = new list<Symbol *>;
                            pls->push_back($dim);
                            $$ = pls;
                        }
      | array_decl_dimension_list[pls] "[" SYMBOL[dim] "]"
                        {
                            // append to existing symbol list
                            $pls->push_back($dim);
                            $$ = $pls;
                        }
    ;

decl_identity_attribute:
        decl_type_part[type_symbol] SYMBOL[attribute] "=" expr_for_attribute ";"
                        {
                            if (check_undeclared($attribute, @attribute, drv)) {
                                auto *sym = new IdentityAttributeSymbol( $attribute, pc.get_entity_context(), $type_symbol, $expr_for_attribute, @attribute );
                                assert(sym);
                            }
                        }
    ;

decl_entity_function:
      decl_type_part[type] SYMBOL "(" decl_func_arg_string[args] ")" ";"
                        {
                            if ($SYMBOL->is_base_symbol()) {
                                string return_type;
                                if ($type == nullptr) {
                                    return_type = "void";
                                }
                                else {
                                    return_type = $type->name;
                                }

                                string arg_list;
                                if ($args == nullptr) {
                                    // empty argument list
                                    arg_list = "";
                                }
                                else {
                                    // argument list assembled into a string
                                    arg_list = *$args;
                                    delete $args;
                                    $args = nullptr;
                                }

                                // argument 5 below (suppress_defn=true) tells omc that the function definition is developer-supplied
                                auto sym = new EntityFuncSymbol( $SYMBOL, pc.get_entity_context(), return_type, arg_list, true, @SYMBOL );
                                assert(sym);
                                sym->is_developer_supplied = true;
                            }
                            else {
                                // redeclaration
                                // ignore redeclaration, but issue warning
                                string msg = LT("warning : ignoring redeclaration of entity function ") + $SYMBOL->name;
                                drv.warning(@SYMBOL, msg);
                            }
                            // Function argument declaration is now complete, so
                            // turn off suppression of SYMBOL recognition.
                            pc.next_word_is_string = false;
                        }
    ;

decl_func_arg_string[result]: // declaration of function arguments, as a string
      decl_func_arg_string[current] decl_func_arg_element[element]
                        {
                            if ($current == nullptr) {
                                // use the string for the element to start the argument string
                                $result = $element; 
                            }
                            else {
                                // append the syntactic element to the complete argument string
                                *$current += " " + *$element;
                                delete $element;
                                $element = nullptr;
                                $result = $current;
                            }
                        }
    | /*nothing*/
                        {
                            // Start building the argument list (starting with nothing)
                            $result = nullptr;
                            // Suppress SYMBOL recognition within the function argument declaration.
                            pc.next_word_is_string = true;
                        }
    ;

// The contents of the argument list are of no use to omc.  Leave any developer errors for the C++ compiler
decl_func_arg_element[result]: // a syntactic element of a function argument list declaration
      STRING[sym]
                        {
                            $result = $sym;
                            // Continue to suppress SYMBOL recognition within the function argument declaration.
                            pc.next_word_is_string = true;
                        }
    | literal[lit]
                        {
                            $result = new string($lit->value());
                            // No other uses of $lit, so delete
                            delete $lit;
                            $lit = nullptr;
                        }
    | decl_func_arg_token[tok]
                        {
                            $result = new string(Symbol::token_to_string((token_type)$tok));
                        }
    ;

// keywords and operators which might be present in the declaration of function arguments
decl_func_arg_token:
      ","
    | "const"
    | "unsigned"
    | "char"
    | "int"
    | "long"
    | "integer"
    | "counter"
    | "real"
    | "float"
    | "double"
    | "::"
    | "<"
    | ">"
    | ">>"
    | "="
    | "-" // to allow unary minus in default argument value
    | "*"
    | "&"
    | "..." // to allow variadic functions
    | "Time"
    | "string"
    ;

decl_entity_event:
      "event" SYMBOL[time_func] "," SYMBOL[implement_func] event_priority_opt[priority] ";"
                        {
                            int event_priority = stoi($priority->value());
                            delete $priority;
                            $priority = nullptr;
                            auto *ent = pc.get_entity_context();
                            // Create entity event symbol
                            string event_name = "om_" + $implement_func->name + "_om_event";
                            auto *sym = new EntityEventSymbol(event_name, ent, $time_func, $implement_func, true, event_priority, @decl_entity_event);
                            assert(sym);
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
                            $event_priority_opt = new IntegerLiteral( to_string(openm::event_priority_time_keeping) );
                        }
    | /* nothing */
                        {
                            // default event priority is zero
                            $event_priority_opt = new IntegerLiteral( "0" );
                        }
    ;

decl_hook:
      "hook"[kw] SYMBOL[from] "," hook_to_symbol[to] hook_order_opt[order_lit] ";"
                        {
                            auto * ent = pc.get_entity_context();
                            int order = stoi($order_lit->value());
                            free($order_lit);

                            if (!EntityHookSymbol::exists(ent, $from, $to)) {
                                // Create the entity hook symbol
                                auto *sym = new EntityHookSymbol(ent, $from, $to, order, @kw);
                                assert(sym);
                                // Use high-level sorting order to control calling order in generated code.
                                // But, add 100 to make all sort after other symbols, so that hooks to ss attributes
                                // are injected at the end of the implement function of the ss event for the
                                // entity (after other symbols have injected their code).
                                sym->sorting_group = 100 + order;
                            }
                            else {
                                // redeclaration
                                string name = EntityHookSymbol::symbol_name( $from , $to);
                                auto hook_sym = dynamic_cast<EntityHookSymbol *>(Symbol::get_symbol(name, ent));
                                assert(hook_sym);
                                if (hook_sym->order == order) {
                                    // redeclaration is benign if hook order is identical in the two declarations
                                }
                                else {
                                    error(@kw, LT("error: invalid redeclaration of hook (order differs)"));
                                }
                            }
                        }
    ;

hook_to_symbol:
      SYMBOL
    | derived_attribute_self_scheduling
    | derived_attribute_trigger
    ;

hook_order_opt:
      "," INTEGER_LITERAL
                        {
                            $hook_order_opt = $INTEGER_LITERAL;
                        }
    | /* nothing */
                        {
                            // default event order is zero
                            $hook_order_opt = new IntegerLiteral( "0" );
                        }
    ;

/*
 * expression for identity attribute
 */

expr_for_attribute[result]:
    //
    // expression terminals
    // 
      symbol_in_expr[sym]
                        {
	                        $result = new ExprForAttributeSymbol( $sym );
                        }
    | literal
                        {
	                        $result = new ExprForAttributeLiteral( $literal );
                            // NB: pointer $literal is used in ExprForAttributeLiteral, so not deleted
                        }
    //
    // arithmetic
    // 
    | "+"[op] expr_for_attribute[right] %prec UNARY_PLUS
                        {
	                        $result = new ExprForAttributeUnaryOp( (token_type) $op, $right );
                        }
    | "-"[op] expr_for_attribute[right] %prec UNARY_MINUS
                        {
	                        $result = new ExprForAttributeUnaryOp( (token_type) $op, $right );
                        }
    | expr_for_attribute[left] "+"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "-"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "*"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "/"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "%"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    //
    // bit-wise operations
    // 
    | expr_for_attribute[left] "|"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "&"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "^"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | "~"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeUnaryOp( (token_type) $op, $right );
                        }
    | expr_for_attribute[left] "<<"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] ">>"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    //
    // logical
    // 
    | "!"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeUnaryOp( (token_type) $op, $right );
                        }
    | expr_for_attribute[left] "&&"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "||"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    //
    // comparison operations
    // 
    | expr_for_attribute[left] "=="[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "!="[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "<"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] ">"[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] "<="[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    | expr_for_attribute[left] ">="[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    //
    // ternary conditional
    // 
    | expr_for_attribute[cond] "?" expr_for_attribute[first] ":" expr_for_attribute[second] 
                        {
	                        $result = new ExprForAttributeTernaryOp( $cond, $first, $second );
                        }
    //
    // array subscripting
    //
    | expr_for_attribute[left] "["[op] expr_for_attribute[right] "]"
                        {
                            // array indexing
                            $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    //
    // comma-separated list
    //
    | expr_for_attribute[left] ","[op] expr_for_attribute[right]
                        {
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, $left, $right );
                        }
    //
    // function-syle cast to numeric type
    //
    | numeric_type "("[op] expr_for_attribute[arg] ")"
                        {
                            // convert numeric_type from a token to a type symbol
                            auto *type_symbol = Symbol::get_symbol(Symbol::token_to_string((token_type)$numeric_type));
                            assert(type_symbol); // grammar/initialization guarantee
	                        auto type = new ExprForAttributeSymbol( type_symbol );
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, type, $arg );
                        }
    //
    // function call, or function-syle cast to a model-specific type, e.g. a classification
    //
    | SYMBOL[sym] "("[op] expr_for_attribute[arg] ")"
                        {
                            // turn the function (or type) symbol into an expression terminal node
	                        auto func = new ExprForAttributeSymbol( $sym );
                            // note that arg can be an argument list because the grammar recognizes ',' as an expression operator
	                        $result = new ExprForAttributeBinaryOp( (token_type) $op, func, $arg );
                        }
    //
    // grouping
    //
    | "(" expr_for_attribute[expr] ")"
                        {
	                        $result = $expr;
                        }
	;


/*
 * the 'type' part of a parameter, attribute or entity function declaration
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
                            // will be disambiguated in post-parse phase
                            $decl_type_part = $type_symbol;
                        }
    | "void"
                        {
                            $$ = nullptr;
                        }
    ;

/*
 * link
 */

decl_link:
          "link" link_symbol[link] ";"
                        {
                            // reciprocal one-to-one link

                            // The containing entity of the link
                            auto ent = $link->entity;
                            // get the type of the link
                            auto type = TypeOfLinkSymbol::get_single($link->entity);
                            // morph EntityMemberSymbol $link to a LinkAttributeSymbol
                            auto lavs = new LinkAttributeSymbol($link, ent, type, $link->decl_loc);
                            lavs->reciprocal_link = lavs; // special case
                        }
        | "link" link_symbol[link1] link_symbol[link2] ";"
                        {
                            // one-to-one link (different entities and/or different link attributes)

                            // The containing entity of each link
                            auto entity1 = $link1->entity;
                            auto entity2 = $link2->entity;
                            // The type of each link, e.g. the type of link1 will point to entity2, like <link entity2>
                            auto type1 = TypeOfLinkSymbol::get_single(entity2);
                            auto type2 = TypeOfLinkSymbol::get_single(entity1);

                            // morph EntityMemberSymbol's to LinkAttributeSymbol's
                            auto lavs1 = new LinkAttributeSymbol($link1, entity1, type1, $link1->decl_loc);
                            auto lavs2 = new LinkAttributeSymbol($link2, entity2, type2, $link2->decl_loc);

                            lavs1->reciprocal_link = lavs2;
                            lavs2->reciprocal_link = lavs1;
                        }
        | "link" link_symbol[link_single] link_symbol[link_multi] "[" "]" ";"
                        {
                            // one-to-many link
                            // The containing entity of each link
                            auto entity_single = $link_single->entity;
                            auto entity_multi = $link_multi->entity;
                            // The type of each link
                            auto type_single = TypeOfLinkSymbol::get_single(entity_multi);
                            auto type_multi = TypeOfLinkSymbol::get_single(entity_single);

                            // morph EntityMemberSymbol's to appropriate link types
                            auto single = new LinkAttributeSymbol($link_single, entity_single, type_single, $link_single->decl_loc);
                            auto multi = new EntityMultilinkSymbol($link_multi, entity_multi, type_multi, $link_multi->decl_loc);

                            // match them up
                            single->reciprocal_multilink = multi;
                            multi->reciprocal_link = single;
                        }
        | "link" link_symbol[link_multi] "[" "]" link_symbol[link_single] ";"
                        {
                            // many-to-one link (identical code to previous rule).
                            // The containing entity of each link
                            auto entity_single = $link_single->entity;
                            auto entity_multi = $link_multi->entity;
                            // The type of each link
                            auto type_single = TypeOfLinkSymbol::get_single(entity_multi);
                            auto type_multi = TypeOfLinkSymbol::get_single(entity_single);

                            // morph EntityMemberSymbol's to appropriate link types
                            auto single = new LinkAttributeSymbol($link_single, entity_single, type_single, $link_single->decl_loc);
                            auto multi = new EntityMultilinkSymbol($link_multi, entity_multi, type_multi, $link_multi->decl_loc);

                            // match them up
                            single->reciprocal_multilink = multi;
                            multi->reciprocal_link = single;
                        }
        | "link" link_symbol[link_multi1] "[" "]" link_symbol[link_multi2] "[" "]" ";"
                        {
                            // many-to-many link
                            // The containing entity of each link
                            auto entity_multi1 = $link_multi1->entity;
                            auto entity_multi2 = $link_multi2->entity;
                            // The type of each link
                            auto type_multi1 = TypeOfLinkSymbol::get_single(entity_multi2);
                            auto type_multi2 = TypeOfLinkSymbol::get_single(entity_multi1);

                            // morph EntityMemberSymbol's to appropriate link types
                            auto multi1 = new EntityMultilinkSymbol($link_multi1, entity_multi1, type_multi1, $link_multi1->decl_loc);
                            auto multi2 = new EntityMultilinkSymbol($link_multi2, entity_multi2, type_multi2, $link_multi2->decl_loc);

                            // match them up
                            multi1->reciprocal_multilink = multi2;
                            multi2->reciprocal_multilink = multi1;
                        }
        | "link" error ";"
        ;

link_symbol:
            SYMBOL[entity] "."
                        {
                            // Set entity context for scanner creation of the immediately following link attribute
                            pc.set_entity_context( $entity );
                        }
            SYMBOL[link]
                        {
                            // Morph Symbol to an EntityMemberSymbol, which will be morphed subsequently to a link symbol
                            auto ams = new EntityMemberSymbol( $link, $entity, @link);
                            $link_symbol = ams;
                        }
        ;

/*
 * entity_set
 */

decl_entity_set: // Some code for decl_entity_set and decl_table is nearly identical
      "entity_set" SYMBOL[entity] SYMBOL[entity_set]
                        {
                            EntitySetSymbol *entity_set = nullptr;

                            if ($entity_set->is_base_symbol()) {
                                // Morph Symbol to EntitySetSymbol
                                entity_set = new EntitySetSymbol( $entity_set, $entity, @entity_set );
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
                            // Set entity context and entity set context for body of entity set declaration
                            pc.set_entity_context( $entity);
                            pc.set_entity_set_context( entity_set );
                            pc.reset_working_counters();
                        }
            entity_set_dimension_list_opt entity_set_filter_opt entity_set_order_opt ";"
                        {
                            // Leaving entity and entity_set context
                            pc.set_entity_set_context( nullptr );
                            pc.set_entity_context( nullptr );
                            pc.reset_working_counters();
                        }
    | "entity_set" error ";"
                        {
                            // Leaving entity and entity_set context
                            pc.set_entity_set_context(nullptr);
                            pc.set_entity_context(nullptr);
                            pc.reset_working_counters();
                        }
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
      name_opt[short_name] "[" symbol_in_table "]"
                        {
                            Symbol *attribute = $symbol_in_table;
                            assert(attribute);

                            auto sym = new DimensionSymbol(pc.get_entity_set_context(), $short_name, pc.counter4, false, attribute, nullptr, false, @symbol_in_table);
                            assert(sym);
                            // add dimension to entity set's dimension_list
                            pc.get_entity_set_context()->dimension_list.push_back(sym);
                            // Increment the counter used for the number of dimensions.
                            // This is the same as as the 0-based ordinal of the next dimension in the declaration.
                            pc.counter4++;
                        }
    ;

entity_set_filter_opt:
    "filter" expr_for_attribute[root]
                        {
                            EntitySetSymbol *entity_set = pc.get_entity_set_context();
                            // create an identity attribute for the filter
                            auto eav = new IdentityAttributeSymbol("om_" + entity_set->name + "_filter", entity_set->entity, BoolSymbol::find(), $root, @root);
                            assert(eav); // out of memory check
                            // note identity attribute in entity set
                            entity_set->filter = eav;
                        }
    | /* nothing */
    ;


entity_set_order_opt:
    "order" SYMBOL[order]
                        {
                            Symbol* order = $order;
                            assert(order);
                            EntitySetSymbol *entity_set = pc.get_entity_set_context();
                            // note order attribute in entity set
                            entity_set->order = order->stable_pp();
                        }
    | /* nothing */
    ;
/*
 * table
 */

decl_table: // Some code for decl_entity_set and decl_table is nearly identical
      "table" SYMBOL[entity] SYMBOL[table] // Note that the symbol 'table' is not created in entity context
                        {
                            EntityTableSymbol *table = nullptr;

                            if ($table->is_base_symbol()) {
                                // Morph Symbol to EntityTableSymbol
                                table = new EntityTableSymbol( $table, $entity, @table );
                                assert(table);
                                $table = table;
                            }
                            else {
                                // re-declaration
                                pc.redeclaration = true;
                                table = dynamic_cast<EntityTableSymbol *>($table);
                                assert(table); // grammar/logic guarantee
                                // redeclaration not allowed
                                error(@table, LT("error: a table named '") + $table->name + LT("' already exists"));
                            }
                            // Set entity context and table context for body of table declaration
                            pc.set_entity_context( $entity);
                            pc.set_table_context( table );
                            pc.reset_working_counters();
                            // working counter1 used for table expressions
                            // working counter2 used for table accumulators
                            // working counter3 used for table attributes
                            // working counter4 used for table classification dimensions
                        }
            table_filter_opt
            "{" table_dimension_list "}" ";"
                        {
                            // Leaving table and entity context
                            pc.set_table_context( nullptr );
                            pc.set_entity_context( nullptr );
                            pc.reset_working_counters();
                        }
    | "table" error ";"
                        {
                            // Leaving table and entity context
                            pc.set_table_context(nullptr);
                            pc.set_entity_context(nullptr);
                            pc.reset_working_counters();
                        }
    ;

table_filter_opt:
    "[" expr_for_attribute[root] "]"
                        {
                            EntityTableSymbol *table = pc.get_table_context();
                            // create an anonymous identity attribute for the filter
                            auto aia = IdentityAttributeSymbol::anonymous_identity_attribute(table->entity, BoolSymbol::find(), $root, @root);
                            aia->is_generated = true;
                            aia->associated_table = table;
                            assert(aia);
                            // note identity attribute in table
                            table->filter = aia;
                        }
    | /* nothing */
    ;

table_dimension_list:
    table_dimension
  | table_dimension_list "*" table_dimension
  ;

table_margin_opt:
      "+"[kw]
                        {
                            $table_margin_opt = $kw;
                        }
    | /* nothing */
                        {
                            $table_margin_opt = token::TK_unused;
                        }
    ;

table_dimension:
    name_opt[short_name] symbol_in_table table_margin_opt
                        {
                            Symbol *attribute = $symbol_in_table;
                            assert(attribute);
                            bool margin_opt = $table_margin_opt == token::TK_PLUS;
                            bool after_analysis_dim = pc.counter1 > 0; // true if the analysis dimension precedes this enumeration dimension

                            auto sym = new DimensionSymbol(pc.get_table_context(), $short_name, pc.counter4, after_analysis_dim, attribute, nullptr, margin_opt, @symbol_in_table);
                            assert(sym);
                            // add dimension to table's dimension_list
                            pc.get_table_context()->dimension_list.push_back(sym);
                            // Increment the counter used for the number of dimensions (excluding analysis dimension).
                            // This is the same as as the 0-based ordinal of the next dimension in the declaration.
                            pc.counter4++;
                        }
    | "{" table_expression_list "}"[closing_brace]
                        {
                            auto table = pc.get_table_context();
                            assert(table);
                            // record analysis dimension position which is the
                            // 0-based ordinal of the immediately preceding classification dimension.
                            table->measures_position = pc.counter4 - 1;
                            // Create the MeasureDimensionSymbol of this table
                            // with location being the closing brace of the measure dimension (used to match label later)
                            table->measure_dimension = new MeasureDimensionSymbol(table, @closing_brace);
                        }
    ;

table_expression_list:
      name_opt[short_name] expr_for_table[root]
                        {
                            auto sym = new EntityTableMeasureSymbol(pc.get_table_context(), $root, $short_name, pc.counter1, @root);
                            assert(sym);
                            pc.counter1++;  // counter for measures
                        }
    | table_expression_list "," name_opt[short_name] expr_for_table[root]
                        {
                            auto sym = new EntityTableMeasureSymbol(pc.get_table_context(), $root, $short_name, pc.counter1, @root);
                            assert(sym);
                            pc.counter1++;  // counter for measures
                        }
	;

/*
 * table expression
 */

expr_for_table[result]:
      // Ex. income
      symbol_in_table
                        {
                            Symbol *attribute = $symbol_in_table;
                            assert(attribute);
                            // Defaults are accumulator=sum, increment=delta, table operator=interval
                            token_type acc = token::TK_sum;
                            token_type incr = token::TK_delta;
                            token_type tabop = token::TK_interval;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(attribute, acc, incr, tabop, pc);
                        }
      // Ex. unit
    | "unit"
                        {
                            // This is the special accumulator which counts increments
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(nullptr, token::TK_unit, token::TK_unused, token::TK_unused, pc);
                        }
      // Ex. max_value_in(income)
    | modgen_cumulation_operator "(" symbol_in_table ")"
                        {
                            Symbol *attribute = $symbol_in_table;
                            // Ex. token::TK_maximum
                            token_type acc = Symbol::modgen_cumulation_operator_to_acc( (token_type) $modgen_cumulation_operator );
                            // Ex. token::TK_value_in
                            token_type incr = Symbol::modgen_cumulation_operator_to_incr( (token_type) $modgen_cumulation_operator );
                            token_type tabop = token::TK_interval;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(attribute, acc, incr, tabop, pc);
                        }
      // Ex. sum(delta(income))
    | table_accumulator[acc] "(" table_increment[incr] "(" symbol_in_table ")" ")"
                        {
                            Symbol *attribute = $symbol_in_table;
                            token_type acc = (token_type) $acc;
                            token_type incr = (token_type) $incr;
                            token_type tabop = token::TK_interval;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(attribute, acc, incr, tabop, pc);
                        }
      // Ex. event(income)
    | table_operator[tabop] "(" symbol_in_table ")"
                        {
                            Symbol *attribute = $symbol_in_table;
                            token_type acc = token::TK_sum;
                            token_type incr = token::TK_delta;
                            token_type tabop = (token_type) $tabop;
                            // The following static helper function is defined in the final section of parser.y (below)
                            $result = table_expr_terminal(attribute, acc, incr, tabop, pc);
                        }
      // Ex. max_value_in(event(income))
    | modgen_cumulation_operator "(" table_operator[tabop] "(" symbol_in_table ")" ")"
                        {
                            Symbol *attribute = $symbol_in_table;
                            // Ex. token::TK_maximum
                            token_type acc = Symbol::modgen_cumulation_operator_to_acc( (token_type) $modgen_cumulation_operator );
                            // Ex. token::TK_value_in
                            token_type incr = Symbol::modgen_cumulation_operator_to_incr( (token_type) $modgen_cumulation_operator );
                            token_type tabop = (token_type) $tabop;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(attribute, acc, incr, tabop, pc);
                        }
      // Ex. sum(delta(event(income)))
    | table_accumulator[acc] "(" table_increment[incr] "(" table_operator[tabop] "(" symbol_in_table ")" ")" ")"
                        {
                            Symbol *attribute = $symbol_in_table;
                            token_type acc = (token_type) $acc;
                            token_type incr = (token_type) $incr;
                            token_type tabop = (token_type) $tabop;
                            // The following static helper function is defined in the final section of parser.y
                            $result = table_expr_terminal(attribute, acc, incr, tabop, pc);
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
    | "sqrt"[op] "(" expr_for_table[right] ")"
                        {
                            $result = new ExprForTableOp((token_type)$op, nullptr, $right);
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
    | TK_minimum
    | TK_maximum
    | TK_P1
    | TK_P2
    | TK_P5
    | TK_P10
    | TK_P20
    | TK_P25
    | TK_P30
    | TK_P40
    | TK_P50
    | TK_P60
    | TK_P70
    | TK_P75
    | TK_P80
    | TK_P90
    | TK_P95
    | TK_P98
    | TK_P99
    | TK_gini
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
 * derived_table
 */

decl_derived_table:
      "derived_table" SYMBOL[derived_table]
                        {
                            DerivedTableSymbol *derived_table = nullptr;

                            if ($derived_table->is_base_symbol()) {
                                // Morph Symbol to DerivedTableSymbol
                                derived_table = new DerivedTableSymbol( $derived_table, @derived_table );
                                assert(derived_table);
                                $derived_table = derived_table;
                            }
                            else {
                                // re-declaration
                                pc.redeclaration = true;
                                derived_table = dynamic_cast<DerivedTableSymbol *>($derived_table);
                                assert(derived_table); // grammar/logic guarantee
                                // redeclaration not allowed
                                error(@derived_table, LT("error: a derived table named '") + derived_table->name + LT("' already exists"));
                            }
                            // Set derived table context for body of derived table declaration
                            pc.set_derived_table_context( derived_table );
                            pc.reset_working_counters();
                            // working counter1 used for derived table measures
                            // working counter4 used for derived table classification dimensions
                        }
            "{" derived_table_dimension_list "}" ";"
                        {
                            // Leaving derived table context
                            pc.set_derived_table_context( nullptr );
                            pc.reset_working_counters();
                        }
    | "derived_table" error ";"
                        {
                            // Leaving derived table context
                            pc.set_derived_table_context(nullptr);
                            pc.reset_working_counters();
                        }
    ;

derived_table_dimension_list:
      derived_table_dimension
    | derived_table_dimension_list "*" derived_table_dimension
  ;

derived_table_dimension:
      name_opt[short_name] SYMBOL[enumeration] table_margin_opt
                        {
                            // This block similar to table_dimension block for native tables
                            Symbol *enumeration = $enumeration;
                            assert(enumeration);
                            bool margin_opt = $table_margin_opt == token::TK_PLUS;
                            bool after_analysis_dim = pc.counter1 > 0; // true if the analysis dimension precedes this enumeration dimension

                            auto sym = new DimensionSymbol(pc.get_derived_table_context(), $short_name, pc.counter4, after_analysis_dim, nullptr, enumeration, margin_opt, @enumeration);
                            assert(sym);
                            // add dimension to derived table's dimension_list
                            pc.get_derived_table_context()->dimension_list.push_back(sym);
                            // Increment the counter used for the number of dimensions (excluding analysis dimension).
                            // This is the same as as the 0-based ordinal of the next dimension in the declaration.
                            pc.counter4++;
                        }
    | "{" 
                        {
                            // measure short_name follows
                            pc.next_word_is_string = true;
                        }
      derived_table_measure_list "}"[closing_brace]
                        {
                            auto table = pc.get_derived_table_context();
                            assert(table);
                            // record measures position which is the
                            // 0-based ordinal of the immediately preceding classification dimension.
                            table->measures_position = pc.counter4 - 1;
                            // Create the MeasureDimensionSymbol of this table
                            // with location being the closing brace of the measure dimension (used to match label later)
                            table->measure_dimension = new MeasureDimensionSymbol(table, @closing_brace);
      }
    ;

derived_table_measure_list:
      STRING[short_name_default]
                        {
                            auto sym = new TableMeasureSymbol(pc.get_derived_table_context(), $short_name_default, nullptr, pc.counter1, @short_name_default);
                            assert(sym);
                            delete $short_name_default; // delete the string created using new in scanner
                            $short_name_default = nullptr;
                            pc.counter1++;  // counter for short_names
                        }
    | derived_table_measure_list ","
                        {
                            // short_name follows
                            pc.next_word_is_string = true;
                        }
      STRING[short_name_default]
                        {
                            auto sym = new TableMeasureSymbol(pc.get_derived_table_context(), $short_name_default, nullptr, pc.counter1, @short_name_default);
                            assert(sym);
                            delete $short_name_default; // delete the string created using new in scanner
                            $short_name_default = nullptr;
                            pc.counter1++;  // counter for measures
                        }
	;



/*
 * extend_parameter
 */

decl_extend_parameter:
	  "extend_parameter"[tok] SYMBOL[target_parameter] ";"
                        {
                            auto sym = new ExtendParameterSymbol($target_parameter, nullptr, @tok);
                            assert(sym);
                        }
	| "extend_parameter"[tok] SYMBOL[target_parameter] SYMBOL[index_parameter] ";"
                        {
                            auto sym = new ExtendParameterSymbol($target_parameter, $index_parameter, @tok);
                            assert(sym);
                        }
	| "extend_parameter" error ";"
    ;



/*
 * symbol in an expression
 */

symbol_in_expr:
      SYMBOL
    | derived_attribute
    | link_to_attribute
	;

link_to_attribute:
      SYMBOL[link] "->"
                        {
                            // Tell the scanner not to apply entity scope resolution to the following 'word'
                            // in the input stream, but just return a STRING instead.  That's because the entity context
                            // depends on the nature of the symbol on the left of "->", whose declaration may not yet have been encountered.
                            pc.next_word_is_string = true;
                        }
            STRING[attribute]
                        {
                            // Create LinkToAttributeSymbol.  Store the r.h.s of pointer operator as a string for
                            // subsequent post-parse resolution.
                            $link_to_attribute = LinkToAttributeSymbol::create_symbol(pc.get_entity_context(), $link, *$attribute);
                            delete $attribute; // delete the string created using new in scanner
                            $attribute = nullptr;
                        }
        ;

/*
 * symbol used as a dimension or table measure
 */

symbol_in_table:
      SYMBOL
    | derived_attribute
    | link_to_attribute_shadowed
	;

/*
 * link to attribute shadowed by a generated identity attribute
 */

link_to_attribute_shadowed:
      SYMBOL[link] "->"
                        {
                            // Tell the scanner not to apply entity scope resolution to the following 'word'
                            // in the input stream, but just return a STRING instead.  That's because the entity context
                            // depends on the nature of the symbol on the left of "->", whose declaration may not yet have been encountered.
                            pc.next_word_is_string = true;
                        }
            STRING[attribute]
                        {
                            // Create an anonymous identity attribute to shadow the linked attribute.
                            // The type of the anonymous identity attribute is set to "unknown type",
                            // which will be resolved subsequently during post-parse processing.
                            auto entity = pc.get_entity_context();

                            // create the link to attribute symbol, which will become the expression of the anonymous identity attribute
                            auto link_to_attribute = LinkToAttributeSymbol::create_symbol(entity, $link, *$attribute);
                            assert(link_to_attribute);
                            delete $attribute; // delete the string created using new in scanner
                            $attribute = nullptr;

                            // Create expression tree consisting of the terminal node
                            auto expr = new ExprForAttributeSymbol(link_to_attribute);

                            // Create the anonymous link to attribute symbol,
                            // whose expression is just the single terminal
                            // the link_to_attribute created immediately above.
                            // The data_type is unknown (it will be resolved post-parse).
                            auto aia = IdentityAttributeSymbol::anonymous_identity_attribute(entity, UnknownTypeSymbol::find(), expr, @attribute);
                            assert(aia);

                            // return the anonymous identity attribute which shadows the link to attribute
                            $link_to_attribute_shadowed = aia;
                        }
        ;

/*
 * derived attributes
 */

any_attribute:
      SYMBOL
    | derived_attribute
    | link_to_attribute_shadowed
        ;

derived_attribute:
      derived_attribute_self_scheduling
    | derived_attribute_trigger
    | derived_attribute_other
        ;

derived_attribute_self_scheduling:
    /*
     * derived attributes - self-scheduling family
     */
      TK_duration_counter[kw] "(" any_attribute[observed] "," constant[constnt] "," constant[interval] ")"
                        {
                            $derived_attribute_self_scheduling = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constnt, $interval);
                        }
    | TK_duration_counter[kw] "(" any_attribute[observed] "," constant[constnt] "," constant[interval]  "," constant[maxcount] ")"
                        {
                            $derived_attribute_self_scheduling = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constnt, $interval, $maxcount);
                        }
    | TK_duration_trigger[kw] "(" any_attribute[observed] "," constant[constnt] "," constant[delay] ")"
                        {
                            $derived_attribute_self_scheduling = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constnt, $delay);
                        }
    | TK_self_scheduling_int[kw] "(" any_attribute[observed] ")"
                        {
                            $derived_attribute_self_scheduling = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed);
                        }
    | TK_self_scheduling_split[kw] "(" any_attribute[observed] "," SYMBOL[partition] ")"
                        {
                            $derived_attribute_self_scheduling = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, static_cast<Symbol *>(nullptr), $partition);
                        }
        ;

derived_attribute_trigger:
    /*
     * derived attributes - trigger family
     */
      TK_trigger_entrances[kw] "(" any_attribute[observed] "," constant ")"
                        {
                            $derived_attribute_trigger = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant);
                        }
    | TK_trigger_exits[kw] "(" any_attribute[observed] "," constant ")"
                        {
                            $derived_attribute_trigger = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant);
                        }
    | TK_trigger_transitions[kw] "(" any_attribute[observed] "," constant[from] "," constant[to] ")"
                        {
                            $derived_attribute_trigger = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $from, $to);
                        }
    | TK_trigger_changes[kw] "(" any_attribute[observed] ")"
                        {
                            $derived_attribute_trigger = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed);
                        }
        ;

derived_attribute_other:
    /*
     * derived attributes - duration family
     */
      TK_duration[kw] "(" ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw);
                        }
    | TK_duration[kw] "(" any_attribute[observed] "," constant ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant);
                        }
    | TK_weighted_duration[kw] "(" any_attribute[weight] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, static_cast<Symbol *>(nullptr), static_cast<ConstantSymbol *>(nullptr), $weight);
                        }
    | TK_weighted_duration[kw] "(" any_attribute[observed] "," constant "," any_attribute[weight] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant, $weight);
                        }
    | TK_weighted_cumulation[kw] "(" any_attribute[observed] "," any_attribute[weight] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $weight);
                        }

    /*
     * derived attributes - spell family
     */
    | TK_active_spell_duration[kw] "(" any_attribute[spell] "," constant ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $spell, $constant);
                        }
    | TK_completed_spell_duration[kw] "(" any_attribute[spell] "," constant ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $spell, $constant);
                        }
    | TK_active_spell_weighted_duration[kw] "(" any_attribute[spell] "," constant "," any_attribute[weight]  ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $spell, $constant, $weight);
                        }
    | TK_completed_spell_weighted_duration[kw] "(" any_attribute[spell] "," constant "," any_attribute[weight]  ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $spell, $constant, $weight);
                        }
    | TK_active_spell_delta[kw] "(" any_attribute[spell] "," constant "," any_attribute[delta]  ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $spell, $constant, $delta);
                        }
    | TK_completed_spell_delta[kw] "(" any_attribute[spell] "," constant "," any_attribute[delta]  ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $spell, $constant, $delta);
                        }

    /*
     * derived attributes - transition occurrence family
     */
    | TK_undergone_entrance[kw] "(" any_attribute[observed] "," constant ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant);
                        }
    | TK_undergone_exit[kw] "(" any_attribute[observed] "," constant ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant);
                        }
    | TK_undergone_transition[kw] "(" any_attribute[observed] "," constant[from] "," constant[to] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $from, $to);
                        }
    | TK_undergone_change[kw] "(" any_attribute[observed] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed);
                        }

    /*
     * derived attributes - transition count family
     */
    | TK_entrances[kw] "(" any_attribute[observed] "," constant ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant);
                        }
    | TK_exits[kw] "(" any_attribute[observed] "," constant ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant);
                        }
    | TK_transitions[kw] "(" any_attribute[observed] "," constant[from] "," constant[to] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $from, $to);
                        }
    | TK_changes[kw] "(" any_attribute[observed] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed);
                        }

    /*
     * derived attributes - transition observer family - value
     */
    | TK_value_at_first_entrance[kw] "(" any_attribute[observed] "," constant "," any_attribute[value] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant, $value);
                        }
    | TK_value_at_latest_entrance[kw] "(" any_attribute[observed] "," constant "," any_attribute[value] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant, $value);
                        }
    | TK_value_at_first_exit[kw] "(" any_attribute[observed] "," constant "," any_attribute[value] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant, $value);
                        }
    | TK_value_at_latest_exit[kw] "(" any_attribute[observed] "," constant "," any_attribute[value] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant, $value);
                        }
    | TK_value_at_first_transition[kw] "(" any_attribute[observed] "," constant[from] "," constant[to] "," any_attribute[value]  ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $from, $to, $value);
                        }
    | TK_value_at_latest_transition[kw] "(" any_attribute[observed] "," constant[from] "," constant[to] "," any_attribute[value]  ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $from, $to, $value);
                        }
    | TK_value_at_first_change[kw] "(" any_attribute[observed] "," any_attribute[value] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $value);
                        }
    | TK_value_at_latest_change[kw] "(" any_attribute[observed] "," any_attribute[value] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $value);
                        }

    /*
     * derived attributes - transition observer family - sum
     */
    | TK_value_at_entrances[kw] "(" any_attribute[observed] "," constant "," any_attribute[value] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant, $value);
                        }
    | TK_value_at_exits[kw] "(" any_attribute[observed] "," constant "," any_attribute[value] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $constant, $value);
                        }
    | TK_value_at_transitions[kw] "(" any_attribute[observed] "," constant[from] "," constant[to] "," any_attribute[value]  ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $from, $to, $value);
                        }
    | TK_value_at_changes[kw] "(" any_attribute[observed] "," any_attribute[value] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, $value);
                        }

    /*
     * derived attributes - transformation family
     */
    | TK_split[kw] "(" any_attribute[observed] "," SYMBOL[partition] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, static_cast<Symbol *>(nullptr), $partition);
                        }
    | TK_aggregate[kw] "(" any_attribute[observed] "," SYMBOL[classification] ")"
                        {
                            $derived_attribute_other = DerivedAttributeSymbol::create_symbol(pc.get_entity_context(), (token_type)$kw, $observed, static_cast<Symbol *>(nullptr), static_cast<Symbol *>(nullptr), $classification);
                        }

    /*
     * derived attributes - multilink family
     */
    | TK_count[function] "(" SYMBOL[multilink] ")"
                        {
                            $derived_attribute_other = MultilinkAttributeSymbol::create_symbol( pc.get_entity_context(), (token_type)$function, $multilink, "" );
                        }
    | aggregate_multilink_function[function] "(" SYMBOL[multilink] ","
                        {
                            // Tell the scanner not to apply entity scope resolution to the following 'word'
                            // in the input stream, but just return a STRING instead.  That's because the entity context
                            // depends on the nature of the preceding multilink symbol, whose declaration may not yet have been encountered.
                            pc.next_word_is_string = true;
                        }
        STRING[attribute] ")"
                        {
                            $derived_attribute_other = MultilinkAttributeSymbol::create_symbol( pc.get_entity_context(), (token_type)$function, $multilink, *$attribute );
                            delete $attribute; // delete the string created using new in scanner
                            $attribute = nullptr;
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
      signed_literal[lit]
                        {
                            // Create a constant representing a literal
                            auto cs = new ConstantSymbol($lit, @lit);
                            assert(cs);
                            // NB: pointer $lit is used in ConstantSymbol, so not deleted
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

initializer_constant:
      signed_literal[lit]
                        {
                            // Create a constant which wraps the literal
                            auto cs = new Constant($lit, @lit);
                            assert(cs);
                            // NB: pointer $lit is used in Constant, so not deleted
                            $initializer_constant = cs;
                        }
    | SYMBOL[enumerator]
                        {
                            // Create a constant representing an enumerator
                            auto cs = new Constant($enumerator, @enumerator);
                            assert(cs);
                            $initializer_constant = cs;
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
                            $INTEGER_LITERAL = nullptr;
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
                            $FLOATING_POINT_LITERAL = nullptr;
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

// Helper function to handle prohibited redeclaration
static bool check_undeclared(Symbol* sym, const yy::parser::location_type& loc, Driver &drv)
{
    if (sym->is_base_symbol()) {
        return true;
    }
    else {
        auto dl = sym->decl_loc; // previous declaration location of sym
        std::string msg =
            LT("error : redeclaration of '")
            + sym->name
            + LT("' - other declaration at ")
            + dl.begin.filename->c_str()
            + "(" + std::to_string(dl.begin.line) + ")";
        drv.error(loc, msg);
        return false;
    }
}

// Helper function to process terminal in table expressions
static ExprForTableAccumulator * table_expr_terminal(Symbol *attribute, token_type acc, token_type incr, token_type table_op, ParseContext & pc)
{
    Symbol *table = pc.get_table_context();
    EntityTableMeasureAttributeSymbol *analysis_attribute = nullptr;
    if (attribute) {
        // Also create symbol for associated analysis attribute if not already present
        if ( EntityTableMeasureAttributeSymbol::exists( table, attribute) ) {
            string unique_name = EntityTableMeasureAttributeSymbol::symbol_name( table, attribute);
            analysis_attribute = dynamic_cast<EntityTableMeasureAttributeSymbol *>(Symbol::get_symbol( unique_name ));
            assert( analysis_attribute );
        }
        else {
            analysis_attribute = new EntityTableMeasureAttributeSymbol( table, attribute, pc.counter3);
            pc.counter3++;
        }
        // determine if the increment requires the creation & maintenance of an associated 'in' member.
        if (   incr == token::TK_delta
            || incr == token::TK_delta2
            || incr == token::TK_nz_delta
            || incr == token::TK_value_in
            || incr == token::TK_value_in2
            || incr == token::TK_nz_value_in ) {

            if (table_op == token::TK_interval) {
                analysis_attribute->need_value_in = true;
            }
            else if (table_op == token::TK_event) {
                analysis_attribute->need_value_in_event = true;
            }
            else {
                assert(false); // logic guarantee
            }
        }
    }
    else {
        // if no attribute, then this is the increment counter accumulator (unit)
        assert(acc == token::TK_unit);
    }
    // Also create symbol for associated accumulator if not already present
    EntityTableAccumulatorSymbol *accumulator = nullptr;
    if ( EntityTableAccumulatorSymbol::exists( table, acc, incr, table_op, attribute) ) {
        string unique_name = EntityTableAccumulatorSymbol::symbol_name( table, acc, incr, table_op, attribute );
        accumulator = dynamic_cast<EntityTableAccumulatorSymbol *>(Symbol::get_symbol( unique_name ));
        assert( accumulator );
    }
    else {
        accumulator = new EntityTableAccumulatorSymbol( table, acc, incr, table_op, attribute, analysis_attribute, pc.counter2);
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
    // modify bison-generated message to help VS detect error message
    std::string m2;
    const std::string s1("syntax error, unexpected");
    const std::string s2("syntax error : unexpected");
    if (0 == m.compare(0, s1.length(), s1)) {
        m2 = s2 + m.substr(s1.length());
    }
    else {
        m2 = m;
    }
     
    drv.error (l, m2);
}



