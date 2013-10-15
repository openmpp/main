/**
 * @file
 * OpenM++ modeling library: holder struct for metadata tables used by model.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef META_RUN_HOLDER_H
#define META_RUN_HOLDER_H

using namespace std;

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"

namespace openm
{
    /** Holder for metadata used to run the model. */
    struct MetaRunHolder
    {
    public:
        /** model run id to store results and parameters in database */
        int runId;

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

        /** table_dic table rows */
        unique_ptr<ITableDicTable> tableDic;

        /** table_dims table rows */
        unique_ptr<ITableDimsTable> tableDims;

        /** table_acc table rows */
        unique_ptr<ITableAccTable> tableAcc;

        /** table_unit table rows */
        unique_ptr<ITableUnitTable> tableUnit;
    };
}

#endif  // META_RUN_HOLDER_H
