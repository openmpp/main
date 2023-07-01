/**
 * @file    ParseContext.cpp
 * Implements some functions in the parse context class.
 */
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "ParseContext.h"

string ParseContext::cxx_process_token(token_type tok, const string yytext, omc::location * loc)
{
    string tok_str = yytext; // create modifiable copy of yytext argument (the token as a string)

    //
    // Handle special global functions (PreSimulation, etc.) which need a suffix to disambiguate
    // Modify the token string tok_str to be the modified name with the suffix appended.
    // 
    if (tok == token::SYMBOL && brace_level == 0) {
        string word = tok_str;
        for (auto sg : { &Symbol::pre_simulation, &Symbol::post_simulation, &Symbol::derived_tables }) {
            if (sg->prefix == word.substr(0, sg->prefix.length())) {
                // Function name starts with one of the special prefixes
                if (word == sg->prefix) {
                    // No suffix, so substitute a disambiguated name based on order encountered in code.
                    // The disambiguated name will have a prefix om_ as well as a generated suffix.
                    word = sg->disambiguated_name(sg->ambiguous_count);
                    ++sg->ambiguous_count;
                }
                else {
                    // Has a suffix
                    string suffix = word.substr(sg->prefix.length());
                    sg->suffixes.push_back(suffix);
                }
            }
        }
        tok_str = word;
    }

    // Maintain the ordered list of all tokens in C++ code
    cxx_tokens.push_back(make_pair(tok, tok_str));

    // The return value (possibly modified in code below)
    string result = tok_str;

    //
    // Note all symbols used in C++ code.
    // 
    if (tok == token::SYMBOL) {
        Symbol::identifiers_in_model_source.insert(tok_str);
    }

    //
    // Identify and process the beginning of a function definition.
    // 
    if (tok == token::TK_LEFT_BRACE && brace_level == 1 && parenthesis_level == 0) {
        // { at level 0 is the possible start of the body of a function definition
        bool is_memfunc = false;    // is a member function like name1::name2 (mutually exclusive with is_globalfunc)
        bool is_globalfunc = false; // is a global function like name2 (mutually exclusive with is_memfunc)
        int pos = 0;
        int plev = 0;
        bool in_parmlist = false;
        vector<string> parmlist;
        bool name2_coming = false;
        string name2;
        bool scope_coming = false;
        bool name1_coming = false;
        string name1;
        // Iterate backwards through the token list,
        // looking for the pattern "name1::name2(parmlist) {" or, if not, "name2(parmlist) {"
        for (auto it = cxx_tokens.rbegin(); it != cxx_tokens.rend(); ++it, ++pos) {
            // token 0 is {
            if (pos == 0) continue;
            // if token -1 is not ), then not a function definition
            else if (pos == 1) {
                if (it->first != token::TK_RIGHT_PAREN) break;
                ++plev;
                in_parmlist = true;
                continue;
            }
            else if (in_parmlist) {
                // look for the matching (
                if (it->first == token::TK_RIGHT_PAREN) {
                    ++plev;
                }
                if (it->first == token::TK_LEFT_PAREN) {
                    --plev;
                    if (plev == 0) {
                        in_parmlist = false;
                        name2_coming = true;
                        continue;
                    }
                }
                // build the parameter list (reversed)
                parmlist.push_back(it->second);
                continue;
            }
            else if (name2_coming) {
                // snaffle name2
                if (it->first != token::SYMBOL) break;
                name2 = it->second;
                name2_coming = false;
                scope_coming = true;
                continue;
            }
            else if (scope_coming) {
                if (it->first != token::TK_SCOPE_RESOLUTION) {
                    is_globalfunc = true;
                    break;
                }
                scope_coming = false;
                name1_coming = true;
                continue;
            }
            else if (name1_coming) {
                // snaffle name1
                if (it->first != token::SYMBOL) break;
                // pattern matches
                name1 = it->second;
                is_memfunc = true;
                break;
            }
        }
        assert(!(is_memfunc && is_globalfunc)); // is_memfunc and is_globalfunc are mutually exclusive
        if (is_memfunc || is_globalfunc) {
            cxx_function_gather = true;
            cxx_function_name = name2;
            if (is_memfunc) {
                // if a member function, the name is the class-qualified name
                cxx_function_name = name1 + "::" + cxx_function_name;
            }
            reverse(parmlist.begin(), parmlist.end());
            Symbol::function_parmlist.emplace(cxx_function_name, parmlist);
            Symbol::function_defn_loc.emplace(cxx_function_name, *loc);
        }
    }

    //
    // Handle interior of functions.
    // 
    if (cxx_function_gather) {
        if (tok == token::TK_RIGHT_BRACE && brace_level == 0) {
            // end of function
            cxx_function_gather = false;
            cxx_function_name = "";
        }
        else if (tok == token::SYMBOL) {
            // Add identifier to map of all identifiers by function name
            assert(cxx_function_name != "");
            Symbol::function_body_identifiers.emplace(cxx_function_name, tok_str);
        }
        else if (tok == token::TK_RIGHT_PAREN && cxx_tokens.size() >= 4) {
            // Detect RNG function call with integer constant argument (or missing argument) in entity function body
            // retrieve previous 3 tokens
            auto it = cxx_tokens.cend();
            --it;
            auto tok_prev0 = *it; // current token (same as tok)
            --it; 
            auto tok_prev1 = *it;
            --it;
            auto tok_prev2 = *it;
            --it;
            auto tok_prev3 = *it;
            bool found_rng = false;
            int random_stream = Symbol::random_stream_error::eInvalidStreamArgument;
            string rng_func = "";
            if (
                   tok_prev1.first == token::INTEGER_LITERAL
                && tok_prev2.first == token::TK_LEFT_PAREN
                && tok_prev3.first == token::SYMBOL
                && Symbol::om_rng_functions.count(tok_prev3.second) > 0
                ) {
                // example: RandUniform(23)
                found_rng = true;
                random_stream = std::stoi(tok_prev1.second);
                if (!(random_stream > 0)) {
                    // must be positive integer
                    random_stream = Symbol::random_stream_error::eInvalidStreamArgument;
                }
                rng_func = tok_prev3.second;
            }
            else if (
                   tok_prev1.first == token::TK_LEFT_PAREN
                && tok_prev2.first == token::SYMBOL
                && Symbol::om_rng_functions.count(tok_prev2.second) > 0
                ) {
                // example: RandUniform()
                found_rng = true;
                random_stream = Symbol::random_stream_error::eMissingStreamArgument;
                rng_func = tok_prev2.second;
            }
            else if (
                   tok_prev2.first == token::TK_LEFT_PAREN
                && tok_prev3.first == token::SYMBOL
                && Symbol::om_rng_functions.count(tok_prev3.second) > 0
                ) {
                // example: RandUniform(x)
                found_rng = true;
                random_stream = Symbol::random_stream_error::eInvalidStreamArgument;
                rng_func = tok_prev3.second;
                // Note that this identifies a limited set of invalid (non-literal) arguments
                // but will identify a common error in model code - using a single variable as argument.
                // If a variable argument is needed and valid, as in random_lcg41.ompp,
                // unary plus can be used to evade detection, e.g.
                // RandUniform(+x).
            }
            if (found_rng) {
                // is an RNG function call like RandUniform(INTEGER_LITERAL)
                // Add stream # to map of all RNG streams in functions (including RandNormal)
                Symbol::function_rng_streams.emplace(cxx_function_name, random_stream);
                // Add stream # to multimap to identify and report clashes
                string loc_string = loc->begin.filename->c_str();
                loc_string += "(" + to_string(loc->begin.line) + ")";
                Symbol::rng_stream_calls.emplace(random_stream, loc_string);
                if (rng_func == "RandNormal") {
                    // is an RNG function call like RandNormal(INTEGER_LITERAL)
                    // Add stream # to map of all RNG Normal streams in functions
                    Symbol::function_rng_normal_streams.emplace(cxx_function_name, random_stream);
                }
            }
        }
    }

    //
    // Handle special functions like LT taking string literal as argument.
    // to identify strings for translation.
    // 
    if (tok == token::STRING_LITERAL && cxx_tokens.size() >= 3) {
        // retrieve previous 2 tokens
        auto it = cxx_tokens.cend();
        --it;
        --it;
        auto tok_prev1 = *it;
        --it;
        auto tok_prev2 = *it;
        if (tok_prev1.first == token::TK_LEFT_PAREN) {
            if (tok_prev2.first == token::SYMBOL) {
                if (Symbol::tran_funcs.count(tok_prev2.second) > 0) {
                    // Ignore occurrences in 'use' source files.
                    if (!Symbol::is_use_file(*loc)) {
                        // remove the enclosing double quotes from the string literal
                        assert(tok_str.length() >= 2); // scanner guarantee
                        auto str = tok_str.substr(1, tok_str.length() - 2);
                        // record the translatable string literal
                        Symbol::tran_strings.insert(str);
                    }
                }
            }
        }
    }


    // Return possibly modified version of the token.
    return result;
}

