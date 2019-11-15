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
        
        /** return type enums size: number of enums for that type or zero if no enumms found (most of built-in types) */
        size_t typeEnumsSize(int i_modelId, int i_typeId) const;

        /** return input parameter size: total number of values in the parameter as product of all of dimensions size */
        size_t parameterSize(const ParamDicRow & paramRow) const;

        /** return accumulator size: total number of accumulator values as product of dimensions size, including total item */
        size_t accumulatorSize(const TableDicRow & i_tableRow) const;
    };
}

#endif  // META_HOLDER_H
