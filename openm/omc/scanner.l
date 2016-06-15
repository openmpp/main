%top{
/* Define C std integer type macros (e.g. INT8_MIN) before flex's fall-back versions */
#include <cstdint>
}

%{ /* -*- C++ -*- */
#include <iostream>
#include <fstream>
#include <typeinfo>
#include "libopenm/common/omFile.h"  // for fileToUtf8
#include "Driver.h"
#include "ParseContext.h"
#include "ast.h"

/* Copyright (c) 2013-2015 OpenM++
* This code is licensed under MIT license (see LICENSE.txt for details)
*/

/* Work around an incompatibility in flex (at least versions
   2.5.31 through 2.5.33): it generates code that does
   not conform to C89.  See Debian bug 333231
   <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.  */
#undef yywrap
#define yywrap() 1

/* By default yylex returns int, we use token_type.
   Unfortunately yyterminate by default returns 0, which is
   not of token_type. */
#define yyterminate() return token::END
%}

%option noyywrap nounput batch debug stack

%{
    /* advance location end position by width of matched pattern */
#define YY_USER_ACTION  yylloc->columns (yyleng);
%}
    /* regular expression for C floating point (decimal) number is from http://rosettacode.org/wiki/Category:C */
    /* see also http://en.cppreference.com/w/cpp/language/floating_literal */

floating    (([0-9]*"."[0-9]+|[0-9]+".")([eE][+-]?[0-9]+)?[flFL]?)|([0-9]+[eE][+-]?[0-9]+[flFL]?)
integer     [0-9]+
word        [a-zA-Z\x80-\xff][_a-zA-Z0-9\x80-\xff]*
blank       [ \t]

%x comment
%x string_literal
%x character_literal
%x om_decl
%x cxx_outside
%x cxx_inside

%%

%{
    /* on each invocation to scanner, advance location to just after previously matched token */
    yylloc->step();
%}

			{
                /* This code block is executed on every call to scanner */
				if (yylloc->begin.line == 1 && yylloc->begin.column == 1 && yylloc->end.line == 1 && yylloc->end.column == 1 ) {
					/* transition to C++ exterior code (beginning of file) */
					BEGIN(cxx_outside);

					/* mark-up file - maintain correspondence of line numbers with source file */
					/* this does not work with windows back-slashed paths */
					/* *drv.outside << "#line 1 \"" << *yylloc->end.filename << "\"\n"; */
				}
			}

<*>\r {
                /* silently ignore ASCII return (strip from input) */
                ;
            }

<om_decl,cxx_inside,cxx_outside>"/*" {
                /* start of C-style comment */

                /* note location of comment body */
                pc.comment_location = *yylloc;

                /* location maintenance */
   				yylloc->step();

                /* build comment body */
                /* comment body does not include comment introducer */
                pc.comment_body = "";

                /* transition to comment */
				yy_push_state(comment);
			}

<om_decl,cxx_inside,cxx_outside>\" {
                /* starting quote of string literal */
				yylloc->step();

                /* build literal specification (includes quote delimiters) */
                pc.literal_specification = yytext;
                pc.literal_length = 0;

                /* transition to string literal */
				yy_push_state(string_literal);
            }

<om_decl,cxx_inside,cxx_outside>\' {
                /* starting quote of character literal */
				yylloc->step();

                /* build literal specification (includes quote delimiters) */
                pc.literal_specification = yytext;
                pc.literal_length = 0;

				/* transition to character literal */
				yy_push_state(character_literal);
            }

<string_literal>{

\"          {
                /* ending quote of string literal */
				yylloc->step();

                /* build literal specification (includes quote delimiters) */
                pc.literal_specification += yytext;

                /* transition out of string literal to previous state */
				yy_pop_state();

                /* mark-up file */
                if ( ( YY_START == cxx_outside || YY_START == cxx_inside ) && ! pc.cxx_suppress ) *drv.outside << pc.literal_specification;

                /* om declarative island */
				if ( YY_START == om_decl ) {
                    yylval->pval_StringLiteral = new StringLiteral ( pc.literal_specification.c_str() );
                    return token::STRING_LITERAL;
                }
            }

[^\\\n\"]   {
                /* unescaped regular character inside string literal */
				yylloc->step();

                /* build literal specification */
                pc.literal_specification += yytext;
                pc.literal_length++;
            }
}

