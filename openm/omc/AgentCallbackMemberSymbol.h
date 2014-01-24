/**
* @file    AgentCallbackMemberSymbol.h
* Declarations for the AgentCallbackMemberSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentDataMemberSymbol.h"

using namespace std;

/**
* Agent callback member symbol.
* Contains functionality to support callback functions at containing agent scope.
*/

class AgentCallbackMemberSymbol : public AgentDataMemberSymbol
{
private:
    typedef AgentDataMemberSymbol super;

public:
    AgentCallbackMemberSymbol(Symbol *sym, const Symbol *agent, token_type type, yy::location decl_loc = yy::location())
        : AgentDataMemberSymbol(sym, agent, type, decl_loc)
    {
    }

    AgentCallbackMemberSymbol(const string member_name, const Symbol *agent, token_type type, yy::location decl_loc = yy::location())
        : AgentDataMemberSymbol(member_name, agent, type, decl_loc)
    {
    }

};

