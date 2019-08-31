/**
* @file    helper.h
* Helper code used internally by run-time
*/
// Copyright (c) 2013-2018 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <string>
#include <sstream>

#include "libopenm/omLog.h"
#include "libopenm/common/omHelper.h" // for openm::SimulationException


namespace omr {

    /**
     * Populates an extended parameter
     *
     * @exception openm::SimulationException Thrown when a Simulation error condition occurs.
     *
     * @tparam T Generic type parameter.
     * @param          name         The name of the target parameter
     * @param [in,out] target       Target flattened parameter to be extended.
     * @param          dim0_size    The size of the first dimension of target, e.g. year.
     * @param          slice_size   Size of a slice of target, e.g. values for a year.
     * @param          index_series
     * (Optional) If non-null, the index series used to grow missing
     * values.
     * @param          index_offset
     * (Optional) Offset to current year in index series.
     * 
     * Parameters are extended using the first dimension (e.g. YEAR).
     * When the parameter is flattened, elements which are consecutive
     * in YEAR will be a fixed distance apart in the flattened array.
     * That distance is the number of elements in a YEAR slice of the
     * array (slice_size).
     *
     * ### tparam T Generic type parameter.
     */
    template<class T>
    void populate_extended_parameter(const std::string name, T *target, const size_t dim0_size, const size_t slice_size, T const *index_series = nullptr, const size_t index_offset = 0)
    {
        for (size_t j = 0; j < slice_size; ++j) {
            int idx = j;
            T prev = 0;
            for (size_t k = 0; k < dim0_size; ++k) {
                auto& value = target[idx];
                if (isnan(value)) {
                    if (k == 0) {
                        std::stringstream ss;
                        ss << "error : Cannot extend parameter '" << name << "' when first value is missing ";
                        ss << "(at flattened slice " << j << ")";;
                        auto msg = ss.str().c_str();
                        theLog->logMsg(msg);
                        throw openm::SimulationException(msg);
                    }
                    else {
                        if (index_series) {
                            auto num = index_series[index_offset + k];
                            auto den = index_series[index_offset + k - 1];
                            if (den == 0) {
                                std::stringstream ss;
                                ss << "error : Cannot extend parameter '" << name << "' when when index series value is 0 ";
                                ss << "(at flattened slice " << j << " and series index" << (index_offset + k - 1) << ")";;
                                auto msg = ss.str().c_str();
                                theLog->logMsg(msg);
                                throw openm::SimulationException(msg);
                            }
                            value = prev * (num / den); // grow previous value
                        }
                        else {
                            value = prev; // duplicate previous value
                        }
                    }
                }
                prev = value;
                idx += slice_size;
            }
        }
    }


};



