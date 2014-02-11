/**
* @file    LinkSymbol.cpp
* Definitions for the LinkSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "LinkSymbol.h"

using namespace std;

LinkSymbol *LinkSymbol::get(const Symbol *agent)
{
    LinkSymbol *ls = nullptr;
    string type_name = "link< " + agent->name + ">";
    Symbol *sym = get_symbol(type_name);
    if (sym) ls = dynamic_cast<LinkSymbol *>(sym);
    else ls = new LinkSymbol(agent);
    assert(ls);
    return ls;
}
