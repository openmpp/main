%top{
/* Define C std integer type macros (e.g. INT8_MIN) before flex's fall-back versions */
#include <cstdint>
}

%{ /* -*- C++ -*- */
#include <iostream>
#include <fstream>
#include <typeinfo>
#include "libopenm/common/omFile.h"  /* for fileToUtf8 */
#include "Driver.h"
#include "ParseContext.h"
#include "ast.h"

/* Copyright (c) 2013-2016 OpenM++
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
%x om_code
%x cxx_code

%%

%{
    /* on each invocation to scanner, advance location to just after previously matched token */
    yylloc->step();
%}

			{
                /* This code block is executed on every call to scanner */
				if (yylloc->begin.line == 1 && yylloc->begin.column == 1 && yylloc->end.line == 1 && yylloc->end.column == 1 ) {
					/* initial state at beginning of file is C++ code */
					BEGIN(cxx_code);
				}
			}

<*>\r       {
                /* silently ignore ASCII return (strip from input) */
                ;
            }

<om_code,cxx_code>"/*" {
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

<om_code,cxx_code>\" {
                /* starting quote of string literal */
				yylloc->step();

                /* build literal specification (includes quote delimiters) */
                pc.literal_specification = yytext;
                pc.literal_length = 0;

                /* transition to string literal */
				yy_push_state(string_literal);
            }

<om_code,cxx_code>\' {
                /* starting quote of character literal */
				yylloc->step();

                /* build literal specification (includes quote delimiters) */
                pc.literal_specification = yytext;
                pc.literal_length = 0;

				/* transition to character literal */
				yy_push_state(character_literal);
            }

<string_literal>{ /* interior of string literal */

\"          {
                /* ending quote of string literal */
				yylloc->step();

                /* build literal specification (includes quote delimiters) */
                pc.literal_specification += yytext;

                /* transition out of string literal to previous state */
				yy_pop_state();

				token_type tok = token::STRING_LITERAL;
				if (YY_START == om_code) {
                    yylval->pval_StringLiteral = new StringLiteral ( pc.literal_specification.c_str() );
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code); /* logic guarantee */
                    *drv.outside << pc.cxx_process_token(tok, pc.literal_specification, yylloc);
                }
            }

[^\\\n\"]   {
                /* unescaped regular character inside string literal */
				yylloc->step();

                /* build literal specification */
                pc.literal_specification += yytext;
                pc.literal_length++;
            }
} /* <string_literal> interior of string literal */

<character_literal>{ /* interior of character literal */

\'          {
                /* ending quote of character literal */
				yylloc->step();

                /* build literal specification (includes quote delimiters) */
                pc.literal_specification += yytext;
				/* TODO check that true length is 1, error if not */

                /* transition out of character literal to previous state */
				yy_pop_state();

				token_type tok = token::CHARACTER_LITERAL;
				if ( YY_START == om_code ) {
                    yylval->pval_CharacterLiteral = new CharacterLiteral ( pc.literal_specification.c_str() );
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code); /* logic guarantee */
                    *drv.outside << pc.cxx_process_token(tok, pc.literal_specification, yylloc);
                }
            }

[^\\\n\']   {
                /* unescaped regular character inside character literal */
				yylloc->step();

                /* build literal specification */
                pc.literal_specification += yytext;
                pc.literal_length++;
            }
} /* <character_literal> interior of character literal */

<string_literal,character_literal>{ /* escape sequences and common code for string and character literals */

\n          {
                /* string or character literal interior - error - unterminated string or character literal */
				/* TODO - generate error message */
				yylloc->lines(1);
				yylloc->step();

                /* transition out of string or character literal to previous state */
				yy_pop_state();
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
} /* <string_literal,character_literal> escape sequences and common code for string and character literals */

<comment>{ /* interior of C-style comment */

[^*\r\n]*     {
				/* comment interior - not a * or newline */
				yylloc->step();

                /* build comment body */
                pc.comment_body += yytext;
			}

"*"+[^*/\r\n]* {
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

                pc.process_c_comment(pc.comment_body, pc.comment_location);

                /* position maintenance */
				yylloc->step();

                /* transition out of comment to previous state */
				yy_pop_state();
			}
} /* <comment> interior of C-style comment */

<om_code,cxx_code>{ /* whitespace and single-line comments */

{blank}+	{
				/* step over blanks in location tracking */
				yylloc->step();

				/* mark-up file - output whitespace in C++ for syntactic significance */
				if (YY_START == cxx_code) *drv.outside << yytext;
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
                /* so matches a single-line C++ comment at eof even if no trailing newline */

                /* note start location of comment body */
                pc.comment_location = *yylloc;

                /* location maintenance */
                yylloc->step();

                /* build comment body, does not include comment introducer or trailing newline */
                pc.comment_body = yytext + 2;

                pc.process_cxx_comment(pc.comment_body, pc.comment_location);
			}
} /* <om_code,cxx_code> whitespace and single-line comments */

<om_code>{ /* identifiers and keywords in ompp declarative islands */
	
{word}/{blank}*=> {
                /* name followed by naming operator => is returned as string */
                yylloc->step();
                string word = yytext;
                yylval->pval_string = new string(word);
                pc.next_word_is_string = false;
                return token::STRING;
            }

"=>"        {
				/* double arrow naming operator */
                yylloc->step();
				token_type tok = Symbol::string_to_token(yytext);
                yylval->val_token = tok;
                return tok;
            }

{word}      {
                yylloc->step();
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
                        /* This case is handled by fix-up in first pass of Symbol::post_parse() */
                    }
                }
				yylval->pval_Symbol = sym;
				return token::SYMBOL;
			}

} /* <om_code> identifiers and keywords in ompp declarative islands */

