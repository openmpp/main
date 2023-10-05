/**
* @file    TypeOfLinkSymbol.cpp
* Definitions for the TypeOfLinkSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "TypeOfLinkSymbol.h"

using namespace std;

TypeOfLinkSymbol *TypeOfLinkSymbol::get_single(const Symbol * ent)
{
    TypeOfLinkSymbol *ls = nullptr;
    string type_name = "entity_ptr<" + ent->name + ">";
    Symbol *sym = get_symbol(type_name);
    if (sym) ls = dynamic_cast<TypeOfLinkSymbol *>(sym);
    else ls = new TypeOfLinkSymbol(ent, true);
    assert(ls); // logic guarantee
    return ls;
}

TypeOfLinkSymbol *TypeOfLinkSymbol::get_multi(const Symbol * ent)
{
    TypeOfLinkSymbol *ls = nullptr;
    string type_name = "multi_link<" + ent->name + ">";
    Symbol *sym = get_symbol(type_name);
    if (sym) ls = dynamic_cast<TypeOfLinkSymbol *>(sym);
    else ls = new TypeOfLinkSymbol(ent, false);
    assert(ls); // logic guarantee
    return ls;
}
