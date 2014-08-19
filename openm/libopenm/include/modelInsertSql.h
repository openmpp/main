/**
 * @file
 * OpenM++ data library: class to produce insert sql statements for new model
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_INSERT_SQL_H
#define MODEL_INSERT_SQL_H

#include "libopenm/db/dbMetaRow.h"
#include "modelSqlWriter.h"
using namespace std;

namespace openm
{
    /** class to produce insert sql statements from metadata rows. */
    class ModelInsertSql
    {
    public:
        /**
        * write sql to insert i_row values to create new model.
        *
        * if new model created or workset then model id selected from id_lst \n
        * it must be done in transaction scope and sql statements can look like: \n
        * 
        * @code
        * BEGIN TRANSACTION; 
        * 
        * UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'model_id'; 
        *
        * INSERT INTO model_dic (model_id, ...) 
        * SELECT 
        *     IL.id_value, 
        *     ... 
        * FROM id_lst IL WHERE IL.id_key = 'model_id';
        * 
        * ...
        * COMMIT;
        * @endcode
        */
        template<class TRow> static void insertSql(const TRow & i_row, ModelSqlWriter & io_wr);

        /**
        * write sql to insert i_row values to create new workset.
        *
        * if new workset created then workset id selected from id_lst \n
        * it must be done in transaction scope and sql statements can look like: \n
        *
        * @code
        * BEGIN TRANSACTION;
        *
        * UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_id_set_id';
        *
        * INSERT INTO workset_lst (set_id, ...)
        * SELECT
        *     RSL.id_value,
        *     ...
        * FROM id_lst RSL WHERE RSL.id_key = 'run_id_set_id';
        *
        * ...
        * COMMIT;
        * @endcode
        */
        template<class TRow> static void insertSetSql(
            const ModelDicRow & i_mdRow, const TRow & i_row, ModelSqlWriter & io_wr
            );

        /** make model prefix from model name and timestamp. */
        static const string makeModelPrefix(const string & i_name, const string & i_timestamp);

        /** make unique part of db table name for parameter or output table, ie: 1234_ageSex */
        static const string makeDbNameSuffix(int i_id, const string & i_src);
    };

    /** write sql to insert into model_dic table. */
    template<> void ModelInsertSql::insertSql<ModelDicRow>(const ModelDicRow & i_row, ModelSqlWriter & io_wr);
    
    /** write sql to insert into model_dic_txt table. */
    template<> void ModelInsertSql::insertSql<ModelDicTxtLangRow>(const ModelDicTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into type_dic table. */
    template<> void ModelInsertSql::insertSql<TypeDicRow>(const TypeDicRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into type_dic_txt table. */
    template<> void ModelInsertSql::insertSql<TypeDicTxtLangRow>(const TypeDicTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into type_enum_lst table. */
    template<> void ModelInsertSql::insertSql<TypeEnumLstRow>(const TypeEnumLstRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into type_enum_txt table. */
    template<> void ModelInsertSql::insertSql<TypeEnumTxtLangRow>(const TypeEnumTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into parameter_dic table. */
    template<> void ModelInsertSql::insertSql<ParamDicRow>(const ParamDicRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into parameter_dic_txt table. */
    template<> void ModelInsertSql::insertSql<ParamDicTxtLangRow>(const ParamDicTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into parameter_dims table. */
    template<> void ModelInsertSql::insertSql<ParamDimsRow>(const ParamDimsRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_dic table. */
    template<> void ModelInsertSql::insertSql<TableDicRow>(const TableDicRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_dic_txt table. */
    template<> void ModelInsertSql::insertSql<TableDicTxtLangRow>(const TableDicTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_dims table. */
    template<> void ModelInsertSql::insertSql<TableDimsRow>(const TableDimsRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_dims_txt table. */
    template<> void ModelInsertSql::insertSql<TableDimsTxtLangRow>(const TableDimsTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_acc table. */
    template<> void ModelInsertSql::insertSql<TableAccRow>(const TableAccRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_acc_txt table. */
    template<> void ModelInsertSql::insertSql<TableAccTxtLangRow>(const TableAccTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_unit table. */
    template<> void ModelInsertSql::insertSql<TableUnitRow>(const TableUnitRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_unit_txt table. */
    template<> void ModelInsertSql::insertSql<TableUnitTxtLangRow>(const TableUnitTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into group_lst table. */
    template<> void ModelInsertSql::insertSql<GroupLstRow>(const GroupLstRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into group_txt table. */
    template<> void ModelInsertSql::insertSql<GroupTxtLangRow>(const GroupTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into group_pc table.  
    *
    * negative value of i_row.childGroupId or i_row.leafId treated as db-NULL
    */
    template<> void ModelInsertSql::insertSql<GroupPcRow>(const GroupPcRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into workset_lst table.   
    * 
    * negative value of i_row.runId treated as db-NULL
    */
    template<> void ModelInsertSql::insertSetSql<WorksetLstRow>(
        const ModelDicRow & i_mdRow, const WorksetLstRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into workset_txt table. */
    template<> void ModelInsertSql::insertSetSql<WorksetTxtLangRow>(
        const ModelDicRow & i_mdRow, const WorksetTxtLangRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into workset_parameter table. */
    template<> void ModelInsertSql::insertSetSql<WorksetParamRow>(
        const ModelDicRow & i_mdRow, const WorksetParamRow & i_row, ModelSqlWriter & io_wr
        );

    /** write sql to insert into workset_parameter_txt table. */
    template<> void ModelInsertSql::insertSetSql<WorksetParamTxtLangRow>(
        const ModelDicRow & i_mdRow, const WorksetParamTxtLangRow & i_row, ModelSqlWriter & io_wr
        );
}

/** max allowed size for table type prefix: parameter_prefix, workset_prefix, sub_prefix, value_prefix */
#define OM_DB_TABLE_TYPE_PREFIX_LEN         1

/** max type id for built-int types, ie: int, double, logical */
#define OM_MAX_BUILTIN_TYPE_ID              25

#endif  // MODEL_INSERT_SQL_H
