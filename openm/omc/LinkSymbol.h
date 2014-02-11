/**
* @file    LinkSymbol.h
* Declarations for the LinkSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "TypeSymbol.h"

/**
* LinkSymbol.
*
*/
class LinkSymbol : public TypeSymbol
{
private:
    typedef TypeSymbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
     * Constructor.
     *
     * @param agent The agent.
     */
    LinkSymbol(const Symbol *agent)
        : TypeSymbol("link<" + agent->name + ">")
    {
    }

    /**
     * Gets the default initial value for a quantity of this type.
     *
     * @return The default initial value as a string.
     */
    const string default_initial_value() const {
        return "nullptr";
    };

    /**
     * Gets the LinkSymbol object for the type which points to a given kind of agent.
     * 
     * Created if it doesn't already exist.
     *
     * @param agent The agent pointed to by the link type.
     *
     * @return Pointer to the LinkSymbol object requested.
     */
    static LinkSymbol *get(const Symbol *agent);


};

