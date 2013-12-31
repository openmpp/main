%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "2.6"
%defines
%define parser_class_name "parser"

// The following code is written to the header file, not the implementation file
%code requires {

// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

// forward declarations
class Driver;
class ParseContext;
class Literal;
class IntegerLiteral;
class CharacterLiteral;
class FloatingPointLiteral;
class StringLiteral;
class Symbol;
class ExprForTable;

}

// The following code is written to the implementation file, not the header file
%code {
#include "Driver.h"
#include "ParseContext.h"
#include "token.h" 
#include "ast.h"
#include <stdarg.h>
#include <cassert>
#include "libopenm/omCommon.h"
// last item found by flex.  Used in grammar to provide error information
// TODO check and eliminate use
extern char *yytext;
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
  @$.begin.filename = @$.end.filename = &drv.file;
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
    Literal              *pval_Literal;
    IntegerLiteral       *pval_IntegerLiteral;
    CharacterLiteral     *pval_CharacterLiteral;
    FloatingPointLiteral *pval_FloatingPointLiteral;
    StringLiteral        *pval_StringLiteral;
	Symbol               *pval_Symbol;

	ExprForTable         *pval_TableExpr;
};

%printer { yyoutput << "token " << token_to_string((token_type)$$); } <val_token>
%printer { yyoutput << "literal " << $$->cxx_token; } <pval_Literal>
%printer { yyoutput << "integer literal " << $$->cxx_token; } <pval_IntegerLiteral>
%printer { yyoutput << "character literal " << $$->cxx_token; } <pval_CharacterLiteral>
%printer { yyoutput << "floating point literal " << $$->cxx_token; } <pval_FloatingPointLiteral>
%printer { yyoutput << "string literal " << $$->cxx_token; } <pval_StringLiteral>
%printer { yyoutput << "symbol " << '"' << $$->name << '"' << " type=" << typeid($$).name(); } <pval_Symbol>
%printer { yyoutput << "table expr "; } <pval_TableExpr>
%printer { yyoutput << "initial value type=" << token_to_string($$->associated_token); } <pval_InitialValue>

%destructor { delete $$; } <pval_IntegerLiteral> <pval_CharacterLiteral> <pval_FloatingPointLiteral> <pval_StringLiteral>


// NB: There is an exact one-to-one correspondnce with code in parser_helper.cpp

// top-level om keywords, in alphabetic order
%token <val_token>    TK_agent          "agent"
%token <val_token>    TK_agent_set      "agent_set"
%token <val_token>    TK_aggregation    "aggregation"
%token <val_token>    TK_classification "classification"
%token <val_token>    TK_counter_type   "counter_type"
%token <val_token>    TK_dependency     "dependency"
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
%token <val_token>    TK_user_table     "user_table"
%token <val_token>    TK_version        "version"

// body-level om keywords, in alphabetic order
%token <val_token>    TK_active_spell_delta        "active_spell_delta"
%token <val_token>    TK_active_spell_duration     "active_spell_duration"
%token <val_token>    TK_active_spell_weighted_duration  "active_spell_weighted_duration"
%token <val_token>    TK_agent_id                  "agent_id"
%token <val_token>    TK_aggregate                 "aggregate"
%token <val_token>    TK_all_base_states           "all_base_states"
%token <val_token>    TK_all_derived_states        "all_derived_states"
%token <val_token>    TK_all_internal_states       "all_internal_states"
%token <val_token>    TK_all_links                 "all_links"
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

%token <pval_Symbol>               SYMBOL
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

%type  <val_token>      ModelType
%type  <val_token>      ParameterType
%type  <val_token>      ModgenCumulationOperator
%type  <val_token>      TableAccumulator
%type  <val_token>      TableIncrement
%type  <val_token>      FundamentalType
%type  <val_token>      BoolLiteral
%type  <pval_Literal>   AnyLiteral

%type  <pval_TableExpr> ExprForTable
%type  <pval_TableExpr> TableExpressions

%type  <pval_Symbol>   AgentVar
%type  <pval_Symbol>   DerivedAgentVar


%token END      0 "end of file"



%%

%start Model;

Model:
Model Declaration
	| /* nothing */
	;

