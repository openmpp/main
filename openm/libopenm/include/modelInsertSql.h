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
    /** class to produce insert sql statements for model creation sql from metadata rows. */
    class ModelInsertSql
    {
    public:
        /**
         * write sql to insert i_row values if new model created.
         *
         * if new model created then model id selected from id_lst \n
         * it must be done in transaction scope \n
         * sql statements look like: \n
         * 
         * INSERT INTO model_dic (model_id, ...) \n
         * SELECT \n
         *     IL.id_value, \n
         *     ... \n
         * FROM id_lst IL WHERE IL.id_key = 'model_id'
         *
         * INSERT INTO model_dic_txt (model_id, lang_id, ...) \n
         * SELECT \n
         *     IL.id_value, \n
         *     (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'), \n
         *     ... \n
         * FROM id_lst IL WHERE IL.id_key = 'model_id'
         */
        template<class TRow> static const void insertSql(const TRow & i_row, ModelSqlWriter & io_wr);

        /** make model prefix from model name and timestamp. */
        static const string makeModelPrefix(const string & i_name, const string & i_timestamp);

        /** make unique part of db table name for parameter or output table, ie: 1234_ageSex */
        static const string makeDbNameSuffix(int i_id, const string & i_src);
    };

    /** write sql to insert into model_dic table if new model created. */
    template<> const void ModelInsertSql::insertSql<ModelDicRow>(const ModelDicRow & i_row, ModelSqlWriter & io_wr);
    
    /** write sql to insert into model_dic_txt table if new model created. */
    template<> const void ModelInsertSql::insertSql<ModelDicTxtLangRow>(const ModelDicTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into type_dic table if new model created. */
    template<> const void ModelInsertSql::insertSql<TypeDicRow>(const TypeDicRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into type_dic_txt table if new model created. */
    template<> const void ModelInsertSql::insertSql<TypeDicTxtLangRow>(const TypeDicTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into type_enum_lst table if new model created. */
    template<> const void ModelInsertSql::insertSql<TypeEnumLstRow>(const TypeEnumLstRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into type_enum_txt table if new model created. */
    template<> const void ModelInsertSql::insertSql<TypeEnumTxtLangRow>(const TypeEnumTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into parameter_dic table if new model created. */
    template<> const void ModelInsertSql::insertSql<ParamDicRow>(const ParamDicRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into parameter_dic_txt table if new model created. */
    template<> const void ModelInsertSql::insertSql<ParamDicTxtLangRow>(const ParamDicTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into parameter_dims table if new model created. */
    template<> const void ModelInsertSql::insertSql<ParamDimsRow>(const ParamDimsRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_dic table if new model created. */
    template<> const void ModelInsertSql::insertSql<TableDicRow>(const TableDicRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_dic_txt table if new model created. */
    template<> const void ModelInsertSql::insertSql<TableDicTxtLangRow>(const TableDicTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_dims table if new model created. */
    template<> const void ModelInsertSql::insertSql<TableDimsRow>(const TableDimsRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_dims_txt table if new model created. */
    template<> const void ModelInsertSql::insertSql<TableDimsTxtLangRow>(const TableDimsTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_acc table if new model created. */
    template<> const void ModelInsertSql::insertSql<TableAccRow>(const TableAccRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_acc_txt table if new model created. */
    template<> const void ModelInsertSql::insertSql<TableAccTxtLangRow>(const TableAccTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_unit table if new model created. */
    template<> const void ModelInsertSql::insertSql<TableUnitRow>(const TableUnitRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into table_unit_txt table if new model created. */
    template<> const void ModelInsertSql::insertSql<TableUnitTxtLangRow>(const TableUnitTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into group_lst table if new model created. */
    template<> const void ModelInsertSql::insertSql<GroupLstRow>(const GroupLstRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into group_txt table if new model created. */
    template<> const void ModelInsertSql::insertSql<GroupTxtLangRow>(const GroupTxtLangRow & i_row, ModelSqlWriter & io_wr);

    /** write sql to insert into group_pc table if new model created.  
    *
    * negative value of i_row.childGroupId or i_row.leafId treated as db-NULL
    */
    template<> const void ModelInsertSql::insertSql<GroupPcRow>(const GroupPcRow & i_row, ModelSqlWriter & io_wr);
}


/** max allowed size for table type prefix: parameter_prefix, workset_prefix, sub_prefix, value_prefix */
#define OM_DB_TABLE_TYPE_PREFIX_LEN         1

/** max type id for built-int types, ie: int, double, logical */
#define OM_MAX_BUILTIN_TYPE_ID              25

#endif  // MODEL_INSERT_SQL_H