<cxx_code>{ /* identifiers and keywords in C++ code */

{word}      {
                yylloc->step();
			    token_type tok = Symbol::string_to_token( yytext );
                /* tok has value TK_error if not a keyword */
                if (Symbol::is_om_outer_keyword(tok)) {
                    if (pc.brace_level == 0) {
                        /* transition to openM++ declarative island */
    				    BEGIN(om_code);
                        yylval->val_token = tok;
                        /* special case to force a string look-ahead token following 'options' to be STRING */
                        if (tok == token::TK_options) {
                            pc.next_word_is_string = true;
                        }
                        /* feed token to parser */
                        return tok;
                    }
                    else {
                        /* treat ompp keywords inside braces as identifiers */
                        tok = token::SYMBOL;
                    }
    			}

                if (tok == token::TK_error) {
                    /* not a keyword, therefore a symbol */
                    tok = token::SYMBOL;
                }
                *drv.outside << pc.cxx_process_token(tok, yytext, yylloc);
			}

} /* <cxx_code> identifiers and keywords in C++ code */

<om_code,cxx_code>{ /* elements of C++ syntax common to ompp and C++ */

"#"[^\r\n]* { /* pre-processor directive */
				token_type tok = token::TK_error;
                if (YY_START == om_code) {
    				/* Let parser raise syntax error. */
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code);
                    /* Let C++ compiler handle it */
                    *drv.outside << pc.cxx_process_token(tok, yytext, yylloc);
                }
            }

"{"|"}"|"("|")"|"["|"]" { /* brace, parenthesis, bracket level maintenance */
                yylloc->step();
                switch ( *yytext ) {
                    case '{': pc.brace_level++; break;
                    case '}': pc.brace_level--; break;
                    case '(': pc.parenthesis_level++; break;
                    case ')': pc.parenthesis_level--; break;
                    case '[': pc.bracket_level++; break;
                    case ']': pc.bracket_level--; break;
                    default: assert(false);
                }
				token_type tok = Symbol::string_to_token(yytext);
                if (YY_START == om_code) {
                    yylval->val_token = tok;
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code);
                    *drv.outside << pc.cxx_process_token(tok, yytext, yylloc);
                }
			}

"->*"|"<<="|">>="|"..." {
				/* three character C++ operators */
                yylloc->step();
				token_type tok = Symbol::string_to_token(yytext);
                if (YY_START == om_code) {
                    yylval->val_token = tok;
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code);
                    *drv.outside << pc.cxx_process_token(tok, yytext, yylloc);
                }
			}

"::"|"++"|"--"|"->"|".*"|"<<"|">>"|"<="|">="|"=="|"!="|"&&"|"||"|"+="|"-="|"*="|"/="|"%="|"&="|"^="|"|=" {
				/* two character C++ operator */
                yylloc->step();
				token_type tok = Symbol::string_to_token(yytext);
                if (YY_START == om_code) {
                    yylval->val_token = tok;
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code);
                    *drv.outside << pc.cxx_process_token(tok, yytext, yylloc);
                }
			}

";"|"."|"+"|"-"|"!"|"~"|"*"|"&"|"/"|"%"|"<"|">"|"^"|"|"|"?"|":"|"="|","|"("|")"|"["|"]" {
				/* one character C++ operator */
                yylloc->step();
				token_type tok = Symbol::string_to_token(yytext);
                if (YY_START == om_code) {
				    if ((tok == token::TK_SEMICOLON) && (pc.brace_level == 0)) {
					    /* transition to C++ code */
					    BEGIN(cxx_code);
				    }
                    yylval->val_token = tok;
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code);
                    *drv.outside << pc.cxx_process_token(tok, yytext, yylloc);
                }
			}

{floating}  {
                yylloc->step();
				token_type tok = token::FLOATING_POINT_LITERAL;
                if (YY_START == om_code) {
                    yylval->pval_FloatingPointLiteral = new FloatingPointLiteral(yytext);
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code);
                    *drv.outside << pc.cxx_process_token(tok, yytext, yylloc);
                }
			}

{integer}   {
                yylloc->step();
				token_type tok = token::INTEGER_LITERAL;
                if (YY_START == om_code) {
                    yylval->pval_IntegerLiteral = new IntegerLiteral(yytext);
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code);
                    *drv.outside << pc.cxx_process_token(tok, yytext, yylloc);
                }
			}

.           {
				/* unrecognized token */
				token_type tok = token::TK_error;
                if (YY_START == om_code) {
    				/* Let parser raise syntax error. */
                    return tok;
                }
                else {
                    assert(YY_START == cxx_code);
                    /* Let C++ compiler handle it, as appropriate */
                    *drv.outside << pc.cxx_process_token(tok, yytext, yylloc);
                }
			}

} /* <om_code,cxx_code> elements of C++ syntax common to ompp and C++ */

.           {
                assert(false);  /* not reached */
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

    openm::the_string.clear();
}