void ParseContext::process_cxx_comment(const string& cmt, const omc::location& loc)
{
    // Parse //LABEL comments
    if (cmt.length() >= 5 && cmt.substr(0, 5) == "LABEL") {
        //TODO use regex!
        std::string::size_type p = 5;
        std::string::size_type q = 5;

        // Extract symbol name
        p = cmt.find_first_not_of("( \t", p);
        if (p == std::string::npos) {
            warning(loc, LT("warning : problem (#1) with LABEL comment - not processed"));
            return;
        }
        q = cmt.find_first_of(", \t", p);
        if (q == std::string::npos) {
            warning(loc, LT("warning : problem (#2) with LABEL comment - not processed"));
            return;
        }
        string sym_name = cmt.substr(p, q - p);
        // Change . to :: to align with Symbol unique_name
        // e.g. Person.age to Person::age
        auto r = sym_name.find(".");
        if (r != string::npos) {
            sym_name.replace(r, 1, "::");
        }

        // Extract language code
        p = cmt.find_first_not_of(", \t", q);
        if (p == std::string::npos) {
            warning(loc, LT("warning : problem (#3) with LABEL comment - not processed"));
            return;
        }
        q = cmt.find_first_of(") \t", p);
        if (q == std::string::npos) {
            warning(loc, LT("warning : problem (#4) with LABEL comment - not processed"));
            return;
        }
        string lang_code = cmt.substr(p, q - p);
        string key = sym_name + "," + lang_code;

        // Extract label
        p = cmt.find_first_not_of(") \t", q);
        if (p == std::string::npos) {
            // ignore empty label
            return;
        }
        string lab = cmt.substr(p);
        
        // Insert label into map of all explicit //LABEL comments
        Symbol::explicit_labels.emplace(key, lab);

        // Don't place //LABEL comments into list of all cxx_comments,
        // which is used for //EN, etc.
        return;
    }

    // Parse //NAME comments
    if (cmt.length() >= 4 && cmt.substr(0, 4) == "NAME") {
        //TODO use regex!
        std::string::size_type p = 4;
        std::string::size_type q = 4;

        // Extract symbol name
        p = cmt.find_first_not_of("( \t", p);
        if (p == std::string::npos) {
            warning(loc, LT("warning : problem (#1) with NAME comment - not processed"));
            return;
        }
        q = cmt.find_first_of(", \t", p);
        if (q == std::string::npos) {
            warning(loc, LT("warning : problem (#2) with NAME comment - not processed"));
            return;
        }
        string sym_name = cmt.substr(p, q - p);

        // Skip white space between symbol name and export name
        p = cmt.find_first_not_of(" \t", q);
        if (p == std::string::npos) {
            // ignore empty name
            return;
        }
        string shrt_nam = cmt.substr(p);
//        string nam = cmt.substr(p);

        // Insert short name into map of all explicit //NAME comments
        Symbol::explicit_names.emplace(sym_name, shrt_nam);

        // Don't place //NAME comments into list of all cxx_comments,
        // which is used for //EN, etc.
        return;
    }

    // Construct key based on the beginning of the line containing the comment.
    omc::position pos(loc.begin.filename, loc.begin.line, 0);
    comment_map_value_type element(pos, cmt);
    Symbol::cxx_comments.insert(element);
}

