%{ /* -*- C++ -*- */
#include <iostream>
#include <fstream>
#include <typeinfo>
#include "Driver.h"
#include "ParseContext.h"
#include "ast.h"

/* Copyright (c) 2013 OpenM++
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
blank       [ \t\r]

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
					/* this does not work with windows back-slashed pathes */
					/* *drv.outside << "#line 1 \"" << *yylloc->end.filename << "\"\n"; */
				}
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

<*>{ /* whitespace and single-line comments */

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

"//".*\n    {
				/* single line comment */

                /* note location of comment body */
                pc.comment_location = *yylloc;

                /* location maintenance */
                yylloc->lines(1);
                yylloc->step();

                /* build comment body, does not include comment introducer or trailing newline */
                pc.comment_body = yytext + 2;
                pc.comment_body.erase( pc.comment_body.length() - 1 );

                Symbol::process_cxx_comment(pc.comment_body, pc.comment_location);

				/* mark-up file - output newline to maintain validity of #line directives (remove comment) */
				*drv.outside << "\n";
			}
}

<om_decl>{ /* interior of openM++ declarative islands */
	
{word}      {
			    token_type tok = Symbol::string_to_token( yytext );
                if ( tok != token::TK_error ) {
                    /* valid keyword */
                    yylval->val_token = tok;
                    return tok;
                }
                string word = yytext;
                /* special hack to silently interpret 'actor_id' as 'entity_id' */
                if (word == "actor_id") word = "entity_id";
                /* suppress name resolution if requested by parser */
                if (pc.next_word_is_string) {
                    yylval->pval_string = new string(word);
                    pc.next_word_is_string = false;
                    return token::STRING;
                }

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

{floating}   {
                yylval->pval_FloatingPointLiteral = new FloatingPointLiteral ( yytext );
                return token::FLOATING_POINT_LITERAL;
			}

{integer}   {
                yylval->pval_IntegerLiteral = new IntegerLiteral ( yytext );
                return token::INTEGER_LITERAL;
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

"->*"|"<<="|">>=" {
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

.           {
				/* error */
				return (token_type) *yytext;
			}

}

<cxx_outside,cxx_inside>{ /* C++ code */

"{"         {
				pc.brace_level++;
				if ( pc.brace_level == 1 ) {
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
                    }
				}
			}

}

<cxx_outside>{ /* outer level of C++ code, i.e. outside of all braces */

{word}::{word} { /*  possible function member definition */
                /* prepare to gather identifiers from function body */
                pc.cxx_memfunc_gather = true;
                pc.cxx_memfunc_name = yytext;

                /* special markup of Start and Finish agent member functions */
                string word1 = yytext;
                word1.resize(word1.find("::"));
                string word2 = yytext;
                word2.erase(0, 2 + word2.find("::"));
                if ( word2 == "Start" || word2 == "Finish" ) {
				    /* modify agent function name to distinguish function supplied by model developer from openM++ function */
				    *drv.outside << word1 << "::om_" << word2 << "_custom" ;
                }
				else if ( ! pc.cxx_suppress ) *drv.outside << yytext;
            }

{word}      {
			    token_type tok = Symbol::string_to_token( yytext );
                /* tok has value TK_error if not a keyword */
                if ( Symbol::is_om_outer_keyword( tok ) ) {
                    /* transition to openM++ declarative island */
    				BEGIN(om_decl);
                    yylval->val_token = tok;
                    /* feed token to parser */
                    return tok;
    			}

                /* create working string version of the word */
                string word ( yytext );
                if ( word == "Simulation" ) {
				    /* default model name is stem of module defining Simulation function */
                    /* supply if not already specified by model code */
                    if ( ! Symbol::find_a_symbol( typeid(ModelSymbol) ) ) {
                        Symbol *sym = new ModelSymbol( drv.stem );
                    }

				    /* suppress Modgen boilerplate of function body */
				    pc.cxx_suppress = true;
				    *drv.outside << yytext << "() {};" ;
                }
                else if ( word == "CaseSimulation" ) {
				    /* suppress Modgen boilerplate of function body */
				    pc.cxx_suppress = true;
				    *drv.outside << yytext << "() {};" ;
                }
                else {
    				if ( ! pc.cxx_suppress ) *drv.outside << yytext;
                }
			}

.           {
				/* all other C++ code */
				if ( ! pc.cxx_suppress ) *drv.outside << yytext;
			}

}

<cxx_inside>{ /* an inner level of C++ code, i.e. inside {} */

{word}      {
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
				/* all other C++ code */
				if ( ! pc.cxx_suppress ) *drv.outside << yytext;
			}

}


.           { /* not reached, hopefully */
			}

%%

void Driver::scan_begin ()
{
    yy_flex_debug = trace_scanning;
    if (file.empty () || file == "-")
        yyin = stdin;
    else if (!(yyin = fopen (file.c_str (), "r"))) {
        error ("cannot open " + file + ": " + strerror(errno));
        exit (EXIT_FAILURE);
    }

    // detect and discard UTF-8 BOM if present
    const char *bom = "\xEF\xBB\xBF";
    char first_three[4];
    first_three[3] = '\0';
    fread(first_three, 1, 3, yyin);
    if ( 0 != strcmp(first_three, bom) ) {
        // bom not present, rewind
        fseek(yyin, 0L, SEEK_SET);
    }
}

void Driver::scan_end ()
{
  fclose (yyin);
}

