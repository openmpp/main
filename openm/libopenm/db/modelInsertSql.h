/**
 * @file
 * OpenM++ data library: class to produce insert sql statements for new model
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MODEL_INSERT_SQL_H
#define MODEL_INSERT_SQL_H

#include <cfloat>
#include "libopenm/db/dbMetaRow.h"
#include "libopenm/db/modelBuilder.h"
#include "helper.h"
#include "dbExec.h"
#include "modelSqlWriter.h"
using namespace std;

namespace openm
{
    /** metadata inserter class: produce sql statements and insert metadata rows into db tables. */
    class ModelInsertSql
    {
    public:
        /** create new sql inserter class */
        ModelInsertSql(void);

        /** insert model master row into model_dic table. */
        void insertModelDic(IDbExec * i_dbExec, ModelDicRow & io_row) const;

        /** insert row into model_dic_txt table. */
        void insertModelDicText(IDbExec * i_dbExec, const map<string, int> & i_langMap, ModelDicTxtLangRow & io_row) const;

        /** insert row into type_dic and model_type_dic tables, return type Hid. */
        int insertTypeDic(IDbExec * i_dbExec, const TypeDicRow & i_row) const;

        /** insert row into type_dic_txt table. */
        void insertTypeText(
            IDbExec * i_dbExec, const map<string, int> & i_langMap, const map<int, int> & i_typeIdMap, TypeDicTxtLangRow & io_row
        ) const;

        /** insert row into type_enum_lst table. */
        void insertTypeEnum(IDbExec * i_dbExec, int i_typeHid, const TypeEnumLstRow & i_row) const;

        /** insert row into type_enum_txt table. */
        void insertTypeEnumText(
            IDbExec * i_dbExec, const map<string, int> & i_langMap, int i_typeHid, TypeEnumTxtLangRow & io_row
        ) const;

        /** insert row into parameter_dic and model_parameter_dic tables. */
        void insertParamDic(IDbExec * i_dbExec, const map<int, int> & i_typeIdMap, ParamDicRow & io_row) const;

        /** insert row into parameter_dic_txt table. */
        void insertParamText(
            IDbExec * i_dbExec, const ParamDicRow & i_paramRow, const map<string, int> & i_langMap, ParamDicTxtLangRow & io_row
        ) const;

        /** insert row into parameter_dims table. */
        void insertParamDims(
            IDbExec * i_dbExec, const ParamDicRow & i_paramRow, const map<int, int> & i_typeIdMap, const ParamDimsRow & i_row
        ) const;

        /** insert row into parameter_dims_txt table. */
        void insertParamDimsText(
            IDbExec * i_dbExec, const ParamDicRow & i_paramRow, const map<string, int> & i_langMap, ParamDimsTxtLangRow & io_row
        ) const;

        /** insert row into table_dic and model_table_dic tables. */
        void insertTableDic(IDbExec * i_dbExec, TableDicRow & io_row) const;

        /** insert row into table_dic_txt table. */
        void insertTableText(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableDicTxtLangRow & io_row
        ) const;

        /** insert row into table_dims table. */
        void insertTableDims(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<int, int> & i_typeIdMap, const TableDimsRow & i_row
        ) const;

        /** insert row into table_dims_txt table. */
        void insertTableDimsText(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableDimsTxtLangRow & io_row
        ) const;

        /** insert row into table_acc table. */
        void insertTableAcc(IDbExec * i_dbExec, const TableDicRow & i_tableRow, const TableAccRow & i_row) const;

        /** insert row into table_acc_txt table. */
        void insertTableAccText(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableAccTxtLangRow & io_row
        ) const;

        /** insert row into table_expr table. */
        void insertTableExpr(IDbExec * i_dbExec, const TableDicRow & i_tableRow, const TableExprRow & i_row) const;

        /** insert row into table_expr_txt table. */
        void insertTableExprText(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableExprTxtLangRow & io_row
        ) const;

        /** insert row into group_lst table. */
        void insertGroupLst(IDbExec * i_dbExec, const GroupLstRow & i_row) const;

        /** insert row into group_txt table. */
        void insertGroupText(IDbExec * i_dbExec, const map<string, int> & i_langMap, GroupTxtLangRow & io_row) const;

        /** insert row into group_pc table. */
        void insertGroupPc(IDbExec * i_dbExec, const GroupPcRow & i_row) const;

        /** create new workset: insert metadata and delete existing workset parameters, if required */
        void createWorksetMeta(IDbExec * i_dbExec, const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet) const;

    private:
        locale defaultLoc;  // default locale for trim
    };
}

#endif  // MODEL_INSERT_SQL_H
