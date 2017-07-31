/**
* @file    ForeignTypeSymbol.cpp
* Definitions for the ForeignTypeSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "ForeignTypeSymbol.h"

using namespace std;


//static
ForeignTypeSymbol *ForeignTypeSymbol::find()
{
    auto bs = dynamic_cast<ForeignTypeSymbol *>(get_symbol("om_unknown"));
    assert(bs); // only called when valid
    return bs;
}
