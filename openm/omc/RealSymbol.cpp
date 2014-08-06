/**
* @file    RealSymbol.cpp
* Definitions for the RealSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "RealSymbol.h"
#include "CodeBlock.h"

using namespace std;

CodeBlock RealSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    // only one keyword for real (either TK_double or TK_float)
    token_type tok = keywords.front();

    // Define macro constant for maximum represenable value
    if (tok == token::TK_double) {
        h += "#define REAL_MAX DBL_MAX" ;
    }
    else {
        assert(tok == token::TK_float); // grammar guarantee
        h += "#define REAL_MAX FLT_MAX";
    }

    return h;
}

bool RealSymbol::is_float()
{
    return keywords.front() == token::TK_float;
}


// static
RealSymbol * RealSymbol::find()
{
    auto rs = dynamic_cast<RealSymbol *>(get_symbol(token_to_string(token::TK_real)));
    assert(rs); // only called when valid
    return rs;
}
