/**
* @file    EntityForeignMemberSymbol.h
* Declarations for the EntityForeignMemberSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "EntityDataMemberSymbol.h"
#include "EnumerationSymbol.h"

using namespace std;

/**
* Foreign data symbol.
* Abstracts a member of an entity class which stores information
* using foreign types which are declared by the model developer.
* These members do not participate in the ompp mechanisms
* to maintain dependencies, tabulation, etc.
*/

class EntityForeignMemberSymbol : public EntityDataMemberSymbol
{
private:
    typedef EntityDataMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Morphing constructor.
     *
     * @param [in,out] sym If non-null, the symbol.
     * @param agent        The agent.
     * @param type         The type.
     */
    EntityForeignMemberSymbol(Symbol *sym, const Symbol *agent, const Symbol *type, yy::location decl_loc = yy::location())
        : EntityDataMemberSymbol(sym, agent, type, decl_loc)
    {
    }

    CodeBlock cxx_initialization_expression(bool type_default) const;

    void post_parse(int pass);

    CodeBlock cxx_declaration_agent();

};