<character_literal>{

\'          {
                /* ending quote of character literal */
				yylloc->step();

                /* build literal specification (includes quote delimiters) */
                pc.literal_specification += yytext;
				/* TODO check that true length is 1, error if not */

                /* transition out of character literal to previous state */
				yy_pop_state();

                /* mark-up file */
				if ( ( YY_START == cxx_outside || YY_START == cxx_inside ) && ! pc.cxx_suppress ) *drv.outside << pc.literal_specification;

                /* om declarative island */
				if ( YY_START == om_decl ) {
                    yylval->pval_CharacterLiteral = new CharacterLiteral ( pc.literal_specification.c_str() );
                    return token::CHARACTER_LITERAL;
                }
            }

[^\\\n\']   {
                /* unescaped regular character inside character literal */
				yylloc->step();

                /* build literal specification */
                pc.literal_specification += yytext;
                pc.literal_length++;
            }
}

<string_literal,character_literal>{ /* escape sequences and common code for string and character literals */

\n          {
                /* string or character literal interior - error - unterminated string or character literal */
				/* TODO - generate error message */
				yylloc->lines(1);
				yylloc->step();

                /* transition out of string or character literal to previous state */
				yy_pop_state();

                /* mark-up file */
				if ( ( YY_START == cxx_outside || YY_START == cxx_inside ) && ! pc.cxx_suppress ) *drv.outside << pc.literal_specification;
            }

\\[0-7]{1,3} {
				/* string or character literal interior - octal escape sequence in string or character literal*/
				yylloc->step();

                /* build literal specification */
                pc.literal_specification += yytext;
                pc.literal_length++;
				/* TODO additional validity checks on value, error if bad */
			}

\\[0-9]+	{
				/* string or character literal interior - bad escape sequence in string or character literal */
				yylloc->step();

                /* build literal specification */
                pc.literal_specification += yytext;
                pc.literal_length++;
				/* TODO - generate error message */
			}

\\[ntrbf]	{
				/* string or character literal interior - \n \t, etc. */
				yylloc->step();

                /* build literal specification */
                pc.literal_specification += yytext;
                pc.literal_length++;
			}

\\\n        {
                /* string or character literal interior - escaped newline, ignore as per C++ specification  */
				yylloc->step();
				yylloc->lines(1);
            }

\\.         {
                /* string or character literal interior - escaped character */
				yylloc->step();

                /* build literal specification */
                pc.literal_specification += yytext;
                pc.literal_length++;
            }
}

<comment>{ /* C-style comment */

[^*\n]*     {
				/* comment interior - not a * or newline */
				yylloc->step();

                /* build comment body */
                pc.comment_body += yytext;
			}

"*"+[^*/\n]* {
				/* comment interior - a * followed by 0 or more ordinary characters */
				yylloc->step();

                /* build comment body */
                pc.comment_body += yytext;
			}

[\n]+       {
				/* comment interior - a newline */
				yylloc->step();
				yylloc->lines(yyleng);

                /* build comment body */
                pc.comment_body += yytext;

				/* markup file - output newlines to maintain validity of #line directives */
				*drv.outside << yytext;
			}

"*"+"/"     {
                /* end of comment */

                /* note end position of comment body */
                pc.comment_location.end = yylloc->end;

                Symbol::process_c_comment(pc.comment_body, pc.comment_location);

                /* position maintenance */
				yylloc->step();

                /* transition out of comment to previous state */
				yy_pop_state();
			}
}

<om_decl,cxx_inside,cxx_outside>{ /* whitespace and single-line comments */

{blank}+	{
				/* step over blanks in location tracking */
				yylloc->step();

				/* mark-up file - output whitespace in C++ for syntactic significance */
				if ( YY_START == cxx_outside || YY_START == cxx_inside ) *drv.outside << yytext;
			}

[\n]+       {
				/* one or more newlines */
				yylloc->lines(yyleng);
				yylloc->step();

                /* mark-up file - output newlines to maintain validity of #line directives */
				*drv.outside << yytext;
			}

"//"[^\r\n]*    {
				/* single line comment */
                /* pattern does not include trailing newline */
                /* so matches a // comment at eof even if no trailing newline */

                /* note location of comment body */
                pc.comment_location = *yylloc;

                /* location maintenance */
                yylloc->step();

                /* build comment body, does not include comment introducer or trailing newline */
                pc.comment_body = yytext + 2;

                Symbol::process_cxx_comment(pc.comment_body, pc.comment_location);
			}
}

