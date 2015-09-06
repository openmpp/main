/**
 * @file    SpecialGlobal.h
 * Declarations for the SpecialGlobal class.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include <list>

using namespace std;

/**
 * Helper class for special global functions PreSimulation, PostSimulation, UserTables
 * 
 * One instance exists for each of PreSimulation, PostSimulation, UserTables.
 */
class SpecialGlobal
{
public:
    SpecialGlobal(string prefix)
        : prefix(prefix)
        , ambiguous_count(0)
    {

    }

    /**
     * Disambiguated name associated with a numeric identifier.
     *
     * @param id The numeric id.
     *
     * @return A string.
     */
    string disambiguated_name(size_t id) const
    {
        return "om_" + prefix + "_" + to_string(id);
    }

    /**
     * The prefix, e.g. "PreSimulation", "PostSimulation", "UserTables"
     */
    const string prefix;

    /**
     * Number of ambiguous definitions of function in model code
     * 
     * Ambiguous definitions have no suffix, e.g. "PreSimulation".
     */
    size_t ambiguous_count;

    /**
     * All suffixes found in source code
     * 
     * E.g. PreSimulation23 has suffix "23".  Duplicates are included, if present in model source
     * code.
     */
    list<string> suffixes;
};