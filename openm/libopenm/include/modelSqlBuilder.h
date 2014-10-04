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
        void build(MetaModelHolder & io_metaRows);

        /** start sql script to create new working set */
        void beginWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet);

        /** append scalar parameter value to sql script for new working set  creation */
        void addWorksetParameter(const MetaModelHolder & i_metaRows, const string & i_name, const string & i_value);

        /** append parameter values to sql script for new working set  creation */
        void addWorksetParameter(
            const MetaModelHolder & i_metaRows, const string & i_name, const list<string> & i_valueLst
            );

        /** finish sql script to create new working set */
        void endWorkset(void) const;

        /** write sql script to create backward compatibility views */
        void buildCompatibilityViews(const MetaModelHolder & i_metaRows) const;

    private:
        string modelTs;                     // model timestamp string, ie: _201208171604590148_
        string outputDir;                   // output directory to write sql script files
        unique_ptr<ModelSqlWriter> setWr;   // workset sql writer

        /** helper struct to collect info for db table */
        struct DbTblInfo
        {
            /** parameter id or output table id */
            int id;

            /** parameter name or output table name */
            string name;

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

            /** if true then parameter added to workset */
            bool isAdded;
        };

        /** helper struct to collect info for db subsample table and value view */
        struct OutTblInfo : DbTblInfo
        {
            /** db name of accumulator table */
            string accTableName;

            /** db name of accumulator flat view */
            string accFlatViewName;

            /** db name of aggregated value view: table expressions */
            string valueTableName;

            /** accumulator ids */
            vector<int> accIdVec;

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
        void buildCreateModel(const MetaModelHolder & i_metaRows, ModelSqlWriter & io_wr) const;

        /** sort and validate workset metadata for uniqueness and referential integrity */
        void prepareWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet) const;

        /** create new workset and write workset metadata */
        void createWorkset(const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet) const;

        /** create table sql for parameter */
        const void paramCreateTableBody(
            const string i_dbTableName, const string & i_runSetId, const ParamTblInfo & i_tblInfo, ModelSqlWriter & io_wr
            ) const;

        /** create table sql for accumulator table */
        const void accCreateTableBody(const OutTblInfo & i_tblInfo, ModelSqlWriter & io_wr) const;

        /** create table sql for accumulator flatten view */
        const void accFlatCreateViewBody(const OutTblInfo & i_tblInfo, ModelSqlWriter & io_wr) const;

        /** create table sql for value table: aggregated table expressions */
        const void valueCreateTableBody(const OutTblInfo & i_tblInfo, ModelSqlWriter & io_wr) const;

        /** set field values for model_dic table row */
        void setModelDicRow(ModelDicRow & io_mdRow) const;

        /** set field values for workset_lst table row */
        void setWorksetRow(const ModelDicRow & i_mdRow, WorksetLstRow & io_wsRow) const;

        /** collect info for db parameter tables */
        void setParamTableInfo(MetaModelHolder & io_metaRows);

        /** collect info for db subsample tables and value views */
        void setOutTableInfo(MetaModelHolder & io_metaRows);

        /** make model prefix from model name and timestamp. */
        static const string makeModelPrefix(const string & i_name, const string & i_timestamp);

        /** make unique part of db table name for parameter or output table, ie: 1234_ageSex */
        static const string makeDbNameSuffix(int i_id, const string & i_src);
    };
}

#endif  // MODEL_SQL_BUILDER_H
