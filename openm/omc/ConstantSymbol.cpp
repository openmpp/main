/**
* @file    ConstantSymbol.cpp
* Definitions for the ConstantSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ConstantSymbol.h"
#include "EnumeratorSymbol.h"
#include "Literal.h"

using namespace std;

const string ConstantSymbol::value() const
{
    if (is_literal) {
        return literal->value();
    }
    else {
        return (*enumerator)->name;
    }
}

const string ConstantSymbol::value_as_name() const
{
    if (is_literal) {
        string work = literal->value();
        for (auto &ch : work) {
            if (ch == '.') ch = 'o';
            if (ch == '-') ch = 'm';
        }
        return work;
    }
    else {
        return (*enumerator)->name;
    }
}


// static
string ConstantSymbol::next_symbol_name()
{
    ++instance_counter;
    return "om_constant_" + to_string(instance_counter);
}

// static
int ConstantSymbol::instance_counter = 0;

void ConstantSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        if (is_enumerator) {
            // assign direct pointer to enumerator for use post-parse
            pp_enumerator = dynamic_cast<EnumeratorSymbol *> (pp_symbol(enumerator));
            if (!pp_enumerator) {
                assert(*enumerator); // parser guarantee
                pp_error("Error - '" + (*enumerator)->name + "' is not an enumerator");
                // OK to continue
            }
        }
        break;
    }
    default:
        break;
    }
}
