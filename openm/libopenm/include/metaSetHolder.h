/**
 * @file
 * OpenM++ modeling library: holder struct for metadata tables used by working sets.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef META_SET_HOLDER_H
#define META_SET_HOLDER_H

using namespace std;

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"

namespace openm
{
    /** Holder for metadata used to read and write working sets of parameters. */
    struct MetaSetHolder
    {
    public:
        /** working set id to store parameters in database */
        int setId;

        /** model_dic table rows */
        unique_ptr<IModelDicTable> modelDic;

        /** run_option table rows */
        unique_ptr<IRunOptionTable> runOption;

        /** type_dic table rows */
        unique_ptr<ITypeDicTable> typeDic;

        /** type_enum_lst table rows */
        unique_ptr<ITypeEnumLstTable> typeEnumLst;

        /** param_dic table rows */
        unique_ptr<IParamDicTable> paramDic;

        /** param_dims table rows */
        unique_ptr<IParamDimsTable> paramDims;

        /** workset_lst table rows */
        vector<WorksetLstRow> worksetVec;

        /** workset_parameter table rows */
        vector<WorksetParamRow> worksetParamVec;
    };
}

#endif  // META_SET_HOLDER_H
