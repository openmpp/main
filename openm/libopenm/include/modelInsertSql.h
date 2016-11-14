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
#include "modelSqlWriter.h"
using namespace std;

namespace openm
{
    /** class to produce sql statements from metadata rows. */
    class ModelInsertSql
    {
    public:
        /**
        * write sql to insert i_row values to create new model.
        *
        * if new model or workset created then model id selected from id_lst \n
        * it must be done in transaction scope and sql statements can look like: \n
        *
        * @code
        * BEGIN TRANSACTION;
        *
        * UPDATE id_lst SET id_value =
        *   CASE
        *     WHEN
        *       NOT EXISTS (SELECT * FROM model_dic WHERE model_digest = '1234abcd') THEN id_value + 1
        *     ELSE id_value
        *   END
        * WHERE id_key = 'model_id';
        *
        * INSERT INTO model_dic (model_id, ...)
        * SELECT
        *     IL.id_value,
        *     ...
        * FROM id_lst IL WHERE IL.id_key = 'model_id'
        * AND NOT EXISTS
        * (
        *   SELECT * FROM model_dic WHERE model_digest = '1234abcd'
        * );
        *
        * ...
        * COMMIT;
        * @endcode
        */
        template<class TRow> static void insertTopSql(const TRow & i_row, ModelSqlWriter & io_wr);

        /**
        * write sql to insert i_row values into details table.
        *
        * it must be done in transaction scope. \n
        * digest from "master" row to avoid select corresponding id, i.e.: model id, type id, etc.
        */
        template<class TMasterRow, class TRow> static void insertDetailSql(
            const TMasterRow & i_masterRow, const TRow & i_row, ModelSqlWriter & io_wr
        );
    };

    /** write sql to insert into model_dic table. */
    template<> void ModelInsertSql::insertTopSql<ModelDicRow>(const ModelDicRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into model_dic_txt table. */
    template<> void ModelInsertSql::insertDetailSql<ModelDicRow, ModelDicTxtLangRow>(
        const ModelDicRow & i_modelRow, const ModelDicTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into type_dic and model_type_dic tables. */
    template<> void ModelInsertSql::insertDetailSql<ModelDicRow, TypeDicRow>(
        const ModelDicRow & i_modelRow, const TypeDicRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into type_dic_txt table. */
    template<> void ModelInsertSql::insertDetailSql<TypeDicRow, TypeDicTxtLangRow>(
        const TypeDicRow & i_typeRow, const TypeDicTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into type_enum_lst table. */
    template<> void ModelInsertSql::insertDetailSql<TypeDicRow, TypeEnumLstRow>(
        const TypeDicRow & i_typeRow, const TypeEnumLstRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into type_enum_txt table. */
    template<> void ModelInsertSql::insertDetailSql<TypeDicRow, TypeEnumTxtLangRow>(
        const TypeDicRow & i_typeRow, const TypeEnumTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into parameter_dic table. */
    template<> void ModelInsertSql::insertDetailSql<ModelDicRow, ParamDicRow>(
        const ModelDicRow & i_modelRow, const ParamDicRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into parameter_dic_txt table. */
    template<> void ModelInsertSql::insertDetailSql<ParamDicRow, ParamDicTxtLangRow>(
        const ParamDicRow & i_paramRow, const ParamDicTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into parameter_dims table. */
    template<> void ModelInsertSql::insertDetailSql<ParamDicRow, ParamDimsRow>(
        const ParamDicRow & i_paramRow, const ParamDimsRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into parameter_dims_txt table. */
    template<> void ModelInsertSql::insertDetailSql<ParamDicRow, ParamDimsTxtLangRow>(
        const ParamDicRow & i_paramRow, const ParamDimsTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into table_dic table. */
    template<> void ModelInsertSql::insertDetailSql<ModelDicRow, TableDicRow>(
        const ModelDicRow & i_modelRow, const TableDicRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into table_dic_txt table. */
    template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableDicTxtLangRow>(
        const TableDicRow & i_tableRow, const TableDicTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into table_dims table. */
    template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableDimsRow>(
        const TableDicRow & i_tableRow, const TableDimsRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into table_dims_txt table. */
    template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableDimsTxtLangRow>(
        const TableDicRow & i_tableRow, const TableDimsTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into table_acc table. */
    template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableAccRow>(
        const TableDicRow & i_tableRow, const TableAccRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into table_acc_txt table. */
    template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableAccTxtLangRow>(
        const TableDicRow & i_tableRow, const TableAccTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into table_expr table. */
    template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableExprRow>(
        const TableDicRow & i_tableRow, const TableExprRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into table_expr_txt table. */
    template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableExprTxtLangRow>(
        const TableDicRow & i_tableRow, const TableExprTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into group_lst table. */
    template<> void ModelInsertSql::insertDetailSql<ModelDicRow, GroupLstRow>(
        const ModelDicRow & i_modelRow, const GroupLstRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into group_txt table. */
    template<> void ModelInsertSql::insertDetailSql<ModelDicRow, GroupTxtLangRow>(
        const ModelDicRow & i_modelRow, const GroupTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into group_pc table.
    *
    * negative value of i_row.childGroupId or i_row.leafId treated as db-NULL
    */
    template<> void ModelInsertSql::insertDetailSql<ModelDicRow, GroupPcRow>(
        const ModelDicRow & i_modelRow, const GroupPcRow & i_row, ModelSqlWriter & io_wr
        );
}

#endif  // MODEL_INSERT_SQL_H
