/**
 * @file
 * OpenM++ modeling library: holder struct for metadata tables used by model.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef META_HOLDER_H
#define META_HOLDER_H

using namespace std;

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"

namespace openm
{
    /** Holder for model metadata core part (not include model text). */
    struct MetaHolder
    {
    public:
        /** model_dic row for the model */
        const ModelDicRow * modelRow;

        /** model_dic table rows */
        unique_ptr<IModelDicTable> modelTable;

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

        /** table_expr table rows */
        unique_ptr<ITableExprTable> tableExpr;

        /** create empty model metadata holder */
        MetaHolder(void) : modelRow(nullptr) { }
    };
}

#endif  // META_HOLDER_H
