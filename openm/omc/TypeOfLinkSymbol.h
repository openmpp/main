/**
* @file    TypeOfLinkSymbol.h
* Declarations for the TypeOfLinkSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "TypeSymbol.h"

/**
* TypeOfLinkSymbol.
*
*/
class TypeOfLinkSymbol : public TypeSymbol
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
    TypeOfLinkSymbol(const Symbol *agent, bool single)
        : TypeSymbol((single ? "link<" : "multi_link<") + agent->name + ">")
        , single(single)
    {
    }

    /**
     * Gets the default initial value for a quantity of this type.
     *
     * @return The default initial value as a string.
     */
    const string default_initial_value() const {
        return single ? "nullptr" : "";
    };

    /**
     * true if this object is the type for a single link, false if for a multi-link.
     */
    bool single;

    /**
     * Gets the TypeOfLinkSymbol object for the type which points to a single given kind of agent.
     * 
     * Created if it doesn't already exist.
     *
     * @param agent The agent pointed to by the link type.
     *
     * @return Pointer to the TypeOfLinkSymbol object requested.
     */
    static TypeOfLinkSymbol *get_single(const Symbol *agent);

    /**
     * Gets the TypeOfLinkSymbol object for the type which holds a set of pointers to a given kind of agent.
     * 
     * Created if it doesn't already exist.
     *
     * @param agent The agent pointed to by the link type.
     *
     * @return Pointer to the TypeOfLinkSymbol object requested.
     */
    static TypeOfLinkSymbol *get_multi(const Symbol *agent);


};

