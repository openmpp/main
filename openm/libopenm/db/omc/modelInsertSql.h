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
        /** insert model master row into model_dic table. */
        static void insertModelDic(IDbExec * i_dbExec, ModelDicLangRow & io_row, const map<string, int> & i_langMap);

        /** insert language into lang_lst table, if not exist. */
        static void insertLangLst(IDbExec * i_dbExec, LangLstRow & io_row);

        /** insert row into model_dic_txt table. */
        static void insertModelDicText(IDbExec * i_dbExec, const map<string, int> & i_langMap, ModelDicTxtLangRow & io_row);

        /** insert row into type_dic and model_type_dic tables, return type Hid. */
        static int insertTypeDic(IDbExec * i_dbExec, const TypeDicRow & i_row);

        /** insert row into type_dic_txt table. */
        static void insertTypeText(
            IDbExec * i_dbExec, const map<string, int> & i_langMap, const map<int, int> & i_typeIdMap, TypeDicTxtLangRow & io_row
        );

        /** insert row into type_enum_lst table. */
        static void insertTypeEnum(IDbExec * i_dbExec, int i_typeHid, const TypeEnumLstRow & i_row);

        /** insert row into type_enum_txt table. */
        static void insertTypeEnumText(
            IDbExec * i_dbExec, const map<string, int> & i_langMap, int i_typeHid, TypeEnumTxtLangRow & io_row
        );

        /** insert row into parameter_dic and model_parameter_dic tables. */
        static void insertParamDic(IDbExec * i_dbExec, const map<int, int> & i_typeIdMap, ParamDicRow & io_row);

        /** insert row into parameter_dic_txt table. */
        static void insertParamText(
            IDbExec * i_dbExec, const ParamDicRow & i_paramRow, const map<string, int> & i_langMap, ParamDicTxtLangRow & io_row
        );

        /** insert row into parameter_dims table. */
        static void insertParamDims(
            IDbExec * i_dbExec, const ParamDicRow & i_paramRow, const map<int, int> & i_typeIdMap, const ParamDimsRow & i_row
        );

        /** insert row into parameter_dims_txt table. */
        static void insertParamDimsText(
            IDbExec * i_dbExec, const ParamDicRow & i_paramRow, const map<string, int> & i_langMap, ParamDimsTxtLangRow & io_row
        );

        /** insert row into table_dic and model_table_dic tables. */
        static void insertTableDic(IDbExec * i_dbExec, TableDicRow & io_row);

        /** insert row into table_dic_txt table. */
        static void insertTableText(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableDicTxtLangRow & io_row
        );

        /** insert row into table_dims table. */
        static void insertTableDims(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<int, int> & i_typeIdMap, const TableDimsRow & i_row
        );

        /** insert row into table_dims_txt table. */
        static void insertTableDimsText(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableDimsTxtLangRow & io_row
        );

        /** insert row into table_acc table. */
        static void insertTableAcc(IDbExec * i_dbExec, const TableDicRow & i_tableRow, const TableAccRow & i_row);

        /** insert row into table_acc_txt table. */
        static void insertTableAccText(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableAccTxtLangRow & io_row
        );

        /** insert row into table_expr table. */
        static void insertTableExpr(IDbExec * i_dbExec, const TableDicRow & i_tableRow, const TableExprRow & i_row);

        /** insert row into table_expr_txt table. */
        static void insertTableExprText(
            IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableExprTxtLangRow & io_row
        );

        /** insert row into group_lst table. */
        static void insertGroupLst(IDbExec * i_dbExec, const GroupLstRow & i_row);

        /** insert row into group_txt table. */
        static void insertGroupText(IDbExec * i_dbExec, const map<string, int> & i_langMap, GroupTxtLangRow & io_row);

        /** insert row into group_pc table. */
        static void insertGroupPc(IDbExec * i_dbExec, const GroupPcRow & i_row);

        /** create new workset: insert metadata and delete existing workset parameters, if required */
        static void createWorksetMeta(IDbExec * i_dbExec, const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet);
    };
}

#endif  // MODEL_INSERT_SQL_H
