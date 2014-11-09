/**
* @file    ClassificationSymbol.cpp
* Definitions for the ClassificationSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <algorithm>
#include <list>
#include <cctype>
#include "ClassificationSymbol.h"
#include "EnumeratorSymbol.h"
#include "CodeBlock.h"

using namespace std;
using namespace openm;

void ClassificationSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateDependencies:
        {
            // Information to compute storage type is known in this pass
            storage_type = optimized_storage_type(0, pp_size() - 1);

            // Determine which integer-classification transformation functions to emit

            // Check for presence of a trailing digit in any of the classification enumerators
            bool any_trailing_digits = 
                any_of(
                    pp_enumerators.cbegin(),
                    pp_enumerators.cend(),
                    [](EnumeratorSymbol *es){ return isdigit(es->name.back()); }
                );
            if (any_trailing_digits) {
                // Only emit code for the function if it is actually used in developer code.
                if (identifiers_in_model_source.end() != identifiers_in_model_source.find("IntIs_" + name)) {
                    pp_generate_IntIs = true;
                }
                if (identifiers_in_model_source.end() != identifiers_in_model_source.find("IntTo_" + name)) {
                    pp_generate_IntTo = true;
                }
                if (identifiers_in_model_source.end() != identifiers_in_model_source.find("IntFrom_" + name)) {
                    pp_generate_IntFrom = true;
                }
            }
        }
        break;
    default:
        break;
    }
}

CodeBlock ClassificationSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    h += "";
    h += doxygen("Classification: " + name);
    h += "enum om_enum_" + name + " : " + token_to_string(storage_type) + " {";
    bool first = true;
    for (auto enumerator : pp_enumerators) {
        h += doxygen_short(enumerator->name);
        if (first) {
            h += enumerator->name + " = 0," ;
            first = false;
        }
        else {
            h += enumerator->name + "," ;
        }
        h += "";
    }
    h += "};";
    h += "typedef Classification<om_enum_" + name + ", " + to_string(pp_size()) + "> " + name + ";" ;

    if (pp_generate_IntIs) {
        h += "";
        h += doxygen(
            "Check an integer value for membership in classification " + name + ".",
            " ",
            "Classification enumerators can have an associated integer value",
            "based on the trailing digits of their name.",
            " ",
            "@param number The integer value to query.",
            " ",
            "@return true if there is a matching enumerator, false otherwise."
            );
        h += "bool IntIs_" + name + "(int number);";
    }
    if (pp_generate_IntTo) {
        h += "";
        h += doxygen(
            "Transform an integer to the associated enumerator in the classification " + name + ".",
            " ",
            "Classification enumerators can have an associated integer value",
            "based on the trailing digits of their name.",
            " ",
            "@param number The integer value to transform.",
            " ",
            "@return The corresponding enumerator, or the final enumerator if none matches."
            );
        h += name + " IntTo_" + name + "(int number);";
    }
    if (pp_generate_IntFrom) {
        h += "";
        h += doxygen(
            "Transform a classification enumerator to the associated integer in classification " + name + ".",
            " ",
            "Classification enumerators can have an associated integer value",
            "based on the trailing digits of their name.",
            " ",
            "@param number The eumerator to transform.",
            " ",
            "@return The corresponding integer, or -1 if there is none."
            );
        h += "int IntFrom_" + name + "(" + name + " level);";
    }

    return h;
}

CodeBlock ClassificationSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.

    if (pp_generate_IntIs) {
        c += "";
        c += "bool IntIs_" + name + "(int number)";
        c += "{";
        c += "switch (number) {";
        unordered_set<string> numbers_used; // to check for duplicates
        for (auto es : pp_enumerators) {
            string name = es->name;
            if (isdigit(name.back())) {
                string number = name.substr(name.find_last_not_of("0123456789") + 1);
                // Remove any leading zeros, since leading zero means octal in a C++ literal.
                int j = number.find_first_not_of("0");
                if (j == std::string::npos) {
                    // if all 0's, leave the final '0' digit
                    j = number.length() - 1;
                }
                number = number.substr(j);
                if (numbers_used.end() == numbers_used.find(number)) {
                    c += "case " + number + ": return true; // " + name;
                    numbers_used.insert(number);
                }
            }
        }
        c += "default: return false;";
        c += "}";
        c += "}";
    }
    if (pp_generate_IntTo) {
        c += "";
        c += name + " IntTo_" + name + "(int number)";
        c += "{";
        c += "switch (number) {";
        unordered_set<string> numbers_used; // to check for duplicates
        for (auto es : pp_enumerators) {
            string name = es->name;
            if (isdigit(name.back())) {
                string number = name.substr(name.find_last_not_of("0123456789") + 1);
                // Remove any leading zeros, since leading zero means octal in a C++ literal.
                int j = number.find_first_not_of("0");
                if (j == std::string::npos) {
                    // if all 0's, leave the final '0' digit
                    j = number.length() - 1;
                }
                number = number.substr(j);
                if (numbers_used.end() == numbers_used.find(number)) {
                    c += "case " + number + ": return " + name + ";";
                    numbers_used.insert(number);
                }
            }
        }
        c += "default: return " + pp_enumerators.back()->name + ";";
        c += "}";
        c += "}";
    }
    if (pp_generate_IntFrom) {
        c += "";
        c += "int IntFrom_" + name + "(" + name + " level)";
        c += "{";
        c += "switch (level) {";
        for (auto es : pp_enumerators) {
            string name = es->name;
            if (isdigit(name.back())) {
                string number = name.substr(name.find_last_not_of("0123456789") + 1);
                // Remove any leading zeros, since leading zero means octal in a C++ literal.
                int j = number.find_first_not_of("0");
                if (j == std::string::npos) {
                    // if all 0's, leave the final '0' digit
                    j = number.length() - 1;
                }
                number = number.substr(j);
                c += "case " + name + ": return " + number + ";";
            }
            else {
                c += "case " + name + ": return -1;";
            }
        }
        c += "default: return -1;";
        c += "}";
        c += "}";
    }

    return c;
}

bool ClassificationSymbol::is_valid_constant(const Constant &k) const
{
    // only named enumerators are valid for a classification
    if (!k.is_enumerator) return false;

    // check that the enumerator value is in the enumeration's map of enumerators to ordinals
    if (pp_name_to_int.count(k.value()) == 0) return false;

    return true;
}

string ClassificationSymbol::format_for_storage(const Constant &k) const
{
    // Get the ordinal corresponding to the enumerator name using 
    // the enumeration's map of enumerators to ordinals
    auto it = pp_name_to_int.find(k.value());
    assert(it != pp_name_to_int.end()); // validity of initializers was already verified
    auto ordinal = it->second;

    string result = to_string(ordinal);
    return result;
}

bool ClassificationSymbol::candidate_for_integer_transforms()
{
    return any_of(pp_enumerators.cbegin(), pp_enumerators.cend(), [](EnumeratorSymbol *es){ return isdigit(es->name.back()); });
}