void ParseContext::process_c_comment(const string& cmt, const omc::location& loc)
{
    // Parse NOTE comments.  Assume starts as /*NOTE or /* NOTE
    if ((cmt.length() > 4) && ((cmt.substr(0, 4) == "NOTE") || (cmt.substr(0, 5) == " NOTE"))) {
        //TODO use regex!
        std::string::size_type p = 4;
        std::string::size_type q = 4;
        if ((cmt.substr(0, 5) == " NOTE")) {
            p = 5;
            q = 5;
        }

        // Extract symbol name
        p = cmt.find_first_not_of("( \t", p);
        if (p == std::string::npos) {
            warning(loc, LT("warning : problem (#1) with NOTE comment - not processed"));
            return;
        }
        q = cmt.find_first_of(", \t", p);
        if (q == std::string::npos) {
            warning(loc, LT("warning : problem (#2) with NOTE comment - not processed"));
            return;
        }
        string sym_name = cmt.substr(p, q - p);
        // Change . to :: to align with Symbol unique_name
        // e.g. Person.age to Person::age
        auto r = sym_name.find(".");
        if (r != string::npos) {
            sym_name.replace(r, 1, "::");
        }

        // Extract language code
        p = cmt.find_first_not_of(", \t", q);
        if (p == std::string::npos) {
            warning(loc, LT("warning : problem (#3) with NOTE comment - not processed"));
            return;
        }
        q = cmt.find_first_of(") \t", p);
        if (q == std::string::npos) {
            warning(loc, LT("warning : problem (#4) with NOTE comment - not processed"));
            return;
        }
        string lang_code = cmt.substr(p, q - p);
        string key = sym_name + "," + lang_code;

        // Extract note
        p = cmt.find_first_not_of(") \t\n", q);
        if (p == std::string::npos) {
            // ignore empty label
            return;
        }
        string note = cmt.substr(p);

        // Insert note into appropriate map of NOTE comments
        if (is_scenario_parameter_value || is_fixed_parameter_value) {
            Symbol::notes_input.emplace(key, note);
        }
        else {
            Symbol::notes_source.emplace(key, note);
        }
    }

    // Construct key based on the start position of the comment.
    omc::position pos(loc.begin);
    comment_map_value_type element(pos, cmt);
    Symbol::c_comments.insert(element);
}