Declaration:
	  Decl_languages
	| Decl_model_type
	| Decl_time_type
	| Decl_real_type
	| Decl_counter_type
	| Decl_integer_type
	| Decl_version
	| Decl_parameters
	| Decl_agent
	| Decl_table
	;

Decl_languages:
          "languages" "{" Languages "}" ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
        | "languages" "{" error "}" ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
        | "languages" error ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
        ;

Languages:
      SYMBOL
                        {
                            // first language is default language
                            // morph existing symbol to LanguageSymbol
                            auto *sym = new LanguageSymbol( $SYMBOL, true );
                        }
	| Languages "," SYMBOL
                        {
                            auto *sym = new LanguageSymbol( $SYMBOL, false );
                        }
	;

Decl_time_type:
	  "time_type" FundamentalType[type_to_use] ";"
                        {
                            auto *sym = new TypeDeclSymbol( token::TK_Time, (token_type) $type_to_use );
                        }
    | "time_type" error ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
    ;

Decl_real_type:
      "real_type" FundamentalType[type_to_use] ";"
                        {
                            auto *sym = new TypeDeclSymbol( token::TK_real, (token_type) $type_to_use );
                        }
    | "real_type" error ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
    ;

Decl_counter_type:
	  "counter_type" FundamentalType[type_to_use] ";"
						{
                            auto *sym = new TypeDeclSymbol( token::TK_counter, (token_type) $type_to_use );
						}
    | "counter_type" error ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
    ;

Decl_integer_type:
      "integer_type" FundamentalType[type_to_use] ";"
                        {
                            auto *sym = new TypeDeclSymbol( token::TK_integer, (token_type) $type_to_use );
                        }
    | "integer_type" error ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
    ;

Decl_model_type:
        "model_type" ModelType[type_to_use] ";"
                        {
                            auto *sym = new TypeDeclSymbol( token::TK_model_type, (token_type) $type_to_use );
                        }
      | "model_type" error ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
      ;

ModelType:
	  "case_based"
	| "time_based"
	;

Decl_version:
        "version" INTEGER_LITERAL[major] "," INTEGER_LITERAL[minor] "," INTEGER_LITERAL[sub_minor] "," INTEGER_LITERAL[sub_sub_minor] ";"
                        {
                            int major = stoi( $major->cxx_token );
                            delete $major;

                            int minor = stoi( $minor->cxx_token );
                            delete $minor;

                            int sub_minor = stoi( $sub_minor->cxx_token );
                            delete $sub_minor;

                            int sub_sub_minor = stoi( $sub_sub_minor->cxx_token );
                            delete $sub_sub_minor;

                            auto *sym = new VersionSymbol( major, minor, sub_minor, sub_sub_minor );
                        }
      | "version" error ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
    ;

Decl_parameters:
	  "parameters" "{" Parameters "}" ";"
	| "parameters" "{" error "}" ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
      ;

Parameters: 
	  Decl_parameter
	| Parameters Decl_parameter
	;

Decl_parameter:
      ParameterType[type] SYMBOL ";"
                        {
                            auto *sym = new ParameterSymbol( $SYMBOL, (token_type)$type );
                        }
	| error ";"
                        {
                            // Error recovery: Prepare to parse another parameter in a parameters declarative island
                            pc.brace_level = 1;
                            pc.parenthesis_level = 0;
                            pc.bracket_level = 0;
                        }
	;

ParameterType:
	  "int"
	| "double"
	;

Decl_agent:
      "agent" SYMBOL[agent]
                        {
                            // Morph Symbol (or AgentSymbol) to AgentSymbol
                            // Note that re-morphing of agent symbols is harmless
                            $agent = new AgentSymbol( $agent );
                            // Set agent context for body of agent declaration
                            pc.set_agent_context( $agent );
                        }
            "{" AgentMembers "}" ";"
                        {
                            // No valid agent context
                            pc.set_agent_context( nullptr );
                        }
            | "agent" "{" error "}" ";"
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
            ;

AgentMembers:
	  AgentMember
	| AgentMembers AgentMember
	;

AgentMember:
	  Decl_SimpleAgentVar
	| Decl_AgentFunction
	| Decl_AgentEvent
    | error ";"
                        {
                            // Error recovery: Prepare to parse another member in an agent declarative island
                            pc.brace_level = 1;
                            pc.parenthesis_level = 0;
                            pc.bracket_level = 0;
                        }
    ;