<om_decl>{ /* interior of openM++ declarative islands */
	
{word}      {
                string word = yytext;

                /* special hack to silently interpret 'actor_id' as 'entity_id' */
                if (word == "actor_id") word = "entity_id";

                /* suppress name resolution if requested by parser */
                if (pc.next_word_is_string) {
                    yylval->pval_string = new string(word);
                    pc.next_word_is_string = false;
                    return token::STRING;
                }

                /* check if word is a recognized C++ or ompp keyword */
			    token_type tok = Symbol::string_to_token( yytext );
                if ( tok != token::TK_error ) {
                    /* valid keyword */
                    yylval->val_token = tok;
                    return tok;
                }

                /* Add the identifier to the set of all identifiers in the model source code. */
                Symbol::identifiers_in_model_source.insert(word);

				/* find existing symbol or create a new one */
                Symbol *sym = nullptr;
                Symbol *agent = pc.get_agent_context();
                if (agent == nullptr) {
                    /* Currently in global context */
                    sym = Symbol::get_symbol(word);
                    if (!sym) sym = new Symbol(word, *yylloc);
                }
                else {
                    /* Currently in agent context */
                    /* Attempt first to resolve the name in global context (global names have precedence) */
                    sym = Symbol::get_symbol(word);
                    if (!sym) {
                        /* Resolve name in agent context */
                        sym = Symbol::get_symbol(word, agent);
                        if (!sym) sym = new Symbol(word, agent, *yylloc);
                        /* Possibly this was a global name not yet encountered in the source code. */
                        /* This case is handled by fix-up in Symbol::post_parse(eCreateMissingSymbols) */
                    }
                }
				yylval->pval_Symbol = sym;
				return token::SYMBOL;
			}

"{"|"}"|"("|")"|"["|"]" { /* brace, parenthesis, bracket level maintenance */
                switch ( *yytext ) {
                    case '{': pc.brace_level++; break;
                    case '}': pc.brace_level--; break;
                    case '(': pc.parenthesis_level++; break;
                    case ')': pc.parenthesis_level--; break;
                    case '[': pc.bracket_level++; break;
                    case ']': pc.bracket_level--; break;
                }
				
				token_type tok = Symbol::string_to_token(yytext);
                yylval->val_token = tok;
                return tok;
			}

";"         {
				if ( pc.brace_level == 0 ) {
					/* transition to C++ outermost brace level */
					BEGIN(cxx_outside);
					/* mark-up file - maintain correspondence of line numbers to source file */
					/* this does not work with windows back-slashed pathes */
					/* *drv.outside << std::endl << "#line " << yylloc->end.line << " \"" << *yylloc->end.filename << "\"" << std::endl; */
				}
				token_type tok = Symbol::string_to_token(yytext);
                yylval->val_token = tok;
                return tok;
			}

"->*"|"<<="|">>="|"..." {
				/* three character C++ operators */
				token_type tok = Symbol::string_to_token(yytext);
                yylval->val_token = tok;
                return tok;
			}

"::"|"++"|"--"|"->"|".*"|"<<"|">>"|"<="|">="|"=="|"!="|"&&"|"||"|"+="|"-="|"*="|"/="|"%="|"&="|"^="|"|=" {
				/* two character C++ operator */
				token_type tok = Symbol::string_to_token(yytext);
                yylval->val_token = tok;
                return tok;
			}

"."|"+"|"-"|"!"|"~"|"*"|"&"|"/"|"%"|"<"|">"|"^"|"|"|"?"|":"|"="|","|"("|")"|"["|"]" {
				/* one character C++ operator */
				token_type tok = Symbol::string_to_token(yytext);
                yylval->val_token = tok;
                return tok;
			}

{floating}  {
                yylval->pval_FloatingPointLiteral = new FloatingPointLiteral ( yytext );
                return token::FLOATING_POINT_LITERAL;
			}

{integer}   {
                yylval->pval_IntegerLiteral = new IntegerLiteral ( yytext );
                return token::INTEGER_LITERAL;
			}

.           {
				/* error */
				return (token_type) *yytext;
			}

}

