/**
 * @file
 * OpenM++ modeling library: holder struct for metadata tables used by model.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef META_HOLDER_H
#define META_HOLDER_H

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"

using namespace std;

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

        /** model_parameter_import table rows */
        unique_ptr<IParamImportTable> paramImport;

        /** table_dic table rows */
        unique_ptr<ITableDicTable> tableDic;

        /** table_dims table rows */
        unique_ptr<ITableDimsTable> tableDims;

        /** table_acc table rows */
        unique_ptr<ITableAccTable> tableAcc;

        /** table_expr table rows */
        unique_ptr<ITableExprTable> tableExpr;

        /** lang_lst table rows */
        unique_ptr<ILangLstTable> langLst;

        /** entity_dic table rows */
        unique_ptr<IEntityDicTable> entityDic;

        /** entity_attr table rows */
        unique_ptr<IEntityAttrTable> entityAttr;

        /** group_lst table rows */
        unique_ptr<IGroupLstTable> groupLst;

        /** group_pc table rows */
        unique_ptr<IGroupPcTable> groupPc;

        /** entity_group_lst table rows */
        unique_ptr<IEntityGroupLstTable> entityGroupLst;

        /** entity_group_pc table rows */
        unique_ptr<IEntityGroupPcTable> entityGroupPc;

        /** run_option table rows */
        unique_ptr<IRunOptionTable> runOptionTable;

        /** create empty model metadata holder */
        MetaHolder(void) : modelRow(nullptr) { }
        
        /** return type enums size: number of enums for that type or zero if no enums found (most of built-in types) */
        size_t typeEnumsSize(int i_modelId, int i_typeId) const;

        /** return input parameter size of single sub value: total number of values in the parameter as product of all of dimensions size */
        size_t parameterSize(const ParamDicRow & paramRow) const;

        /** return accumulator size: total number of accumulator values as product of dimensions size, including total item */
        size_t accumulatorSize(const TableDicRow & i_tableRow) const;
    };
}

#endif  // META_HOLDER_H
