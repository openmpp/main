/**
 * @file
 * OpenM++ data library: class to produce sql creation script for new model
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_SQL_BUILDER_H
#define MODEL_SQL_BUILDER_H

#include <fstream>
#include <streambuf>
#include "libopenm/db/dbMetaRow.h"
#include "libopenm/db/metaModelHolder.h"
#include "libopenm/db/modelBuilder.h"
#include "modelSqlWriter.h"
#include "modelInsertSql.h"
#include "modelAggregationSql.h"

using namespace std;

namespace openm
{
    /** class to build model creation sql from metadata rows. */
    class ModelSqlBuilder : public IModelBuilder
    {
    public:
        /** create new model builder. */
        ModelSqlBuilder(const string & i_outputDir);

        /** release builder resources. */
        ~ModelSqlBuilder() throw() { }

        /** return timestamp string, ie: _201208171604590148_  result does not changed during object lifetime. */
        const string timeStamp(void) const { return modelTs; }

        /** update metadata and write sql script to create new model from supplied metadata rows */
        void build(MetaModelHolder & i_metaRows);

        /** write sql script to create backward compatibility views (Modgen compatibility) */
        const void buildCompatibilityViews(const MetaModelHolder & i_metaRows) const;

        /** return sql script to insert parameters if template file exists */
        const string buildInsertParameters(const MetaModelHolder & i_metaRows, const string & i_sqlTemplateFilePath) const;

    private:
        string modelTs;         // model timestamp string, ie: _201208171604590148_
        string outputDir;       // output directory to write sql script files

        /** helper struct to collect info for db table */
        struct DbTblInfo
        {
            /** parameter id or output table id */
            int id;

            /** parameter name or output table name */
            string name;

            /** db unique name suffix */
            string suffix;

            /** dimension names */
            vector<string> dimNameVec;

            DbTblInfo(void) : id(-1) { }
        };

        /** helper struct to collect info for db parameter table */
        struct ParamTblInfo : DbTblInfo
        {
            /** db name of parameter table */
            string paramTableName;

            /** db name of parameter workset table */
            string setTableName;

            /** db type of value column to store parameter */
            string valueTypeName;
        };

        /** helper struct to collect info for db subsample table and value view */
        struct OutTblInfo : DbTblInfo
        {
            /** db name of subsample table */
            string subTableName;

            /** db name of aggregated value view: table expressions */
            string valueTableName;

            /** accumulator names */
            vector<string> accNameVec;
        };

        /** vector of db parameter tables info */
        vector<ParamTblInfo> paramInfoVec;

        /** vector of db subsample tables and value views info */
        vector<OutTblInfo> outInfoVec;

        /** sort and validate metadata rows for uniqueness and referential integrity */
        void prepare(MetaModelHolder & io_metaRows) const;

        /** write sql script to create new model from supplied metadata rows */
        const void buildCreateModel(MetaModelHolder & i_metaRows, ModelSqlWriter & io_wr) const;

        /** set field values for model_dic table row */
        void setModelDicRow(ModelDicRow & io_mdRow);

        /** collect info for db parameter tables */
        void setParamTableInfo(const MetaModelHolder & i_metaRows);

        /** collect info for db subsample tables and value views */
        void setOutTableInfo(MetaModelHolder & i_metaRows);

        /** return body of create table sql for parameter */
        const string paramCreateTableBody(const string & i_runSetId, const ParamTblInfo & i_tblInfo) const;

        /** return body of create table sql for subsample table */
        const string subCreateTableBody(const OutTblInfo & i_tblInfo) const;

        /** return body of create table sql for value table: aggregated table expressions */
        const string valueCreateTableBody(const OutTblInfo & i_tblInfo) const;

        /** return body of create view sql for parameter compatibility view */
        const string paramCompatibilityViewBody(const MetaModelHolder & i_metaRows, const ParamTblInfo & i_tblInfo) const;

        /** return body of create view sql for output table compatibility view */
        const string outputCompatibilityViewBody(const MetaModelHolder & i_metaRows, const OutTblInfo & i_tblInfo) const;
    };
}

#endif  // MODEL_SQL_BUILDER_H
