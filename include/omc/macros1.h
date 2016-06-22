/**
* @file    macros1.h
* Macros available to all models (late).
*
* includes min/max macros which can break std headers if not included last
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

// override std:min and std::max to enable type conversion of arguments
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
