/**
* @file    AggregationSymbol.cpp
* Definitions for the AggregationSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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
            pp_error(LT("error : '") + to->name + LT("' must be a classification in the aggregation"));
            break;
        }
        pp_from = dynamic_cast<ClassificationSymbol *> (pp_symbol(from));
        if (!pp_from) {
            pp_error(LT("error : '") + from->name + LT("' must be a classification in the aggregation"));
            break;
        }
        if (0 != symbol_list.size() % 2) {
            pp_error(LT("error : non-even number of enumerators in the aggregation"));
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
                pp_error(LT("error : '") + pp_sym->name + LT("' must be an enumerator in the aggregation"));
                break;
            }
            if (is_first) {
                to_enumerator = enumerator;
            }
            else { // second
                from_enumerator = enumerator;
                if (pp_enumerator_map.count(from_enumerator) != 0) {
                    pp_error(LT("error : '") + from_enumerator->name + LT("' specified more than once in the aggregation"));
                }
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
                pp_error(LT("error : enumerator '") + enumerator_from->name + LT("' not found in classification '") + pp_from->name + LT("'"));
            }
            auto enumerator_to = element.second;
            if (enumerator_to->pp_enumeration != pp_to) {
                pp_error(LT("error : enumerator '") + enumerator_to->name + LT("' not found in classification '") + pp_to->name + LT("'"));
            }
        }

        // Check that aggregation is complete, i.e.
        // and that all values in the 'from' classification are present.
        for (auto enumerator : pp_from->pp_enumerators) {
            auto element = pp_enumerator_map.find(enumerator);
            if (element == pp_enumerator_map.end()) {
                pp_error(LT("error : required enumerator '") + enumerator->name + LT("' is missing in aggregation"));
            }
        }

        // Only emit code for the XXX_To_YYY function if it is actually used in developer code.
        if (identifiers_in_model_source.end() != identifiers_in_model_source.find(pp_helper_name())) {
            pp_generate_To = true;
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
    h += doxygen_short("Helper map to implement the aggregation from " + from_type + " to " + to_type);
    h += "extern const std::map<" + from_type +", " + to_type + "> " + name + ";";
    if (pp_generate_To) {
        h += doxygen_short("Map an enumerator in " + from_type + " to the corresponding enumerator in " + to_type);
        h += to_type + " " + pp_helper_name() + "(" + from_type + " level);";
    }

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
    string helper_name = from_type + "_To_" + to_type; // name of the generated XX_To_YY function
    c += "const std::map<" + from_type +", " + to_type + "> " + name + " = {";
    for (auto from_enumerator : pp_from->pp_enumerators) {
        auto element = pp_enumerator_map.find(from_enumerator);
        assert(element != pp_enumerator_map.end()); // guarantee - aggregation validated
        auto to_enumerator = element->second;
        assert(to_enumerator);
        c += "{ " + from_enumerator->name + ", " + to_enumerator->name + " },";
    }
    c += "};";

    if (pp_generate_To) {
        c += to_type + " " + pp_helper_name() + "(" + from_type + " level)";
        c += "{";
        c += "return " + name + ".find(level)->second;";
        c += "}";
    }

    return c;
}

string AggregationSymbol::pp_helper_name() const
{
    assert(pp_from);
    assert(pp_to);
    return pp_from->name + "_To_" + pp_to->name;
}
