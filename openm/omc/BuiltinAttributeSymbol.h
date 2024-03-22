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
    bool is_base_symbol() const override { return false; }

    /**
     * Constructor by name.
     *
     * @param member_name Name of the member.
     * @param ent         The entity.
     * @param type        The type.
     */
    BuiltinAttributeSymbol(const string member_name, const Symbol *ent, const Symbol *type)
        : AttributeSymbol(member_name, ent, type)
    {
    }

    /**
     * Morphing constructor.
     *
     * @param [in,out] sym If non-null, the symbol.
     * @param ent          The entity.
     * @param type         The type.
     */
    BuiltinAttributeSymbol(Symbol *sym, const Symbol * ent, const Symbol *type)
    : AttributeSymbol(sym, ent, type)
    {
    }

    /**
     * Get the default symbol label for the given language.
     *
     * @param language The language.
     *
     * Overrides the default behaviour implemented in Symbol::default_label
     *
     * @return A string.
     */
    string default_label(const LanguageSymbol& language) const override;

    void post_parse(int pass) override;

    CodeBlock cxx_declaration_entity() override;

    /** The English label */
    string the_english_label;
};

