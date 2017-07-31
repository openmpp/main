/**
* @file    UnknownTypeSymbol.cpp
* Definitions for the UnknownTypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "UnknownTypeSymbol.h"

using namespace std;


//static
UnknownTypeSymbol *UnknownTypeSymbol::find()
{
    auto bs = dynamic_cast<UnknownTypeSymbol *>(get_symbol("om_unknown"));
    assert(bs); // only called when valid
    return bs;
}
