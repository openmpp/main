/**
* @file    BuiltinAttributeSymbol.h
* Declarations for the BuiltinAttributeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "AttributeSymbol.h"

using namespace std;

/**
* BuiltinAttributeSymbol.
*/
class BuiltinAttributeSymbol : public AttributeSymbol
{
private:
    typedef AttributeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor by name.
     *
     * @param member_name Name of the member.
     * @param agent       The agent.
     * @param type        The type.
     */
    BuiltinAttributeSymbol(const string member_name, const Symbol *agent, const Symbol *type)
        : AttributeSymbol(member_name, agent, type)
    {
    }

    /**
     * Morphing constructor.
     *
     * @param [in,out] sym If non-null, the symbol.
     * @param agent        The agent.
     * @param type         The type.
     */
    BuiltinAttributeSymbol(Symbol *sym, const Symbol *agent, const Symbol *type)
    : AttributeSymbol(sym, agent, type)
    {
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

};

