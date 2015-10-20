/**
* @file    EntityInternalSymbol.h
* Declarations for the EntityInternalSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "EntityDataMemberSymbol.h"

using namespace std;

/**
* Internal data symbol.
* Abstracts a member of an agent class which stores information
* used internally by om.  These members are not accessible to
* developer code, and have no side-effects.
*/

class EntityInternalSymbol : public EntityDataMemberSymbol
{
private:
    typedef EntityDataMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityInternalSymbol(const string member_name, const Symbol *agent, const Symbol *type, const string initializer = "")
        : EntityDataMemberSymbol(member_name, agent, type)
        , initializer(initializer)
    {
    }

    string initialization_value(bool type_default) const;

    CodeBlock cxx_initialization_expression(bool type_default) const;

    CodeBlock cxx_declaration_agent();

    string initializer;
};

