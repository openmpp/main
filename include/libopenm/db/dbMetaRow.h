/**
 * @file
 * OpenM++ data library: db rows of model metadata tables
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_META_ROW_H
#define DB_META_ROW_H

#include <memory>
using namespace std;

#include "libopenm/omCommon.h"
#include "dbCommon.h"

namespace openm
{
    /** base class for model metadata db-rows */
    template<class TRow> struct IMetaRow : public IRowBase
    {
        /** less comparator by row primary key. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
        {
            return TRow::isKeyLess(*dynamic_cast<TRow *>(i_left.get()), *dynamic_cast<TRow *>(i_right.get()));
        }

        /** equal comparator by row primary key. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
        {
            return TRow::isKeyEqual(*dynamic_cast<TRow *>(i_left.get()), *dynamic_cast<TRow *>(i_right.get()));
        }
    };

    /** lang_lst table row. */
    struct LangLstRow : public IMetaRow<LangLstRow>
    {
        /** lang_id   INT          NOT NULL */
        int langId;

        /** lang_code VARCHAR(32)  NOT NULL */
        string code;

        /** lang_name VARCHAR(255) NOT NULL */
        string name;
        
        /** create row with supplied primary key field values. */
        LangLstRow(int i_langId) : 
            langId(i_langId), 
            code(""), 
            name("")
        { }

        /** create row with default empty field values. */
        LangLstRow(void) : LangLstRow(0) { }

        ~LangLstRow(void) throw() { }

        /** less comparator by primary key: language id. */
        static bool isKeyLess(const LangLstRow & i_left, const LangLstRow & i_right);

        /** equal comparator by primary key: language id. */
        static bool isKeyEqual(const LangLstRow & i_left, const LangLstRow & i_right);
    };

    /** lang_word table row. */
    struct LangWordRow : public IMetaRow<LangWordRow>
    {
        /** lang_id    INT          NOT NULL */
        int langId;

        /** word_code  VARCHAR(255) NOT NULL */
        string code;

        /** word_value VARCHAR(255) NOT NULL */
        string value;
        
        /** create row with supplied primary key field values. */
        LangWordRow(int i_langId, const string & i_code) : 
            langId(i_langId), 
            code(i_code), 
            value("")
        { }

        /** create row with default empty field values. */
        LangWordRow(void) : LangWordRow(0, "") { }

        ~LangWordRow(void) throw() { }

        /** less comparator by primary key: language id and word code. */
        static bool isKeyLess(const LangWordRow & i_left, const LangWordRow & i_right);

        /** equal comparator by primary key: language id and word code. */
        static bool isKeyEqual(const LangWordRow & i_left, const LangWordRow & i_right);
    };

    /** model_dic table row. */
    struct ModelDicRow : public IMetaRow<ModelDicRow>
    {
        /** model_id       INT          NOT NULL */
        int modelId;

        /** model_name     VARCHAR(255) NOT NULL */
        string name;

        /** model_type     INT          NOT NULL */
        int type;

        /** model_ver      VARCHAR(255) NOT NULL */
        string version;

        /** model_ts       VARCHAR(32)  NOT NULL */
        string timestamp;

        /** model_prefix   VARCHAR(32)  NOT NULL */
        string modelPrefix;

        /** parameter_prefix VARCHAR(4) NOT NULL, -- parameter tables prefix: p */
        string paramPrefix;

        /** workset_prefix   VARCHAR(4) NOT NULL, -- workset tables prefix: w */
        string setPrefix;

        /** acc_prefix       VARCHAR(4) NOT NULL, -- accumulator table prefix: a */
        string accPrefix;

        /** value_prefix     VARCHAR(4) NOT NULL, -- value tables prefix: v */
        string valuePrefix;

        /** create row with supplied primary key field values. */
        ModelDicRow(int i_modelId) : 
            modelId(i_modelId), 
            name(""), 
            type(0),
            version(""),
            timestamp(""),
            modelPrefix(""),
            paramPrefix(""),
            setPrefix(""),
            accPrefix(""),
            valuePrefix("")
        { }

        /** create row with default empty field values. */
        ModelDicRow(void) : ModelDicRow(0) { }

        ~ModelDicRow(void) throw() { }

        /** less comparator by primary key: model id. */
        static bool isKeyLess(const ModelDicRow & i_left, const ModelDicRow & i_right);

        /** equal comparator by primary key: model id. */
        static bool isKeyEqual(const ModelDicRow & i_left, const ModelDicRow & i_right);
    };

    /** model_dic_txt table row. */
    struct ModelDicTxtRow : public IMetaRow<ModelDicTxtRow>
    {
        /** model_id INT NOT NULL */
        int modelId;

        /** lang_id  INT NOT NULL */
        int langId;

        /** descr VARCHAR(255) NOT NULL */
        string descr;

        /** note     VARCHAR(32000) */
        string note;

        /** create row with supplied primary key field values. */
        ModelDicTxtRow(int i_modelId, int i_langId) : 
            modelId(i_modelId), 
            langId(i_langId),
            descr(""), 
            note("")
        { }

        /** create row with default empty field values. */
        ModelDicTxtRow(void) : ModelDicTxtRow(0, 0) { }

        ~ModelDicTxtRow(void) throw() { }

        /** less comparator by primary key: model id and language id. */
        static bool isKeyLess(const ModelDicTxtRow & i_left, const ModelDicTxtRow & i_right);

        /** equal comparator by primary key: model id and language id. */
        static bool isKeyEqual(const ModelDicTxtRow & i_left, const ModelDicTxtRow & i_right);
    };

    /** model_dic_txt table row and language name. */
    struct ModelDicTxtLangRow : public ModelDicTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id and language name. */
        static bool uniqueLangKeyLess(const ModelDicTxtLangRow & i_left, const ModelDicTxtLangRow & i_right);

        /** equal comparator by unique key: model id and language name. */
        static bool uniqueLangKeyEqual(const ModelDicTxtLangRow & i_left, const ModelDicTxtLangRow & i_right);
    };

    /** run_lst table row. */
    struct RunLstRow : public IMetaRow<RunLstRow>
    {
        /** run_id        INT          NOT NULL */
        int runId;

        /** model_id      INT          NOT NULL */
        int modelId;

        /** run_name      VARCHAR(255) NOT NULL */
        string name;

        /** sub_count     INT          NOT NULL */
        int subCount;

        /** sub_started   INT          NOT NULL */
        int subStarted;

        /** sub_completed INT          NOT NULL */
        int subCompleted;

        /** sub_restart   INT          NOT NULL */
        int subRestart;

        /** create_dt     VARCHAR(32) NOT NULL */
        string createDateTime;

        /** run status: i=init p=progress s=success e=error(failed) */
        string status;

        /** update_dt     VARCHAR(32)  NOT NULL */
        string updateDateTime;

        /** create row with supplied primary key field values. */
        RunLstRow(int i_runId) : 
            runId(i_runId),
            modelId(0),
            name(""),
            subCount(0),
            subStarted(0),
            subCompleted(0),
            subRestart(0),
            createDateTime(""),
            status(""),
            updateDateTime("")
        { }

        /** create row with default empty field values. */
        RunLstRow(void) : RunLstRow(0) { }

        ~RunLstRow(void) throw() { }

        /** less comparator by primary key: run id. */
        static bool isKeyLess(const RunLstRow & i_left, const RunLstRow & i_right);

        /** equal comparator by primary key: run id. */
        static bool isKeyEqual(const RunLstRow & i_left, const RunLstRow & i_right);

        /** find row by primary key: run id. */
        static vector<RunLstRow>::const_iterator byKey(int i_runId, const vector<RunLstRow> & i_rowVec);
    };

    /** run_txt table row. */
    struct RunTxtRow : public IMetaRow<RunTxtRow>
    {
        /** run_id   INT          NOT NULL */
        int runId;

        /** model_id INT          NOT NULL */
        int modelId;

        /** lang_id  INT          NOT NULL */
        int langId;

        /** descr    VARCHAR(255) NOT NULL */
        string descr;

        /** note     VARCHAR(32000)          */
        string note;

        /** create row with supplied primary key field values. */
        RunTxtRow(int i_runId, int i_langId) : 
            runId(i_runId),
            modelId(0),
            langId(i_langId),
            descr(""),
            note("")
        { }

        /** create row with default empty field values. */
        RunTxtRow(void) : RunTxtRow(0, 0) { }

        ~RunTxtRow(void) throw() { }

        /** less comparator by primary key: run id and language id. */
        static bool isKeyLess(const RunTxtRow & i_left, const RunTxtRow & i_right);

        /** equal comparator by primary key: run id and language id. */
        static bool isKeyEqual(const RunTxtRow & i_left, const RunTxtRow & i_right);

        /** find row by primary key: run id and language id. */
        static vector<RunTxtRow>::const_iterator byKey(int i_runId, int i_langId, const vector<RunTxtRow> & i_rowVec);
    };

    /** type_dic table row. */
    struct TypeDicRow : public IMetaRow<TypeDicRow>
    {
        /** model_id      INT          NOT NULL */
        int modelId;

        /** mod_type_id   INT          NOT NULL */
        int typeId;

        /** mod_type_name VARCHAR(255) NOT NULL, -- type name: int, double, etc. */
        string name;

        /** dic_id INT NOT NULL, -- dictionary id: 0=simple 1=logical 2=classification 3=range 4=partition 5=link */
        int dicId;

        /** total_enum_id INT NOT NULL, -- if total enabled this is enum_value of total item =max+1 */
        int totalEnumId;

        /** create row with supplied primary key field values. */
        TypeDicRow(int i_modelId, int i_typeId) : 
            modelId(i_modelId), 
            typeId(i_typeId), 
            name(""),
            dicId(0), 
            totalEnumId(0)
        { }

        /** create row with default empty field values. */
        TypeDicRow(void) : TypeDicRow(0, 0) { }

        ~TypeDicRow(void) throw() { }

        /** less comparator by primary key: model id, type id. */
        static bool isKeyLess(const TypeDicRow & i_left, const TypeDicRow & i_right);

        /** equal comparator by primary key: model id, type id. */
        static bool isKeyEqual(const TypeDicRow & i_left, const TypeDicRow & i_right);
    };

    /** type_dic_txt table row. */
    struct TypeDicTxtRow : public IMetaRow<TypeDicTxtRow>
    {
        /** model_id    INT NOT NULL */
        int modelId;
        
        /** mod_type_id INT NOT NULL */
        int typeId;
        
        /** lang_id     INT NOT NULL */
        int langId;
        
        /** descr       VARCHAR(255) NOT NULL */
        string descr;
        
        /** note        VARCHAR(32000) */
        string note;
        
        /** create row with supplied primary key field values. */
        TypeDicTxtRow(int i_modelId, int i_typeId, int i_langId) : 
            modelId(i_modelId), 
            typeId(i_typeId), 
            langId(i_langId),
            descr(""), 
            note("")
        { }

        /** create row with default empty field values. */
        TypeDicTxtRow(void) : TypeDicTxtRow(0, 0, 0) { }

        ~TypeDicTxtRow(void) throw() { }

        /** less comparator by primary key: model id, type id, language id. */
        static bool isKeyLess(const TypeDicTxtRow & i_left, const TypeDicTxtRow & i_right);

        /** equal comparator by primary key: model id, type id, language id. */
        static bool isKeyEqual(const TypeDicTxtRow & i_left, const TypeDicTxtRow & i_right);
    };

    /** type_dic_txt table row and language name. */
    struct TypeDicTxtLangRow : public TypeDicTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id, type id, language name. */
        static bool uniqueLangKeyLess(const TypeDicTxtLangRow & i_left, const TypeDicTxtLangRow & i_right);

        /** equal comparator by unique key: model id, type id, language name. */
        static bool uniqueLangKeyEqual(const TypeDicTxtLangRow & i_left, const TypeDicTxtLangRow & i_right);
    };

    /** type_enum_lst table row. */
    struct TypeEnumLstRow : public IMetaRow<TypeEnumLstRow>
    {
        /** model_id    INT          NOT NULL */
        int modelId;

        /** mod_type_id INT          NOT NULL */
        int typeId;

        /** enum_id     INT          NOT NULL */
        int enumId;

        /** enum_name   VARCHAR(255) NOT NULL */
        string name;

        /** create row with supplied primary key field values. */
        TypeEnumLstRow(int i_modelId, int i_typeId, int i_enumId) : 
            modelId(i_modelId), 
            typeId(i_typeId), 
            enumId(i_enumId), 
            name("")
        { }

        /** create row with default empty field values. */
        TypeEnumLstRow(void) : TypeEnumLstRow(0, 0, 0) { }

        ~TypeEnumLstRow(void) throw() { }

        /** less comparator by primary key: model id, type id, enum id. */
        static bool isKeyLess(const TypeEnumLstRow & i_left, const TypeEnumLstRow & i_right);

        /** equal comparator by primary key: model id, type id, enum id. */
        static bool isKeyEqual(const TypeEnumLstRow & i_left, const TypeEnumLstRow & i_right);
    };

    /** type_enum_txt table row. */
    struct TypeEnumTxtRow : public IMetaRow<TypeEnumTxtRow>
    {
        /** model_id    INT NOT NULL */
        int modelId;
        
        /** mod_type_id INT NOT NULL */
        int typeId;
        
        /** enum_id     INT NOT NULL */
        int enumId;
        
        /** lang_id     INT NOT NULL */
        int langId;
        
        /** descr       VARCHAR(255) NOT NULL */
        string descr;
        
        /** note        VARCHAR(32000) */
        string note;
        
        /** create row with supplied primary key field values. */
        TypeEnumTxtRow(int i_modelId, int i_typeId, int i_enumId, int i_langId) : 
            modelId(i_modelId), 
            typeId(i_typeId), 
            enumId(i_enumId),
            langId(i_langId),
            descr(""), 
            note("")
        { }

        /** create row with default empty field values. */
        TypeEnumTxtRow(void) : TypeEnumTxtRow(0, 0, 0, 0) { }

        ~TypeEnumTxtRow(void) throw() { }

        /** less comparator by primary key: model id, type id, enum id, language id. */
        static bool isKeyLess(const TypeEnumTxtRow & i_left, const TypeEnumTxtRow & i_right);

        /** equal comparator by primary key: model id, type id, enum id, language id. */
        static bool isKeyEqual(const TypeEnumTxtRow & i_left, const TypeEnumTxtRow & i_right);
    };

    /** type_enum_txt table row and language name. */
    struct TypeEnumTxtLangRow : public TypeEnumTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id, type id, enum id, language name. */
        static bool uniqueLangKeyLess(const TypeEnumTxtLangRow & i_left, const TypeEnumTxtLangRow & i_right);

        /** equal comparator by unique key: model id, type id, enum id, language name. */
        static bool uniqueLangKeyEqual(const TypeEnumTxtLangRow & i_left, const TypeEnumTxtLangRow & i_right);
    };

    /** parameter_dic table row. */
    struct ParamDicRow : public IMetaRow<ParamDicRow>
    {
        /** model_id       INT          NOT NULL */
        int modelId;

        /** parameter_id   INT          NOT NULL */
        int paramId;

        /** db_name_suffix VARCHAR(32)  NOT NULL */
        string dbNameSuffix;

        /** parameter_name VARCHAR(255) NOT NULL */
        string paramName;

        /** parameter_rank INT          NOT NULL */
        int rank;

        /** mod_type_id    INT          NOT NULL */
        int typeId;

        /** is_hidden      SMALLINT     NOT NULL */
        bool isHidden;

        /** is_generated   SMALLINT     NOT NULL */
        bool isGenerated;

        /** num_cumulated  INT          NOT NULL */
        int numCumulated;

        /** create row with supplied primary key field values. */
        ParamDicRow(int i_modelId, int i_paramId) : 
            modelId(i_modelId), 
            paramId(i_paramId), 
            dbNameSuffix(""),
            paramName(""),
            rank(0),
            typeId(0),
            isHidden(false),
            isGenerated(false),
            numCumulated(0)
        { }

        /** create row with default empty field values. */
        ParamDicRow(void) : ParamDicRow(0, 0) { }

        ~ParamDicRow(void) throw() { }

        /** less comparator by primary key: model id, parameter id. */
        static bool isKeyLess(const ParamDicRow & i_left, const ParamDicRow & i_right);

        /** equal comparator by primary key: model id, parameter id. */
        static bool isKeyEqual(const ParamDicRow & i_left, const ParamDicRow & i_right);
    };

    /** parameter_dic_txt table row. */
    struct ParamDicTxtRow : public IMetaRow<ParamDicTxtRow>
    {
        /** model_id     INT NOT NULL */
        int modelId;
        
        /** parameter_id INT NOT NULL */
        int paramId;
        
        /** lang_id      INT NOT NULL */
        int langId;
        
        /** descr        VARCHAR(255) NOT NULL */
        string descr;
        
        /** note         VARCHAR(32000) */
        string note;
        
        /** create row with supplied primary key field values. */
        ParamDicTxtRow(int i_modelId, int i_paramId, int i_langId) : 
            modelId(i_modelId), 
            paramId(i_paramId), 
            langId(i_langId),
            descr(""), 
            note("")
        { }

        /** create row with default empty field values. */
        ParamDicTxtRow(void) : ParamDicTxtRow(0, 0, 0) { }

        ~ParamDicTxtRow(void) throw() { }

        /** less comparator by primary key: model id, parameter id, language id. */
        static bool isKeyLess(const ParamDicTxtRow & i_left, const ParamDicTxtRow & i_right);

        /** equal comparator by primary key: model id, parameter id, language id. */
        static bool isKeyEqual(const ParamDicTxtRow & i_left, const ParamDicTxtRow & i_right);
    };

    /** parameter_dic_txt table row and language name. */
    struct ParamDicTxtLangRow : public ParamDicTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id, parameter id, language name. */
        static bool uniqueLangKeyLess(const ParamDicTxtLangRow & i_left, const ParamDicTxtLangRow & i_right);

        /** equal comparator by unique key: model id, parameter id, language name. */
        static bool uniqueLangKeyEqual(const ParamDicTxtLangRow & i_left, const ParamDicTxtLangRow & i_right);
    };

    /** parameter_run_txt table row. */
    struct ParamRunTxtRow : public IMetaRow<ParamRunTxtRow>
    {
        /** run_id       INT NOT NULL */
        int runId;
        
        /** model_id     INT NOT NULL */
        int modelId;
        
        /** parameter_id INT NOT NULL */
        int paramId;
        
        /** lang_id      INT NOT NULL */
        int langId;
        
        /** note         VARCHAR(32000) */
        string note;
        
        /** create row with supplied primary key field values. */
        ParamRunTxtRow(int i_runId, int i_paramId, int i_langId) : 
            runId(i_runId), 
            modelId(0), 
            paramId(i_paramId), 
            langId(i_langId),
            note("")
        { }

        /** create row with default empty field values. */
        ParamRunTxtRow(void) : ParamRunTxtRow(0, 0, 0) { }

        ~ParamRunTxtRow(void) throw() { }

        /** less comparator by primary key: run id, parameter id, language id. */
        static bool isKeyLess(const ParamRunTxtRow & i_left, const ParamRunTxtRow & i_right);

        /** equal comparator by primary key: run id, parameter id, language id. */
        static bool isKeyEqual(const ParamRunTxtRow & i_left, const ParamRunTxtRow & i_right);
    };

    /** parameter_run_txt table row and language name. */
    struct ParamRunTxtLangRow : public ParamRunTxtRow
    {
        /** language name */
        string langName;
    };

    /** parameter_dims table row. */
    struct ParamDimsRow : public IMetaRow<ParamDimsRow>
    {
        /** model_id     INT NOT NULL */
        int modelId;
        
        /** parameter_id INT NOT NULL */
        int paramId;
        
        /** dim_id      INT NOT NULL */
        int dimId;

        /** dim_name     VARCHAR(8) NOT NULL */
        string name;
        
        /** mod_type_id  INT NOT NULL */
        int typeId;
        
        /** create row with supplied primary key field values. */
        ParamDimsRow(int i_modelId, int i_paramId, int i_dimId) : 
            modelId(i_modelId), 
            paramId(i_paramId), 
            dimId(i_dimId),
            name(""),
            typeId(0)
        { }

        /** create row with default empty field values. */
        ParamDimsRow(void) : ParamDimsRow(0, 0, 0) { }

        ~ParamDimsRow(void) throw() { }

        /** less comparator by primary key: model id, parameter id, dimension id. */
        static bool isKeyLess(const ParamDimsRow & i_left, const ParamDimsRow & i_right);

        /** equal comparator by primary key: model id, parameter id, dimension id. */
        static bool isKeyEqual(const ParamDimsRow & i_left, const ParamDimsRow & i_right);
    };


    /** parameter_dims_txt table row. */
    struct ParamDimsTxtRow : public IMetaRow<ParamDimsTxtRow>
    {
        /** model_id INT          NOT NULL */
        int modelId;

        /** parameter_id INT      NOT NULL */
        int paramId;

        /** dim_id   INT          NOT NULL */
        int dimId;

        /** lang_id  INT          NOT NULL */
        int langId;

        /** descr    VARCHAR(255) NOT NULL */
        string descr;

        /** note     VARCHAR(32000) */
        string note;

        /** create row with supplied primary key field values. */
        ParamDimsTxtRow(int i_modelId, int i_paramId, int i_dimId, int i_langId) :
            modelId(i_modelId),
            paramId(i_paramId),
            dimId(i_dimId),
            langId(i_langId),
            descr(""),
            note("")
        { }

        /** create row with default empty field values. */
        ParamDimsTxtRow(void) : ParamDimsTxtRow(0, 0, 0, 0) { }

        ~ParamDimsTxtRow(void) throw() { }

        /** less comparator by primary key: model id, parameter id, dimension id, language id. */
        static bool isKeyLess(const ParamDimsTxtRow & i_left, const ParamDimsTxtRow & i_right);

        /** equal comparator by primary key: model id, parameter id, dimension id, language id. */
        static bool isKeyEqual(const ParamDimsTxtRow & i_left, const ParamDimsTxtRow & i_right);
    };

    /** parameter_dims_txt table row and language name. */
    struct ParamDimsTxtLangRow : public ParamDimsTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id, parameter id, dimension name, language name. */
        static bool uniqueLangKeyLess(const ParamDimsTxtLangRow & i_left, const ParamDimsTxtLangRow & i_right);

        /** equal comparator by unique key: model id, parameter id, dimension name, language name. */
        static bool uniqueLangKeyEqual(const ParamDimsTxtLangRow & i_left, const ParamDimsTxtLangRow & i_right);
    };

    /** table_dic table row. */
    struct TableDicRow : public IMetaRow<TableDicRow>
    {
        /** model_id       INT NOT NULL          */
        int modelId;
        
        /** table_id       INT NOT NULL          */
        int tableId;
        
        /** db_name_suffix VARCHAR(64) NOT NULL  */
        string dbNameSuffix;
        
        /** table_name     VARCHAR(255) NOT NULL */
        string tableName;
        
        /** is_user        SMALLINT NOT NULL     */
        bool isUser;
        
        /** table_rank     INT NOT NULL          */
        int rank;
        
        /** is_sparse      SMALLINT NOT NULL     */
        bool isSparse;
        
        /** is_hidden      SMALLINT NOT NULL     */
        bool isHidden;

        /** expr_dim_pos   INT      NOT NULL     */
        int exprPos;

        /** create row with supplied primary key field values. */
        TableDicRow(int i_modelId, int i_tableId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            dbNameSuffix(""),
            tableName(""),
            isUser(false),
            rank(1),
            isSparse(false),
            isHidden(false),
            exprPos(-1)
        { }

        /** create row with default empty field values. */
        TableDicRow(void) : TableDicRow(0, 0) { }

        ~TableDicRow(void) throw() { }

        /** less comparator by primary key: model id, table id. */
        static bool isKeyLess(const TableDicRow & i_left, const TableDicRow & i_right);

        /** equal comparator by primary key: model id, table id. */
        static bool isKeyEqual(const TableDicRow & i_left, const TableDicRow & i_right);
    };

    /** table_dic_txt table row. */
    struct TableDicTxtRow : public IMetaRow<TableDicTxtRow>
    {
        /** model_id   INT NOT NULL */
        int modelId;
        
        /** table_id   INT NOT NULL */
        int tableId;
        
        /** lang_id    INT NOT NULL */
        int langId;
        
        /** descr      VARCHAR(255) NOT NULL */
        string descr;
        
        /** note       VARCHAR(32000)        */
        string note;
        
        /** expr_descr VARCHAR(255) NOT NULL */
        string exprDescr;
        
        /** expr_note  VARCHAR(32000) */
        string exprNote;
        
        /** create row with supplied primary key field values. */
        TableDicTxtRow(int i_modelId, int i_tableId, int i_langId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            langId(i_langId),
            descr(""), 
            note(""),
            exprDescr(""),
            exprNote("")
        { }

        /** create row with default empty field values. */
        TableDicTxtRow(void) : TableDicTxtRow(0, 0, 0) { }

        ~TableDicTxtRow(void) throw() { }

        /** less comparator by primary key: model id, table id, language id. */
        static bool isKeyLess(const TableDicTxtRow & i_left, const TableDicTxtRow & i_right);

        /** equal comparator by primary key: model id, table id, language id. */
        static bool isKeyEqual(const TableDicTxtRow & i_left, const TableDicTxtRow & i_right);
    };

    /** table_dic_txt table row and language name. */
    struct TableDicTxtLangRow : public TableDicTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id, table id, language name. */
        static bool uniqueLangKeyLess(const TableDicTxtLangRow & i_left, const TableDicTxtLangRow & i_right);

        /** equal comparator by unique key: model id, table id, language name. */
        static bool uniqueLangKeyEqual(const TableDicTxtLangRow & i_left, const TableDicTxtLangRow & i_right);
    };

    /** table_dims table row. */
    struct TableDimsRow : public IMetaRow<TableDimsRow>
    {
        /** model_id    INT NOT NULL */
        int modelId;
        
        /** table_id    INT NOT NULL */
        int tableId;
        
        /** dim_id      INT NOT NULL */
        int dimId;

        /** dim_name    VARCHAR(8) NOT NULL */
        string name;
        
        /** mod_type_id INT NOT NULL */
        int typeId;
        
        /** is_total    SMALLINT NOT NULL */
        bool isTotal;
        
        /** dim_size    INT NOT NULL */
        int dimSize;
        
        /** create row with supplied primary key field values. */
        TableDimsRow(int i_modelId, int i_tableId, int i_dimId) :
            modelId(i_modelId), 
            tableId(i_tableId), 
            dimId(i_dimId),
            name(""),
            typeId(0),
            isTotal(false),
            dimSize(1)
        { }

        /** create row with default empty field values. */
        TableDimsRow(void) : TableDimsRow(0, 0, 0) { }

        ~TableDimsRow(void) throw() { }

        /** less comparator by primary key: model id, table id, dimension id. */
        static bool isKeyLess(const TableDimsRow & i_left, const TableDimsRow & i_right);
        
        /** equal comparator by primary key: model id, table id, dimension id. */
        static bool isKeyEqual(const TableDimsRow & i_left, const TableDimsRow & i_right);
    };

    /** table_dims_txt table row. */
    struct TableDimsTxtRow : public IMetaRow<TableDimsTxtRow>
    {
        /** model_id INT          NOT NULL */
        int modelId;
        
        /** table_id INT          NOT NULL */
        int tableId;
        
        /** dim_id   INT          NOT NULL */
        int dimId;

        /** lang_id  INT          NOT NULL */
        int langId;
        
        /** descr    VARCHAR(255) NOT NULL */
        string descr;
        
        /** note     VARCHAR(32000) */
        string note;
        
        /** create row with supplied primary key field values. */
        TableDimsTxtRow(int i_modelId, int i_tableId, int i_dimId, int i_langId) :
            modelId(i_modelId), 
            tableId(i_tableId), 
            dimId(i_dimId),
            langId(i_langId),
            descr(""), 
            note("")
        { }

        /** create row with default empty field values. */
        TableDimsTxtRow(void) : TableDimsTxtRow(0, 0, 0, 0) { }

        ~TableDimsTxtRow(void) throw() { }

        /** less comparator by primary key: model id, table id, dimension id, language id. */
        static bool isKeyLess(const TableDimsTxtRow & i_left, const TableDimsTxtRow & i_right);

        /** equal comparator by primary key: model id, table id, dimension id, language id. */
        static bool isKeyEqual(const TableDimsTxtRow & i_left, const TableDimsTxtRow & i_right);
    };

    /** table_dims_txt table row and language name. */
    struct TableDimsTxtLangRow : public TableDimsTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id, table id, dimension name, language name. */
        static bool uniqueLangKeyLess(const TableDimsTxtLangRow & i_left, const TableDimsTxtLangRow & i_right);

        /** equal comparator by unique key: model id, table id, dimension name, language name. */
        static bool uniqueLangKeyEqual(const TableDimsTxtLangRow & i_left, const TableDimsTxtLangRow & i_right);
    };

    /** table_acc table row. */
    struct TableAccRow : public IMetaRow<TableAccRow>
    {
        /** model_id INT          NOT NULL */
        int modelId;
        
        /** table_id INT          NOT NULL */
        int tableId;
        
        /** acc_id INT            NOT NULL */
        int accId;
        
        /** acc_name VARCHAR(8)   NOT NULL */
        string name;
        
        /** acc_expr VARCHAR(255) NOT NULL */
        string expr;
        
        /** create row with supplied primary key field values. */
        TableAccRow(int i_modelId, int i_tableId, int i_accid) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            accId(i_accid), 
            name(""),
            expr("")
        { }

        /** create row with default empty field values. */
        TableAccRow(void) : TableAccRow(0, 0, 0) { }

        ~TableAccRow(void) throw() { }

        /** less comparator by primary key: model id, table id, accumulator id. */
        static bool isKeyLess(const TableAccRow & i_left, const TableAccRow & i_right);

        /** equal comparator by primary key: model id, table id, accumulator id. */
        static bool isKeyEqual(const TableAccRow & i_left, const TableAccRow & i_right);
    };

    /** table_acc_txt table row. */
    struct TableAccTxtRow : public IMetaRow<TableAccTxtRow>
    {
        /** model_id INT          NOT NULL */
        int modelId;
        
        /** table_id INT          NOT NULL */
        int tableId;
        
        /** acc_id   INT          NOT NULL */
        int accId;
        
        /** lang_id  INT          NOT NULL */
        int langId;
        
        /** descr    VARCHAR(255) NOT NULL */
        string descr;
        
        /** note     VARCHAR(32000) */
        string note;

        /** create row with supplied primary key field values. */
        TableAccTxtRow(int i_modelId, int i_tableId, int i_accId, int i_langId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            accId(i_accId),
            langId(i_langId),
            descr(""), 
            note("")
        { }

        /** create row with default empty field values. */
        TableAccTxtRow(void) : TableAccTxtRow(0, 0, 0, 0) { }

        ~TableAccTxtRow(void) throw() { }

        /** less comparator by primary key: model id, table id, accumulator id, language id. */
        static bool isKeyLess(const TableAccTxtRow & i_left, const TableAccTxtRow & i_right);

        /** equal comparator by primary key: model id, table id, accumulator id, language id. */
        static bool isKeyEqual(const TableAccTxtRow & i_left, const TableAccTxtRow & i_right);
    };

    /** table_acc_txt table row and language name. */
    struct TableAccTxtLangRow : public TableAccTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id, table id, accumulator id, language name. */
        static bool uniqueLangKeyLess(const TableAccTxtLangRow & i_left, const TableAccTxtLangRow & i_right);

        /** equal comparator by unique key: model id, table id, accumulator id, language name. */
        static bool uniqueLangKeyEqual(const TableAccTxtLangRow & i_left, const TableAccTxtLangRow & i_right);
    };

    /** table_expr table row. */
    struct TableExprRow : public IMetaRow<TableExprRow>
    {
        /** model_id      INT           NOT NULL */
        int modelId;
        
        /** table_id      INT           NOT NULL */
        int tableId;
        
        /** expr_id       INT           NOT NULL */
        int exprId;
        
        /** expr_name     VARCHAR(8)    NOT NULL */
        string name;
        
        /** expr_decimals INT           NOT NULL */
        int decimals;
        
        /** expr_src      VARCHAR(255)  NOT NULL */
        string src;

        /** expr_sql      VARCHAR(2048) NOT NULL */
        string expr;

        /** create row with supplied primary key field values. */
        TableExprRow(int i_modelId, int i_tableId, int i_exprId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            exprId(i_exprId), 
            name(""),
            decimals(0),
            src(""),
            expr("")
        { }

        /** create row with default empty field values. */
        TableExprRow(void) : TableExprRow(0, 0, 0) { }

        ~TableExprRow(void) throw() { }

        /** less comparator by primary key: model id, table id, expr id. */
        static bool isKeyLess(const TableExprRow & i_left, const TableExprRow & i_right);

        /** equal comparator by primary key: model id, table id, expr id. */
        static bool isKeyEqual(const TableExprRow & i_left, const TableExprRow & i_right);
    };

    /** table_expr_txt table row. */
    struct TableExprTxtRow : public IMetaRow<TableExprTxtRow>
    {
        /** model_id INT          NOT NULL */
        int modelId;
        
        /** TableID  INT          NOT NULL */
        int tableId;
        
        /** expr_id  INT          NOT NULL */
        int exprId;
        
        /** lang_id  INT          NOT NULL */
        int langId;
        
        /** descr    VARCHAR(255) NOT NULL */
        string descr;
        
        /** note     VARCHAR(32000) */
        string note;

        /** create row with supplied primary key field values. */
        TableExprTxtRow(int i_modelId, int i_tableId, int i_exprId, int i_langId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            exprId(i_exprId),
            langId(i_langId),
            descr(""), 
            note("")
        { }

        /** create row with default empty field values. */
        TableExprTxtRow(void) : TableExprTxtRow(0, 0, 0, 0) { }

        ~TableExprTxtRow(void) throw() { }

        /** less comparator by primary key: model id, table id, expr id, language id. */
        static bool isKeyLess(const TableExprTxtRow & i_left, const TableExprTxtRow & i_right);

        /** equal comparator by primary key: model id, table id, expr id, language id. */
        static bool isKeyEqual(const TableExprTxtRow & i_left, const TableExprTxtRow & i_right);
    };

    /** table_expr_txt table row and language name. */
    struct TableExprTxtLangRow : public TableExprTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id, table id, expr id, language name. */
        static bool uniqueLangKeyLess(const TableExprTxtLangRow & i_left, const TableExprTxtLangRow & i_right);

        /** equal comparator by unique key: model id, table id, expr id, language name. */
        static bool uniqueLangKeyEqual(const TableExprTxtLangRow & i_left, const TableExprTxtLangRow & i_right);
    };

    /** group_lst table row. */
    struct GroupLstRow : public IMetaRow<GroupLstRow>
    {
        /** model_id     INT          NOT NULL */
        int modelId;

        /** group_id     INT          NOT NULL */
        int groupId;

        /** is_parameter SMALLINT     NOT NULL */
        bool isParam;

        /** group_name   VARCHAR(255) NOT NULL */
        string name;

        /** is_hidden    SMALLINT     NOT NULL */
        bool isHidden;

        /** is_generated SMALLINT     NOT NULL */
        bool isGenerated;

        /** create row with supplied primary key field values. */
        GroupLstRow(int i_modelId, int i_groupId) : 
            modelId(i_modelId), 
            groupId(i_groupId), 
            isParam(false),
            name(""),
            isHidden(false),
            isGenerated(false)
        { }

        /** create row with default empty field values. */
        GroupLstRow(void) : GroupLstRow(0, 0) { }

        ~GroupLstRow(void) throw() { }

        /** less comparator by primary key: model id, group id. */
        static bool isKeyLess(const GroupLstRow & i_left, const GroupLstRow & i_right);

        /** equal comparator by primary key: model id, group id. */
        static bool isKeyEqual(const GroupLstRow & i_left, const GroupLstRow & i_right);
    };

    /** group_txt table row. */
    struct GroupTxtRow : public IMetaRow<GroupTxtRow>
    {
        /** model_id INT          NOT NULL */
        int modelId;
        
        /** group_id INT          NOT NULL */
        int groupId;
        
        /** lang_id  INT          NOT NULL */
        int langId;
        
        /** descr    VARCHAR(255) NOT NULL */
        string descr;
        
        /** note     VARCHAR(32000) */
        string note;
        
        /** create row with supplied primary key field values. */
        GroupTxtRow(int i_modelId, int i_groupId, int i_langId) : 
            modelId(i_modelId), 
            groupId(i_groupId), 
            langId(i_langId),
            descr(""), 
            note("")
        { }

        /** create row with default empty field values. */
        GroupTxtRow(void) : GroupTxtRow(0, 0, 0) { }

        ~GroupTxtRow(void) throw() { }

        /** less comparator by primary key: model id, group id, language id. */
        static bool isKeyLess(const GroupTxtRow & i_left, const GroupTxtRow & i_right);

        /** equal comparator by primary key: model id, group id, language id. */
        static bool isKeyEqual(const GroupTxtRow & i_left, const GroupTxtRow & i_right);
    };

    /** group_txt table row and language name. */
    struct GroupTxtLangRow : public GroupTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: model id, group id, language name. */
        static bool uniqueLangKeyLess(const GroupTxtLangRow & i_left, const GroupTxtLangRow & i_right);

        /** equal comparator by unique key: model id, group id, language name. */
        static bool uniqueLangKeyEqual(const GroupTxtLangRow & i_left, const GroupTxtLangRow & i_right);
    };

    /** group_pc table row. */
    struct GroupPcRow : public IMetaRow<GroupPcRow>
    {
        /** model_id       INT NOT NULL */
        int modelId;

        /** group_id       INT NOT NULL */
        int groupId;

        /** child_pos      INT NOT NULL */
        int childPos;

        /** child_group_id INT NULL */
        int childGroupId;

        /** leaf_id        INT NULL */
        int leafId;

        /** create row with supplied primary key field values. */
        GroupPcRow(int i_modelId, int i_groupId, int i_childPos) : 
            modelId(i_modelId), 
            groupId(i_groupId), 
            childPos(i_childPos), 
            childGroupId(-1), 
            leafId(-1)
        { }

        /** create row with default empty field values. */
        GroupPcRow(void) : GroupPcRow(0, 0, 0) { }

        ~GroupPcRow(void) throw() { }

        /** less comparator by primary key: model id, group id, child position. */
        static bool isKeyLess(const GroupPcRow & i_left, const GroupPcRow & i_right);

        /** equal comparator by primary key: model id, group id, child position. */
        static bool isKeyEqual(const GroupPcRow & i_left, const GroupPcRow & i_right);
    };

    /** profile_lst table row. */
    struct ProfileLstRow : public IMetaRow<ProfileLstRow>
    {
        /** profile_name VARCHAR(255) NOT NULL */
        string name;

        /** create row with supplied primary key field values. */
        ProfileLstRow(const string & i_name) : name(i_name) { }

        /** create row with default empty field values. */
        ProfileLstRow(void) : ProfileLstRow("") { }

        ~ProfileLstRow(void) throw() { }

        /** less comparator by primary key: profile name. */
        static bool isKeyLess(const ProfileLstRow & i_left, const ProfileLstRow & i_right);

        /** equal comparator by primary key: profile name. */
        static bool isKeyEqual(const ProfileLstRow & i_left, const ProfileLstRow & i_right);
    };

    /** profile_option table row. */
    struct ProfileOptionRow : public IMetaRow<ProfileOptionRow>
    {
        /** profile_name VARCHAR(255)  NOT NULL */
        string name;

        /** option_key   VARCHAR(255)  NOT NULL */
        string key;

        /** option_value VARCHAR(2048) NOT NULL */
        string value;

        /** create row with supplied primary key field values. */
        ProfileOptionRow(const string & i_name, const string & i_key) :
            name(i_name),
            key(i_key),
            value("")
        { }

        /** create row with default empty field values. */
        ProfileOptionRow(void) : ProfileOptionRow("", "") { }

        ~ProfileOptionRow(void) throw() { }

        /** less comparator by primary key: profile name, option key. */
        static bool isKeyLess(const ProfileOptionRow & i_left, const ProfileOptionRow & i_right);

        /** equal comparator by primary key: profile name, option key. */
        static bool isKeyEqual(const ProfileOptionRow & i_left, const ProfileOptionRow & i_right);
    };

    /** run_option table row. */
    struct RunOptionRow : public IMetaRow<RunOptionRow>
    {
        /** run_id       INT NOT NULL */
        int runId;

        /** option_key   VARCHAR(255)  NOT NULL */
        string key;

        /** option_value VARCHAR(2048) NOT NULL */
        string value;

        /** create row with supplied primary key field values. */
        RunOptionRow(int i_runId, const string & i_key) :
            runId(i_runId),
            key(i_key),
            value("")
        { }

        /** create row with default empty field values. */
        RunOptionRow(void) : RunOptionRow(0, "") { }

        ~RunOptionRow(void) throw() { }

        /** less comparator by primary key: run id, option key. */
        static bool isKeyLess(const RunOptionRow & i_left, const RunOptionRow & i_right);

        /** equal comparator by primary key: run id, option key. */
        static bool isKeyEqual(const RunOptionRow & i_left, const RunOptionRow & i_right);
    };

    /** workset_lst table row. */
    struct WorksetLstRow : public IMetaRow<WorksetLstRow>
    {
        /** set_id      INT          NOT NULL */
        int setId;

        /** run_id      INT          NULL */
        int runId;

        /** model_id    INT          NOT NULL */
        int modelId;

        /** set_name    VARCHAR(255) NOT NULL */
        string name;

        /** is_readonly SMALLINT     NOT NULL */
        bool isReadonly;

        /** update_dt   VARCHAR(32)  NOT NULL */
        string updateDateTime;

        /** create row with supplied primary key field values. */
        WorksetLstRow(int i_setId) :
            setId(i_setId),
            runId(0),
            modelId(0),
            name(""),
            isReadonly(false),
            updateDateTime("")
        { }

        /** create row with default empty field values. */
        WorksetLstRow(void) : WorksetLstRow(0) { }

        ~WorksetLstRow(void) throw() { }

        /** less comparator by primary key: set id. */
        static bool isKeyLess(const WorksetLstRow & i_left, const WorksetLstRow & i_right);

        /** equal comparator by primary key: set id. */
        static bool isKeyEqual(const WorksetLstRow & i_left, const WorksetLstRow & i_right);

        /** find row by primary key: set id. */
        static vector<WorksetLstRow>::const_iterator byKey(int i_setId, const vector<WorksetLstRow> & i_rowVec);
    };

    /** workset_txt table row. */
    struct WorksetTxtRow : public IMetaRow<WorksetTxtRow>
    {
        /** set_id   INT          NOT NULL */
        int setId;

        /** model_id INT          NOT NULL */
        int modelId;

        /** lang_id  INT          NOT NULL */
        int langId;

        /** descr    VARCHAR(255) NOT NULL */
        string descr;

        /** note     VARCHAR(32000)          */
        string note;

        /** create row with supplied primary key field values. */
        WorksetTxtRow(int i_setId, int i_langId) :
            setId(i_setId),
            modelId(0),
            langId(i_langId),
            descr(""),
            note("")
        { }

        /** create row with default empty field values. */
        WorksetTxtRow(void) : WorksetTxtRow(0, 0) { }

        ~WorksetTxtRow(void) throw() { }

        /** less comparator by primary key: set id and language id. */
        static bool isKeyLess(const WorksetTxtRow & i_left, const WorksetTxtRow & i_right);

        /** equal comparator by primary key: set id and language id. */
        static bool isKeyEqual(const WorksetTxtRow & i_left, const WorksetTxtRow & i_right);

        /** find row by primary key: set id and language id. */
        static vector<WorksetTxtRow>::const_iterator byKey(int i_setId, int i_langId, const vector<WorksetTxtRow> & i_rowVec);
    };

    /** workset_txt table row and language name. */
    struct WorksetTxtLangRow : public WorksetTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: set id, language name. */
        static bool uniqueLangKeyLess(const WorksetTxtLangRow & i_left, const WorksetTxtLangRow & i_right);

        /** equal comparator by unique key: set id, language name. */
        static bool uniqueLangKeyEqual(const WorksetTxtLangRow & i_left, const WorksetTxtLangRow & i_right);
    };

    /** workset_parameter table row. */
    struct WorksetParamRow : public IMetaRow<WorksetParamRow>
    {
        /** set_id       INT NOT NULL */
        int setId;

        /** model_id     INT NOT NULL */
        int modelId;

        /** parameter_id INT NOT NULL */
        int paramId;

        /** create row with supplied primary key field values. */
        WorksetParamRow(int i_setId, int i_paramId) :
            setId(i_setId),
            modelId(0),
            paramId(i_paramId)
        { }

        /** create row with default empty field values. */
        WorksetParamRow(void) : WorksetParamRow(0, 0) { }

        ~WorksetParamRow(void) throw() { }

        /** less comparator by primary key: set id and parameter id. */
        static bool isKeyLess(const WorksetParamRow & i_left, const WorksetParamRow & i_right);

        /** equal comparator by primary key: set id and parameter id. */
        static bool isKeyEqual(const WorksetParamRow & i_left, const WorksetParamRow & i_right);

        /** find row by primary key: set id and parameter id. */
        static vector<WorksetParamRow>::const_iterator byKey(int i_setId, int i_paramId, const vector<WorksetParamRow> & i_rowVec);
    };

    /** workset_parameter_txt table row. */
    struct WorksetParamTxtRow : public IMetaRow<WorksetParamTxtRow>
    {
        /** set_id       INT        NOT NULL */
        int setId;

        /** model_id     INT        NOT NULL */
        int modelId;

        /** parameter_id INT        NOT NULL */
        int paramId;

        /** lang_id      INT        NOT NULL */
        int langId;

        /** note         VARCHAR(32000)      */
        string note;

        /** create row with supplied primary key field values. */
        WorksetParamTxtRow(int i_setId, int i_langId, int i_paramId) :
            setId(i_setId),
            modelId(0),
            paramId(i_paramId),
            langId(i_langId),
            note("")
        { }

        /** create row with default empty field values. */
        WorksetParamTxtRow(void) : WorksetParamTxtRow(0, 0, 0) { }

        ~WorksetParamTxtRow(void) throw() { }

        /** less comparator by primary key: set id, parameter id, language id. */
        static bool isKeyLess(const WorksetParamTxtRow & i_left, const WorksetParamTxtRow & i_right);

        /** equal comparator by primary key: set id, parameter id, language id. */
        static bool isKeyEqual(const WorksetParamTxtRow & i_left, const WorksetParamTxtRow & i_right);

        /** find row by primary key: set id, parameter id, language id. */
        static vector<WorksetParamTxtRow>::const_iterator byKey(
            int i_setId, int i_paramId, int i_langId, const vector<WorksetParamTxtRow> & i_rowVec
            );
    };

    /** workset_parameter_txt table row and language name. */
    struct WorksetParamTxtLangRow : public WorksetParamTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: set id, parameter id, language name. */
        static bool uniqueLangKeyLess(const WorksetParamTxtLangRow & i_left, const WorksetParamTxtLangRow & i_right);

        /** equal comparator by unique key: set id, parameter id, language name. */
        static bool uniqueLangKeyEqual(const WorksetParamTxtLangRow & i_left, const WorksetParamTxtLangRow & i_right);
    };

    /** task_lst table row. */
    struct TaskLstRow : public IMetaRow<TaskLstRow>
    {
        /** task_id   INT          NOT NULL */
        int taskId;

        /** model_id  INT          NOT NULL */
        int modelId;

        /** task_name VARCHAR(255) NOT NULL */
        string name;

        /** create row with supplied primary key field values. */
        TaskLstRow(int i_taskId) :
            taskId(i_taskId),
            modelId(0),
            name("")
        { }

        /** create row with default empty field values. */
        TaskLstRow(void) : TaskLstRow(0) { }

        ~TaskLstRow(void) throw() { }

        /** less comparator by primary key: task id. */
        static bool isKeyLess(const TaskLstRow & i_left, const TaskLstRow & i_right);

        /** equal comparator by primary key: task id. */
        static bool isKeyEqual(const TaskLstRow & i_left, const TaskLstRow & i_right);

        /** find row by primary key: task id. */
        static vector<TaskLstRow>::const_iterator byKey(int i_taskId, const vector<TaskLstRow> & i_rowVec);
    };

    /** task_txt table row. */
    struct TaskTxtRow : public IMetaRow<TaskTxtRow>
    {
        /** task_id INT             NOT NULL */
        int taskId;

        /** lang_id INT             NOT NULL */
        int langId;

        /** descr   VARCHAR(255)    NOT NULL */
        string descr;

        /** note    VARCHAR(32000)           */
        string note;

        /** create row with supplied primary key field values. */
        TaskTxtRow(int i_taskId, int i_langId) :
            taskId(i_taskId),
            langId(i_langId),
            descr(""),
            note("")
        { }

        /** create row with default empty field values. */
        TaskTxtRow(void) : TaskTxtRow(0, 0) { }

        ~TaskTxtRow(void) throw() { }

        /** less comparator by primary key: task id and language id. */
        static bool isKeyLess(const TaskTxtRow & i_left, const TaskTxtRow & i_right);

        /** equal comparator by primary key: task id and language id. */
        static bool isKeyEqual(const TaskTxtRow & i_left, const TaskTxtRow & i_right);

        /** find row by primary key: task id and language id. */
        static vector<TaskTxtRow>::const_iterator byKey(int i_taskId, int i_langId, const vector<TaskTxtRow> & i_rowVec);
    };

    /** task_txt table row and language name. */
    struct TaskTxtLangRow : public TaskTxtRow
    {
        /** language name */
        string langName;

        /** less comparator by unique key: task id, language name. */
        static bool uniqueLangKeyLess(const TaskTxtLangRow & i_left, const TaskTxtLangRow & i_right);

        /** equal comparator by unique key: task id, language name. */
        static bool uniqueLangKeyEqual(const TaskTxtLangRow & i_left, const TaskTxtLangRow & i_right);
    };

    /** task_set table row. */
    struct TaskSetRow : public IMetaRow<TaskSetRow>
    {
        /** task_id INT NOT NULL  */
        int taskId;

        /** set_id  INT NOT NULL  */
        int setId;

        /** create row with supplied primary key field values. */
        TaskSetRow(int i_taskId, int i_setId) :
            taskId(i_taskId),
            setId(i_setId)
        { }

        /** create row with default empty field values. */
        TaskSetRow(void) : TaskSetRow(0, 0) { }

        ~TaskSetRow(void) throw() { }

        /** less comparator by primary key: task id and set id. */
        static bool isKeyLess(const TaskSetRow & i_left, const TaskSetRow & i_right);

        /** equal comparator by primary key: task id and set id. */
        static bool isKeyEqual(const TaskSetRow & i_left, const TaskSetRow & i_right);

        /** find row by primary key: task id and set id. */
        static vector<TaskSetRow>::const_iterator byKey(int i_taskId, int i_setId, const vector<TaskSetRow> & i_rowVec);
    };

    /** task_run_lst table row. */
    struct TaskRunLstRow : public IMetaRow<TaskRunLstRow>
    {
        /** task_run_id INT         NOT NULL */
        int taskRunId;

        /** task_id     INT         NOT NULL */
        int taskId;

        /** sub_count   INT         NOT NULL */
        int subCount;

        /** create_dt   VARCHAR(32) NOT NULL */
        string createDateTime;

        /** task status: i=init p=progress w=wait s=success x=exit e=error(failed) */
        string status;

        /** update_dt   VARCHAR(32) NOT NULL */
        string updateDateTime;

        /** create row with supplied primary key field values. */
        TaskRunLstRow(int i_taskRunId) :
            taskRunId(i_taskRunId),
            taskId(0),
            subCount(0),
            createDateTime(""),
            status(""),
            updateDateTime("")
        { }

        /** create row with default empty field values. */
        TaskRunLstRow(void) : TaskRunLstRow(0) { }

        ~TaskRunLstRow(void) throw() { }

        /** less comparator by primary key: task run id. */
        static bool isKeyLess(const TaskRunLstRow & i_left, const TaskRunLstRow & i_right);

        /** equal comparator by primary key: task run id. */
        static bool isKeyEqual(const TaskRunLstRow & i_left, const TaskRunLstRow & i_right);

        /** find row by primary key: task run id. */
        static vector<TaskRunLstRow>::const_iterator byKey(int i_taskRunId, const vector<TaskRunLstRow> & i_rowVec);
    };

    /** task_run_set table row. */
    struct TaskRunSetRow : public IMetaRow<TaskRunSetRow>
    {
        /** task_run_id INT NOT NULL */
        int taskRunId;

        /** run_id      INT NOT NULL */
        int runId;

        /** set_id      INT NOT NULL */
        int setId;

        /** task_id     INT NOT NULL */
        int taskId;

        /** create row with supplied primary key field values. */
        TaskRunSetRow(int i_taskRunId, int i_runId) :
            taskRunId(i_taskRunId),
            runId(i_runId),
            setId(0),
            taskId(0)
        { }

        /** create row with default empty field values. */
        TaskRunSetRow(void) : TaskRunSetRow(0, 0) { }

        ~TaskRunSetRow(void) throw() { }

        /** less comparator by primary key: task run id, run id. */
        static bool isKeyLess(const TaskRunSetRow & i_left, const TaskRunSetRow & i_right);

        /** equal comparator by primary key: task run id, run id. */
        static bool isKeyEqual(const TaskRunSetRow & i_left, const TaskRunSetRow & i_right);

        /** find row by primary key: task run id, run id. */
        static vector<TaskRunSetRow>::const_iterator byKey(int i_taskRunId, int i_runId, const vector<TaskRunSetRow> & i_rowVec);
    };
}

#endif  // DB_META_ROW_H