Decl_SimpleAgentVar:
      FundamentalType[type] SYMBOL ";"
                        {
                            auto *sym = new SimpleAgentVarSymbol( $SYMBOL, pc.get_agent_context(), (token_type)$type, nullptr );
                        }
    | FundamentalType[type] SYMBOL "=" "{" AnyLiteral "}" ";"
                        {
                            auto *sym = new SimpleAgentVarSymbol( $SYMBOL, pc.get_agent_context(), (token_type)$type, $AnyLiteral );
                        }
    ;

FundamentalType:
      "int"
    | "double"
    | "bool"
	;

BoolLiteral:
      "true"
    | "false"
	;

AnyLiteral:
      INTEGER_LITERAL
                        {
                            $AnyLiteral = $INTEGER_LITERAL;
                        }
    | CHARACTER_LITERAL
                        {
                            $AnyLiteral = $CHARACTER_LITERAL;
                        }
    | FLOATING_POINT_LITERAL
                        {
                            $AnyLiteral = $FLOATING_POINT_LITERAL;
                        }
    | STRING_LITERAL
                        {
                            $AnyLiteral = $STRING_LITERAL;
                        }
    | BoolLiteral
                        {
                            $AnyLiteral = new BooleanLiteral( token_to_string( (token_type) $BoolLiteral ) );
                        }
    | "nullptr"
                        {
                            $AnyLiteral = new PointerLiteral( token_to_string( token::TK_nullptr ) );
                        }
    | "time_infinite"
                        {
                            $AnyLiteral = new TimeLiteral( token_to_string( token::TK_time_infinite ) );
                        }
    | "time_undef"
                        {
                            $AnyLiteral = new TimeLiteral( token_to_string( token::TK_time_undef ) );
                        }
    ;

Decl_AgentFunction:
      "void" SYMBOL "(" ")" ";"
                        {
                            auto sym = new AgentFuncSymbol( $SYMBOL, pc.get_agent_context() );
                        }
    ;

Decl_AgentEvent:
      "event" SYMBOL[time_func] "," SYMBOL[implement_func] ";"
                        {
                            auto *agent = pc.get_agent_context();
                            // create agent event symbol
                            auto *event = new AgentEventSymbol( $implement_func, agent, $time_func );
                            // also create associated AgentEventTimeSymbol
                            string member_name = AgentEventTimeSymbol::member_name( event );
                            auto *sym = new AgentEventTimeSymbol( member_name, agent, event );
                        }
    ;

Decl_table:
      "table" SYMBOL[agent] SYMBOL[table] 
                        {
                            // morph $table to TableSymbol
                            $table = new TableSymbol( $table, $agent );
                            // Set agent context and table context for body of table declaration
                            pc.set_agent_context( $agent );
                            pc.set_table_context( $table );
                            // initialize working counter used for table expressions
                            pc.counter1 = 0;
                            // initialize working counter used for table accumulators
                            pc.counter2 = 0;
                            // initialize working counter used for table agentvars
                            pc.counter3 = 0;
                        }
            "{" "{" TableExpressions[expr1] "}" "}" ";"
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
                        {
                            // Error recovery: Prepare to parse outermost code - C++ or an openm declarative island
                            pc.InitializeForCxxOutside();
                        }
    ;

TableExpressions:
      ExprForTable[root]
                        {
                            auto sym = new TableExpressionSymbol( pc.get_table_context(), $root, pc.counter1 );
                            pc.counter1++;  // counter for expressions
                        }
    | TableExpressions "," ExprForTable[root]
                        {
                            auto sym = new TableExpressionSymbol( pc.get_table_context(), $root, pc.counter1 );
                            pc.counter1++;  // counter for expressions
                        }
	;

