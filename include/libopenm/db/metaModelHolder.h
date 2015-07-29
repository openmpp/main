/**
 * @file
 * OpenM++ modeling library: holder struct for model metadata.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef META_MODEL_HOLDER_H
#define META_MODEL_HOLDER_H

using namespace std;

#include "dbMetaRow.h"

namespace openm
{
    /** Holder for model metadata db rows. */
    struct MetaModelHolder
    {
    public:
        /** model_dic table row */
        ModelDicRow modelDic;

        /** model_dic_txt table rows and language name */
        vector<ModelDicTxtLangRow> modelTxt;

        /** type_dic table rows */
        vector<TypeDicRow> typeDic;

        /** type_dic_txt table rows and language name */
        vector<TypeDicTxtLangRow> typeTxt;

        /** type_enum_lst table rows */
        vector<TypeEnumLstRow> typeEnum;

        /** type_enum_txt table rows and language name */
        vector<TypeEnumTxtLangRow> typeEnumTxt;

        /** parameter_dic table rows */
        vector<ParamDicRow> paramDic;

        /** parameter_dic_txt table rows and language name */
        vector<ParamDicTxtLangRow> paramTxt;

        /** parameter_dims table rows */
        vector<ParamDimsRow> paramDims;

        /** table_dic table rows */
        vector<TableDicRow> tableDic;

        /** table_dic_txt table rows and language name */
        vector<TableDicTxtLangRow> tableTxt;

        /** table_dims table rows */
        vector<TableDimsRow> tableDims;

        /** table_dims_txt table rows and language name */
        vector<TableDimsTxtLangRow> tableDimsTxt;

        /** table_acc table rows */
        vector<TableAccRow> tableAcc;

        /** table_acc_txt table rows and language name */
        vector<TableAccTxtLangRow> tableAccTxt;

        /** table_expr table rows */
        vector<TableExprRow> tableExpr;

        /** table_expr_txt table rows and language name */
        vector<TableExprTxtLangRow> tableExprTxt;

        /** group_lst table rows */
        vector<GroupLstRow> groupLst;

        /** group_txt table rows and language name */
        vector<GroupTxtLangRow> groupTxt;

        /** group_pc table rows */
        vector<GroupPcRow> groupPc;
    };
}

#endif  // META_MODEL_HOLDER_H
