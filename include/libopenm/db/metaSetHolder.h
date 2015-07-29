/**
 * @file
 * OpenM++ modeling library: holder struct for working sets metadata.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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
        /** workset_lst row */
        WorksetLstRow worksetRow;

        /** workset_parameter table rows */
        vector<WorksetParamRow> worksetParam;
    };

    /** Holder for working sets metadata. */
    struct MetaSetLangHolder : public MetaSetHolder
    {
    public:
        /** workset_txt table rows and language name */
        vector<WorksetTxtLangRow> worksetTxt;

        /** workset_parameter_txt table rows and language name */
        vector<WorksetParamTxtLangRow> worksetParamTxt;
    };
}

#endif  // META_SET_HOLDER_H