ExprForTable[result]:
      AgentVar
                        {
                            Symbol *agentvar = $AgentVar;
                            Symbol *table = pc.get_table_context();
                            // The default accumulator is sum and the default increment is delta
                            token_type acc = token::TK_sum;
                            token_type incr = token::TK_delta;

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
                            // determine if the increment requires the creation & maintenance of an assoicated 'in' agent member.
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
                            if ( TableAccumulatorSymbol::exists( table, acc, incr, agentvar) ) {
                                string unique_name = TableAccumulatorSymbol::symbol_name( table, acc, incr, agentvar );
                                accumulator = dynamic_cast<TableAccumulatorSymbol *>(Symbol::get_symbol( unique_name ));
                                assert( accumulator );
                            }
                            else {
                                accumulator = new TableAccumulatorSymbol( table, acc, incr, agentvar, analysis_agentvar, pc.counter2);
                                pc.counter2++;
                            }
	                        $result = new ExprForTableLeaf( accumulator );
                        }
    | ModgenCumulationOperator "(" AgentVar ")"
                        {
                            Symbol *agentvar = $AgentVar;
                            Symbol *table = pc.get_table_context();
                            token_type acc = modgen_cumulation_operator_to_acc( (token_type) $ModgenCumulationOperator );
                            token_type incr = modgen_cumulation_operator_to_incr( (token_type) $ModgenCumulationOperator );

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
                            // determine if the increment requires the creation & maintenance of an assoicated 'in' agent member.
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
                            if ( TableAccumulatorSymbol::exists( table, acc, incr, agentvar) ) {
                                string unique_name = TableAccumulatorSymbol::symbol_name( table, acc, incr, agentvar );
                                accumulator = dynamic_cast<TableAccumulatorSymbol *>(Symbol::get_symbol( unique_name ));
                                assert( accumulator );
                            }
                            else {
                                accumulator = new TableAccumulatorSymbol( table, acc, incr, agentvar, analysis_agentvar, pc.counter2);
                                pc.counter2++;
                            }
	                        $result = new ExprForTableLeaf( accumulator );
                        }
    | TableAccumulator[acc] "(" TableIncrement[incr] "(" AgentVar ")" ")"
                        {
                            Symbol *agentvar = $AgentVar;
                            Symbol *table = pc.get_table_context();
                            token_type acc = (token_type) $acc;
                            token_type incr = (token_type) $incr;

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
                            if ( TableAccumulatorSymbol::exists( table, acc, incr, agentvar) ) {
                                string unique_name = TableAccumulatorSymbol::symbol_name( table, acc, incr, agentvar );
                                accumulator = dynamic_cast<TableAccumulatorSymbol *>(Symbol::get_symbol( unique_name ));
                                assert( accumulator );
                            }
                            else {
                                accumulator = new TableAccumulatorSymbol( table, acc, incr, agentvar, analysis_agentvar, pc.counter2);
                                pc.counter2++;
                            }
	                        $result = new ExprForTableLeaf( accumulator );
                        }
    | ExprForTable[left] "+"[op] ExprForTable[right]
                        {
	                        $result = new ExprForTableOp( (token_type) $op, $left, $right );
                        }
    | ExprForTable[left] "-"[op] ExprForTable[right]
                        {
	                        $result = new ExprForTableOp( (token_type) $op, $left, $right );
                        }
    | ExprForTable[left] "*"[op] ExprForTable[right]
                        {
	                        $result = new ExprForTableOp( (token_type) $op, $left, $right );
                        }
    | ExprForTable[left] "/"[op] ExprForTable[right]
                        {
	                        $result = new ExprForTableOp( (token_type) $op, $left, $right );
                        }
    | "(" ExprForTable[expr] ")"
                        {
	                        $result = $expr;
                        }
	;

ModgenCumulationOperator:
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

TableAccumulator:
      TK_sum
    | TK_min
    | TK_max
    ;

TableIncrement:
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

AgentVar:
		SYMBOL
		| DerivedAgentVar
	;

DerivedAgentVar:
    TK_duration "(" ")"
                        {
                            $DerivedAgentVar = DurationAgentVarSymbol::get_symbol( pc.get_agent_context() );
                        }
    | TK_duration "(" SYMBOL[agentvar] "," AnyLiteral[constant] ")"
                        {
                            $DerivedAgentVar = ConditionedDurationAgentVarSymbol::get_symbol( pc.get_agent_context(), $agentvar, $constant );
                        }
	;


%%

void
yy::parser::error (const yy::parser::location_type& l,
                          const std::string& m)
{
    drv.error (l, m);
}