<cxx_outside,cxx_inside>{ /* C++ code */

"{"         {
				pc.brace_level++;
				if ( pc.brace_level == 1 ) {
                    if (pc.cxx_memfunc_gather) {
                        /* record the now-complete function declaration parameter list */
                        Symbol::memfunc_parmlist.emplace(pc.cxx_memfunc_name, pc.cxx_memfunc_parmlist);
                        pc.cxx_memfunc_parmlist.clear();
                    }

				    /* transition to C++ code, inside braces */
				    BEGIN(cxx_inside);
				}
				if ( ! pc.cxx_suppress ) *drv.outside << yytext;
			}

"}"         {
				pc.brace_level--;
				if ( ! pc.cxx_suppress ) *drv.outside << yytext;
				if ( pc.brace_level == 0 ) {
					/* transition to C++ code, outside braces */
					BEGIN(cxx_outside);
                    /* no longer in function body, stop any code suppression */
					if ( pc.cxx_suppress ) pc.cxx_suppress = false;
                    if ( pc.cxx_memfunc_gather ) {
                        /* no longer in function body, stop gathering identifiers */
                        pc.cxx_memfunc_gather = false;
                        pc.cxx_memfunc_name = "";
                        pc.cxx_memfunc_parmlist.clear();
                    }
				}
			}

}

<cxx_outside>{ /* outer level of C++ code, i.e. outside of all braces */

{word}::{word} { /*  possible function member definition */
                /* prepare to gather argument parmlist and identifiers in function body */
                pc.cxx_memfunc_gather = true;
                pc.cxx_memfunc_name = yytext;
                pc.cxx_memfunc_parmlist.clear();

				if ( ! pc.cxx_suppress ) *drv.outside << yytext;
            }

{word}      {
			    token_type tok = Symbol::string_to_token( yytext );
                /* tok has value TK_error if not a keyword */
                if ( Symbol::is_om_outer_keyword( tok ) ) {
                    /* transition to openM++ declarative island */
    				BEGIN(om_decl);
                    yylval->val_token = tok;
                    /* special case to force a 'word' look-ahead token following 'options' to be STRING */
                    if (tok == token::TK_options) {
                        pc.next_word_is_string = true;
                    }
                    /* feed token to parser */
                    return tok;
    			}

                if (pc.cxx_memfunc_gather) {
                    /* build the member function parameter list declaration */
                    pc.cxx_memfunc_parmlist.push_back(yytext);
                }

                /* Working string copy of the word which may be modified by mark-up code below */
                string word = yytext;

                for (auto sg : {&Symbol::pre_simulation, &Symbol::post_simulation, &Symbol::derived_tables}) {
                    /* Detect and handle special global function definitions */
                    //auto sg = &Symbol::pre_simulation;
                    if (sg->prefix == word.substr(0, sg->prefix.length())) {
                        /* starts with prefix */
                        if (word == sg->prefix) {
                            /* No suffix, so substitute a disambiguated name based on order encountered in code */
                            word = sg->disambiguated_name(sg->ambiguous_count);
                            ++sg->ambiguous_count;
                        }
                        else {
                            /* Has a suffix */
                            string suffix = word.substr(sg->prefix.length());
                            sg->suffixes.push_back(suffix);
                        }
                    }
                }

    			if ( ! pc.cxx_suppress ) *drv.outside << word;
			}

.           {
				/* all other C++ code outside braces */
                if (pc.cxx_memfunc_gather) {
                    /* build the member function parameter list declaration */
                    pc.cxx_memfunc_parmlist.push_back(yytext);
                }

				if ( ! pc.cxx_suppress ) *drv.outside << yytext;
			}

}

<cxx_inside>{ /* an inner level of C++ code, i.e. inside {} */

{word}      {
                string word = yytext;
                /* Add the identifier to the set of all identifiers in the model source code. */
                Symbol::identifiers_in_model_source.insert(word);

                if ( pc.cxx_memfunc_gather ) {
			        token_type tok = Symbol::string_to_token( yytext );
                    if ( tok == token::TK_error ) {
                        /* word is not a token, so it must be an identifier */
                        Symbol::memfunc_bodyids.insert( pair<string, string>(pc.cxx_memfunc_name, yytext) );
                    }
                }
   				if ( ! pc.cxx_suppress ) *drv.outside << yytext;
            }

.           {
				/* all other C++ code inside braces */
				if ( ! pc.cxx_suppress ) *drv.outside << yytext;
			}

}


.           { /* not reached, hopefully */
			}

%%

// Use globals to manage string and flex buffer lifecycle/persistence
namespace openm
{
    string the_string;
    yy_buffer_state * the_buffer;
};

void Driver::scan_begin()
{
    yy_flex_debug = trace_scanning;

    openm::the_string = fileToUtf8(file.c_str(), Symbol::code_page.c_str());

    // Tell flex to use a const char * string for input instead of a file.
    // It returns the buffer so we can clean up after all's done.
    openm::the_buffer = yy_scan_string(openm::the_string.c_str());
}

void Driver::scan_end()
{
    // Tell flex to clean up its buffer
    yy_delete_buffer(openm::the_buffer);
}

