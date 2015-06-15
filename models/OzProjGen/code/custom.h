/**
 * @file   custom.h
 * Custom include file for the model
 *  
 * This file is included in all compiler-generated files for the model.
 * Place any required globals declarations in this file, or include other
 * files in this file, for correct placement in compiler-generated files.
 */

#pragma once
#include <fstream>

/**
 * Information about the case.
 * 
 * Some case-based models need to communicate additional information to the case, or between
 * cases.  This structure can be used for that purpose.  It is required but can be left empty.
 */
struct case_info {

    /**
     * Stream to write data to.
     */
    ofstream *output_stream;
};
