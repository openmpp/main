/**
 * @file   custom.h
 * Model-specific declarations and includes (late)
 *  
 * This file is included in all compiler-generated files for the model.
 * This file comes late in the include order, after entity declarations.
 */

#pragma once
#include "case_based.h"

#if defined(MODGEN)
namespace mm {
    //typedef EMOTION EMOTION_t;
    //typedef TIME TIME_t;
}
#else
//typedef om_enum_EMOTION EMOTION_t;
//typedef double TIME_t;
#endif
