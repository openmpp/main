/**
* @file    ExtendParameterSymbol.cpp
* Definitions for the ExtendParameterSymbol class.
*/
// Copyright (c) 2013-2018 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ExtendParameterSymbol.h"
#include "ParameterSymbol.h"
#include "EnumerationSymbol.h"
#include "RangeSymbol.h"

using namespace std;

// static
string ExtendParameterSymbol::symbol_name(const Symbol *param)
{
    assert(param);
    return "om_extend_" + param->name;
}

void ExtendParameterSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to target parameter for post-parse use
        pp_target_parameter = dynamic_cast<ParameterSymbol *> (pp_symbol(target_parameter));
        if (!pp_target_parameter) {
            pp_error(LT("error : '") + target_parameter->name + LT("' is not a parameter"));
        }

        // assign direct pointer to index parameter (if present) for post-parse use
        if (index_parameter) {
            pp_index_parameter = dynamic_cast<ParameterSymbol *> (pp_symbol(index_parameter));
            if (!pp_index_parameter) {
                pp_error(LT("error : '") + (*index_parameter)->name + LT("' is not a parameter"));
            }
        }

        break;
    }
    case ePopulateDependencies:
    {
        if (pp_target_parameter->rank() == 0) {
            pp_error(LT("error : target parameter '") + pp_target_parameter->name + LT("' must be vector or matrix"));
            break;
        }
        if (!pp_target_parameter->pp_datatype->is_floating()) {
            pp_error(LT("error : target parameter '") + pp_target_parameter->name + LT("' must be floating point type"));
            break;
        }
        if (pp_target_parameter->is_derived()) {
            pp_error(LT("error : target parameter '") + pp_target_parameter->name + LT("' cannot be derived"));
            break;
        }

        // mark target parameter as extendable
        pp_target_parameter->is_extendable = true;

        if (pp_index_parameter) {
            // target parameter is extended using an index series parameter

            RangeSymbol *target_range = nullptr;
            RangeSymbol *index_range = nullptr;

            if (pp_index_parameter->rank() != 1) {
                pp_error(LT("error : index series parameter '") + pp_index_parameter->name + LT("' must be a vector"));
                break;
            }
            if (!pp_index_parameter->pp_datatype->is_floating()) {
                pp_error(LT("error : index series parameter '") + pp_index_parameter->name + LT("' must be floating point type"));
                break;
            }
            if (pp_index_parameter->pp_datatype != pp_target_parameter->pp_datatype) {
                pp_error(LT("error : index series parameter must be same floating type as target parameter"));
                break;
            }
            if (pp_index_parameter->source != pp_target_parameter->source) {
                pp_error(LT("error : index series parameter must be same source (fixed or scenario) as target parameter"));
                break;
            }
            if (pp_index_parameter->is_derived()) {
                pp_error(LT("error : index series parameter '") + pp_index_parameter->name + LT("' cannot be derived"));
                break;
            }

            {
                // the enumeration of the leading dimension of the target parameter
                auto enumeration = pp_target_parameter->pp_enumeration_list.front();
                if (enumeration->is_range()) {
                    target_range = dynamic_cast<RangeSymbol *> (enumeration);
                }
                else {
                    pp_error(LT("error : target parameter '") + pp_target_parameter->name + LT("' leading dimension must be a range"));
                    break;
                }
            }

            {
                // the enumeration of the index series vector parameter
                auto enumeration = pp_index_parameter->pp_enumeration_list.front();
                if (enumeration->is_range()) {
                    index_range = dynamic_cast<RangeSymbol *> (enumeration);
                }
                else {
                    pp_error(LT("error : index series parameter '") + pp_target_parameter->name + LT("' dimension must be a range"));
                    break;
                }
            }
            assert(target_range);
            assert(index_range);
            if ((target_range->lower_bound < index_range->lower_bound) || (target_range->upper_bound > index_range->upper_bound)) {
                pp_error(
                    LT("error : range of target parameter '") + target_range->name +
                    LT("' not contained within range of index series parameter '") + index_range->name +
                    LT("'")
                );
                break;
            }
            // note the index series parameter used to extend the target parameter
            pp_target_parameter->pp_index_series = pp_index_parameter;
            // note the offset in the target parameter
            pp_target_parameter->index_series_offset = target_range->lower_bound - index_range->lower_bound;
        }

        break;
    }
    default:
        break;
    }
}