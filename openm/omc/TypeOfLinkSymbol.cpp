/**
* @file    TypeOfLinkSymbol.cpp
* Definitions for the TypeOfLinkSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TypeOfLinkSymbol.h"

using namespace std;

TypeOfLinkSymbol *TypeOfLinkSymbol::get_single(const Symbol *agent)
{
    TypeOfLinkSymbol *ls = nullptr;
    string type_name = "link<" + agent->name + ">";
    Symbol *sym = get_symbol(type_name);
    if (sym) ls = dynamic_cast<TypeOfLinkSymbol *>(sym);
    else ls = new TypeOfLinkSymbol(agent, true);
    assert(ls); // logic guarantee
    return ls;
}

TypeOfLinkSymbol *TypeOfLinkSymbol::get_multi(const Symbol *agent)
{
    TypeOfLinkSymbol *ls = nullptr;
    string type_name = "multi_link<" + agent->name + ">";
    Symbol *sym = get_symbol(type_name);
    if (sym) ls = dynamic_cast<TypeOfLinkSymbol *>(sym);
    else ls = new TypeOfLinkSymbol(agent, false);
    assert(ls); // logic guarantee
    return ls;
}
