/**
 * @file   custom_early.h
 * Model-specific declarations and includes (early)
 *  
 * This file is included in compiler-generated files for the model.
 * This file comes early in the include order, before entity declarations.
 * It is an appropriate place to put forward declarations of classes
 * used in entity function declarations.
 */

#pragma once
#include <string>
typedef std::string std_string; // Can be used instead of std::string in model code, to avoid Modgen issues when using 'string' in model code.

#include "omc/microdata_csv.h"

#if !defined(OPENM)
namespace mm {
#endif

/**
 * Information about the case.
 * 
 * Some case-based models need to communicate additional information to the case, or between
 * cases.  This structure can be passed as a function argument for that purpose.  It is required
 * but can be left empty.
 */
struct case_info {
    /**
     * Microdata output csv object
     */
    output_csv out_csv;
};

#if !defined(OPENM)
}
#endif

