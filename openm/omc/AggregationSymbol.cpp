/**
* @file    AggregationSymbol.cpp
* Definitions for the AggregationSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "CodeBlock.h"
#include "AggregationSymbol.h"

using namespace std;

// static
string AggregationSymbol::symbol_name(const Symbol* from, const Symbol* to)
{
    return "om_aggregation_" + from->name + "_om_" + to->name;
}


// static
bool AggregationSymbol::exists(const Symbol* from, const Symbol* to)
{
    return Symbol::exists(symbol_name(from, to));
}

void AggregationSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to classifications for use post-parse
        pp_to = dynamic_cast<ClassificationSymbol *> (pp_symbol(to));
        if (!pp_to) {
            pp_error("error: '" + to->name + "' must be a classification in the aggregation");
            break;
        }
        pp_from = dynamic_cast<ClassificationSymbol *> (pp_symbol(from));
        if (!pp_from) {
            pp_error("error: '" + from->name + "' must be a classification in the aggregation");
            break;
        }
        if (0 != symbol_list.size() % 2) {
            pp_error("error: Non-even number of enumerators in the aggregation");
            break;
        }
        
        // build the map
        bool is_first = true;
        EnumeratorSymbol * to_enumerator = nullptr;
        EnumeratorSymbol * from_enumerator = nullptr;
        for (auto sym : symbol_list) {
            auto pp_sym = pp_symbol(sym);
            assert(pp_sym); // scanner guarantee
            auto enumerator = dynamic_cast<EnumeratorSymbol *>(pp_sym);
            if (!enumerator) {
                pp_error("error: '" + pp_sym->name + "' must be an enumerator in the aggregation");
                break;
            }
            if (is_first) {
                to_enumerator = enumerator;
            }
            else { // second
                from_enumerator = enumerator;
                // insert into the map
                pp_enumerator_map.emplace(from_enumerator, to_enumerator);
            }
            is_first = !is_first;
        }

        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of aggregations
        pp_all_aggregations.push_back(this);
        break;
    }
    case ePopulateDependencies:
    {
        // Validate the aggregation.

        // Check that all keys are enumerators of the 'from' classification
        // and that all values are enumerators of the 'to' classification.
        for (auto element : pp_enumerator_map) {
            auto enumerator_from = element.first;
            if (enumerator_from->pp_enumeration != pp_from) {
                pp_error("error: enumerator '" + enumerator_from->name + "' not found in classification '" + pp_from->name + "'");
            }
            auto enumerator_to = element.second;
            if (enumerator_to->pp_enumeration != pp_to) {
                pp_error("error: enumerator '" + enumerator_to->name + "' not found in classification '" + pp_to->name + "'");
            }
        }

        // Check that aggregation is complete, i.e.
        // and that all values in the 'from' classification are present.
        for (auto enumerator : pp_from->pp_enumerators) {
            auto element = pp_enumerator_map.find(enumerator);
            if (element == pp_enumerator_map.end()) {
                pp_error("error: required enumerator '" + enumerator->name + "' is missing in aggregation");
            }
        }

        break;
    }
    default:
        break;
    }
}

CodeBlock AggregationSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "";
    string from_type = pp_from->name;
    string to_type = pp_to->name;
    h += "extern const map<" + from_type +", " + to_type + "> " + name + ";";

    return h;
}

CodeBlock AggregationSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "";
    string from_type = pp_from->name;
    string to_type = pp_to->name;
    c += "const map<" + from_type +", " + to_type + "> " + name + " = {";
    for (auto from_enumerator : pp_from->pp_enumerators) {
        auto element = pp_enumerator_map.find(from_enumerator);
        assert(element != pp_enumerator_map.end()); // guarantee - aggregation validated
        auto to_enumerator = element->second;
        assert(to_enumerator);
        c += "{ " + from_enumerator->name + ", " + to_enumerator->name + " },";
    }
    c += "};";

    return c;
}
