/**
 * @file
 * OpenM++ modeling library: holder struct for working sets metadata.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef META_SET_HOLDER_H
#define META_SET_HOLDER_H

using namespace std;

#include "libopenm/db/dbMetaRow.h"

namespace openm
{
    /** Holder for working sets metadata. */
    struct MetaSetHolder
    {
    public:
        /** workset_lst table row */
        WorksetLstRow worksetRow;

        /** workset_txt table rows */
        vector<WorksetTxtRow> worksetTxtVec;

        /** workset_parameter table rows */
        vector<WorksetParamRow> worksetParamVec;

        /** workset_parameter_txt table rows */
        vector<WorksetParamTxtRow> worksetParamTxtVec;
    };
}

#endif  // META_SET_HOLDER_H
