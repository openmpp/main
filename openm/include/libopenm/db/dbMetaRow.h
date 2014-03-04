/**
 * @file
 * OpenM++ data library: db rows of model metadata tables
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef DB_META_ROW_H
#define DB_META_ROW_H

#include <memory>
using namespace std;

#include "libopenm/omCommon.h"
#include "dbCommon.h"

namespace openm
{
    /** lang_lst table row. */
    struct LangLstRow : public IRowBase
    {
        /** lang_id   INT         NOT NULL */
        int langId;

        /** lang_code VARCHAR(32) NOT NULL */
        string code;

        /** lang_name VARCHAR(32) NOT NULL */
        string name;
        
        /** create row with default empty field values. */
        LangLstRow(void) : 
            langId(0), 
            code(""), 
            name("")
        { }

        /** create row with supplied primary key field values. */
        LangLstRow(int i_langId) : 
            langId(i_langId), 
            code(""), 
            name("")
        { }

        ~LangLstRow(void) throw() { }

        /** less comparator by primary key: language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** lang_lst row equal comparator by language code. */
        static bool codeEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** lang_word table row. */
    struct LangWordRow : public IRowBase
    {
        /** lang_id    INT         NOT NULL */
        int langId;

        /** word_code  VARCHAR(8)  NOT NULL */
        string code;

        /** word_value VARCHAR(32) NOT NULL */
        string value;
        
        /** create row with default empty field values. */
        LangWordRow(void) : 
            langId(0), 
            code(""), 
            value("")
        { }

        /** create row with supplied primary key field values. */
        LangWordRow(int i_langId, const string & i_code) : 
            langId(i_langId), 
            code(i_code), 
            value("")
        { }

        ~LangWordRow(void) throw() { }

        /** less comparator by primary key: language id and word code. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: language id and word code. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** model_dic table row. */
    struct ModelDicRow : public IRowBase
    {
        /** model_id       INT         NOT NULL */
        int modelId;

        /** model_name     VARCHAR(32) NOT NULL */
        string name;

        /** model_type     INT         NOT NULL */
        ModelType type;

        /** model_ver      VARCHAR(255) NOT NULL */
        string version;

        /** model_ts       VARCHAR(32) NOT NULL */
        string timestamp;

        /** model_prefix   VARCHAR(32) NOT NULL */
        string modelPrefix;

        /** parameter_prefix VARCHAR(4)  NOT NULL, -- parameter tables prefix: p */
        string paramPrefix;

        /** workset_prefix   VARCHAR(4)  NOT NULL, -- workset tables prefix: w */
        string setPrefix;

        /** sub_prefix       VARCHAR(4)  NOT NULL, -- subsample tables prefix: s */
        string subPrefix;

        /** value_prefix     VARCHAR(4)  NOT NULL, -- value tables prefix: v */
        string valuePrefix;

        /** create row with default empty field values. */
        ModelDicRow(void) : 
            modelId(0), 
            name(""), 
            type(ModelType::caseBased),
            version(""),
            timestamp(""),
            modelPrefix(""),
            paramPrefix(""),
            setPrefix(""),
            subPrefix(""),
            valuePrefix("")
        { }

        /** create row with supplied primary key field values. */
        ModelDicRow(int i_modelId) : 
            modelId(i_modelId), 
            name(""), 
            type(ModelType::caseBased),
            version(""),
            timestamp(""),
            modelPrefix(""),
            paramPrefix(""),
            setPrefix(""),
            subPrefix(""),
            valuePrefix("")
        { }

        ~ModelDicRow(void) throw() { }

        /** less comparator by primary key: model id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** model_dic row equal comparator by model name and timestamp. */
        static bool nameTimeStampEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** model_dic_txt table row. */
    struct ModelDicTxtRow : public IRowBase
    {
        /** model_id INT NOT NULL */
        int modelId;

        /** lang_id  INT NOT NULL */
        int langId;

        /** descr VARCHAR(255) NOT NULL */
        string descr;

        /** note     VARCHAR(32000) */
        string note;

        /** create row with default empty field values. */
        ModelDicTxtRow(void) : 
            modelId(0), 
            langId(0),
            descr(""), 
            note("")
        { }

        /** create row with supplied primary key field values. */
        ModelDicTxtRow(int i_modelId, int i_langId) : 
            modelId(i_modelId), 
            langId(i_langId),
            descr(""), 
            note("")
        { }

        ~ModelDicTxtRow(void) throw() { }

        /** less comparator by primary key: model id and language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** model_dic_txt table row and language name. */
    struct ModelDicTxtLangRow : public ModelDicTxtRow
    {
        /** language name */
        string langName;
    };

    /** run_lst table row. */
    struct RunLstRow : public IRowBase
    {
        /** run_id        INT         NOT NULL */
        int runId;

        /** model_id      INT         NOT NULL */
        int modelId;

        /** run_name      VARCHAR(64) NOT NULL */
        string name;

        /** sub_count     INT         NOT NULL */
        int subCount;

        /** sub_started   INT         NOT NULL */
        int subStarted;

        /** sub_completed INT         NOT NULL */
        int subCompleted;

        /** create_dt     VARCHAR(32) NOT NULL */
        string createDateTime;

        /** create row with default empty field values. */
        RunLstRow(void) : 
            runId(0),
            modelId(0),
            name(""),
            subCount(0),
            subStarted(0),
            subCompleted(0),
            createDateTime("")
        { }

        /** create row with supplied primary key field values. */
        RunLstRow(int i_runId) : 
            runId(i_runId),
            modelId(0),
            name(""),
            subCount(0),
            subStarted(0),
            subCompleted(0),
            createDateTime("")
        { }

        ~RunLstRow(void) throw() { }

        /** less comparator by primary key: run id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: run id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** find row by primary key: run id. */
        static vector<RunLstRow>::const_iterator byKey(int i_runId, const vector<RunLstRow> & i_rowVec)
        {
            return find_if(
                i_rowVec.cbegin(), 
                i_rowVec.cend(),
                [i_runId](const RunLstRow & i_row) -> bool { return i_row.runId == i_runId; }
                );
        }
    };

    /** run_txt table row. */
    struct RunTxtRow : public IRowBase
    {
        /** run_id   INT            NOT NULL */
        int runId;

        /** model_id INT            NOT NULL */
        int modelId;

        /** lang_id  INT            NOT NULL */
        int langId;

        /** descr VARCHAR(255) NOT NULL */
        string descr;

        /** note     VARCHAR(32000)          */
        string note;

        /** create row with default empty field values. */
        RunTxtRow(void) : 
            runId(0),
            modelId(0),
            langId(0),
            descr(""),
            note("")
        { }

        /** create row with supplied primary key field values. */
        RunTxtRow(int i_runId, int i_langId) : 
            runId(i_runId),
            modelId(0),
            langId(i_langId),
            descr(""),
            note("")
        { }

        ~RunTxtRow(void) throw() { }

        /** less comparator by primary key: run id and language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: run id and language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** find row by primary key: run id and language id. */
        static vector<RunTxtRow>::const_iterator byKey(int i_runId, int i_langId, const vector<RunTxtRow> & i_rowVec)
        {
            return find_if(
                i_rowVec.cbegin(), 
                i_rowVec.cend(),
                [i_runId, i_langId](const RunTxtRow & i_row) -> bool { return i_row.runId == i_runId && i_row.langId == i_langId; }
                );
        }
    };

    /** profile_lst table row. */
    struct ProfileLstRow : public IRowBase
    {
        /** profile_name VARCHAR(32) NOT NULL */
        string name;

        /** create row with default empty field values. */
        ProfileLstRow(void) : name("") { }

        /** create row with supplied primary key field values. */
        ProfileLstRow(const string & i_name) : name(i_name) { }

        ~ProfileLstRow(void) throw() { }

        /** less comparator by primary key: profile name. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: profile name. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** profile_option table row. */
    struct ProfileOptionRow : public IRowBase
    {
        /** profile_name VARCHAR(32) NOT NULL */
        string name;

        /** option_key   VARCHAR(255)  NOT NULL */
        string key;

        /** option_value VARCHAR(1024) NOT NULL */
        string value;

        /** create row with default empty field values. */
        ProfileOptionRow(void) : 
            name(""), 
            key(""),
            value("")
        { }

        /** create row with supplied primary key field values. */
        ProfileOptionRow(const string & i_name, const string & i_key) : 
            name(i_name), 
            key(i_key),
            value("")
        { }

        ~ProfileOptionRow(void) throw() { }

        /** less comparator by primary key: profile name, option key. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: profile name, option key. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by profile name. */
        static bool nameEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** run_option table row. */
    struct RunOptionRow : public IRowBase
    {
        /** run_id    INT NOT NULL */
        int runId;

        /** option_key   VARCHAR(255)  NOT NULL */
        string key;

        /** option_value VARCHAR(1024) NOT NULL */
        string value;

        /** create row with default empty field values. */
        RunOptionRow(void) : 
            runId(0), 
            key(""),
            value("")
        { }

        /** create row with supplied primary key field values. */
        RunOptionRow(int i_runId, const string & i_key) : 
            runId(i_runId), 
            key(i_key),
            value("")
        { }

        ~RunOptionRow(void) throw() { }

        /** less comparator by primary key: run id, option key. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: run id, option key. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** run_option row equal comparator by run id. */
        static bool runIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** type_dic table row. */
    struct TypeDicRow : public IRowBase
    {
        /** model_id      INT         NOT NULL */
        int modelId;

        /** mod_type_id   INT         NOT NULL */
        int typeId;

        /** mod_type_name VARCHAR(64) NOT NULL, -- type name: int, double, etc. */
        string name;

        /** dic_id INT NOT NULL, -- dictionary id: 0=simple 1=logical 2=classification 3=range 4=partition 5=link */
        int dicId;

        /** total_enum_id INT NOT NULL, -- if total enabled this is enum_value of total item =max+1 */
        int totalEnumId;

        /** create row with default empty field values. */
        TypeDicRow(void) : 
            modelId(0), 
            typeId(0), 
            name(""),
            dicId(0), 
            totalEnumId(0)
        { }

        /** create row with supplied primary key field values. */
        TypeDicRow(int i_modelId, int i_typeId) : 
            modelId(i_modelId), 
            typeId(i_typeId), 
            name(""),
            dicId(0), 
            totalEnumId(0)
        { }

        ~TypeDicRow(void) throw() { }

        /** less comparator by primary key: model id, type id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, type id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by model id. */
        static bool modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by model id and type name. */
        static bool modelIdNameEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** type_dic_txt table row. */
    struct TypeDicTxtRow : public IRowBase
    {
        /** model_id     INT NOT NULL */
        int modelId;
        
        /** mod_type_id  INT NOT NULL */
        int typeId;
        
        /** lang_id      INT NOT NULL */
        int langId;
        
        /** descr VARCHAR(255) NOT NULL */
        string descr;
        
        /** note         VARCHAR(32000) */
        string note;
        
        /** create row with default empty field values. */
        TypeDicTxtRow(void) : 
            modelId(0), 
            typeId(0), 
            langId(0),
            descr(""), 
            note("")
        { }

        /** create row with supplied primary key field values. */
        TypeDicTxtRow(int i_modelId, int i_typeId, int i_langId) : 
            modelId(i_modelId), 
            typeId(i_typeId), 
            langId(i_langId),
            descr(""), 
            note("")
        { }

        ~TypeDicTxtRow(void) throw() { }

        /** less comparator by primary key: model id, type id, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, type id, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** type_dic_txt table row and language name. */
    struct TypeDicTxtLangRow : public TypeDicTxtRow
    {
        /** language name */
        string langName;
    };

    /** type_enum_lst table row. */
    struct TypeEnumLstRow : public IRowBase
    {
        /** model_id      INT         NOT NULL */
        int modelId;

        /** mod_type_id   INT         NOT NULL */
        int typeId;

        /** enum_id       INT         NOT NULL */
        int enumId;

        /** enum_name     VARCHAR(64) NOT NULL */
        string name;

        /** create row with default empty field values. */
        TypeEnumLstRow(void) : 
            modelId(0), 
            typeId(0), 
            enumId(0), 
            name("")
        { }

        /** create row with supplied primary key field values. */
        TypeEnumLstRow(int i_modelId, int i_typeId, int i_enumId) : 
            modelId(i_modelId), 
            typeId(i_typeId), 
            enumId(i_enumId), 
            name("")
        { }

        ~TypeEnumLstRow(void) throw() { }

        /** less comparator by primary key: model id, type id, enum id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, type id, enum id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by model id. */
        static bool modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by model id and type id. */
        static bool modelIdTypeIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** type_enum_txt table row. */
    struct TypeEnumTxtRow : public IRowBase
    {
        /** model_id     INT NOT NULL */
        int modelId;
        
        /** mod_type_id  INT NOT NULL */
        int typeId;
        
        /** enum_id      INT NOT NULL */
        int enumId;
        
        /** lang_id      INT NOT NULL */
        int langId;
        
        /** descr VARCHAR(255) NOT NULL */
        string descr;
        
        /** note         VARCHAR(32000) */
        string note;
        
        /** create row with default empty field values. */
        TypeEnumTxtRow(void) : 
            modelId(0), 
            typeId(0), 
            enumId(0),
            langId(0),
            descr(""), 
            note("")
        { }

        /** create row with supplied primary key field values. */
        TypeEnumTxtRow(int i_modelId, int i_typeId, int i_enumId, int i_langId) : 
            modelId(i_modelId), 
            typeId(i_typeId), 
            enumId(i_enumId),
            langId(i_langId),
            descr(""), 
            note("")
        { }

        ~TypeEnumTxtRow(void) throw() { }

        /** less comparator by primary key: model id, type id, enum id, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, type id, enum id, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by model id and type id. */
        static bool modelIdTypeIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** type_enum_txt table row and language name. */
    struct TypeEnumTxtLangRow : public TypeEnumTxtRow
    {
        /** language name */
        string langName;
    };

    /** parameter_dic table row. */
    struct ParamDicRow : public IRowBase
    {
        /** model_id       INT         NOT NULL */
        int modelId;

        /** parameter_id   INT         NOT NULL */
        int paramId;

        /** db_name_suffix VARCHAR(32) NOT NULL */
        string dbNameSuffix;

        /** parameter_name VARCHAR(64) NOT NULL */
        string paramName;

        /** parameter_rank INT         NOT NULL */
        int rank;

        /** mod_type_id    INT NOT NULL */
        int typeId;

        /** is_hidden      SMALLINT    NOT NULL */
        bool isHidden;

        /** is_generated   SMALLINT    NOT NULL */
        bool isGenerated;

        /** num_cumulated  INT         NOT NULL */
        int numCumulated;

        /** create row with default empty field values. */
        ParamDicRow(void) : 
            modelId(0), 
            paramId(0), 
            dbNameSuffix(""),
            paramName(""),
            rank(0),
            typeId(0),
            isHidden(false),
            isGenerated(false),
            numCumulated(0)
        { }

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

        ~ParamDicRow(void) throw() { }

        /** less comparator by primary key: model id, parameter id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, parameter id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** parameter_dic row equal comparator by model id. */
        static bool modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** parameter_dic row equal comparator by model id and parameter name. */
        static bool modelIdNameEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** parameter_dic_txt table row. */
    struct ParamDicTxtRow : public IRowBase
    {
        /** model_id     INT NOT NULL */
        int modelId;
        
        /** parameter_id INT NOT NULL */
        int paramId;
        
        /** lang_id      INT NOT NULL */
        int langId;
        
        /** descr VARCHAR(255) NOT NULL */
        string descr;
        
        /** note         VARCHAR(32000) */
        string note;
        
        /** create row with default empty field values. */
        ParamDicTxtRow(void) : 
            modelId(0), 
            paramId(0), 
            langId(0),
            descr(""), 
            note("")
        { }

        /** create row with supplied primary key field values. */
        ParamDicTxtRow(int i_modelId, int i_paramId, int i_langId) : 
            modelId(i_modelId), 
            paramId(i_paramId), 
            langId(i_langId),
            descr(""), 
            note("")
        { }

        ~ParamDicTxtRow(void) throw() { }

        /** less comparator by primary key: model id, parameter id, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, parameter id, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** parameter_dic_txt table row and language name. */
    struct ParamDicTxtLangRow : public ParamDicTxtRow
    {
        /** language name */
        string langName;
    };

    /** parameter_run_txt table row. */
    struct ParamRunTxtRow : public IRowBase
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
        
        /** create row with default empty field values. */
        ParamRunTxtRow(void) : 
            runId(0), 
            modelId(0), 
            paramId(0), 
            langId(0),
            note("")
        { }

        /** create row with supplied primary key field values. */
        ParamRunTxtRow(int i_runId, int i_paramId, int i_langId) : 
            runId(i_runId), 
            modelId(0), 
            paramId(i_paramId), 
            langId(i_langId),
            note("")
        { }

        ~ParamRunTxtRow(void) throw() { }

        /** less comparator by primary key: run id, parameter id, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: run id, parameter id, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** parameter_run_txt table row and language name. */
    struct ParamRunTxtLangRow : public ParamRunTxtRow
    {
        /** language name */
        string langName;
    };

    /** parameter_dims table row. */
    struct ParamDimsRow : public IRowBase
    {
        /** model_id     INT NOT NULL */
        int modelId;
        
        /** parameter_id INT NOT NULL */
        int paramId;
        
        /** dim_name     VARCHAR(8) NOT NULL */
        string name;
        
        /** dim_pos      INT NOT NULL */
        int pos;
        
        /** mod_type_id  INT NOT NULL */
        int typeId;
        
        /** create row with default empty field values. */
        ParamDimsRow(void) : 
            modelId(0), 
            paramId(0), 
            name(""),
            pos(0),
            typeId(0)
        { }

        /** create row with supplied primary key field values. */
        ParamDimsRow(int i_modelId, int i_paramId, const string & i_name) : 
            modelId(i_modelId), 
            paramId(i_paramId), 
            name(i_name),
            pos(0),
            typeId(0)
        { }

        ~ParamDimsRow(void) throw() { }

        /** less comparator by primary key: model id, parameter id, dimension name. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, parameter id, dimension name. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** table_dims row equal comparator by model id, parameter id. */
        static bool modelIdParamIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** workset_lst table row. */
    struct WorksetLstRow : public IRowBase
    {
        /** set_id      INT         NOT NULL */
        int setId;

        /** run_id      INT         NULL */
        int runId;

        /** model_id    INT         NOT NULL */
        int modelId;

        /** is_readonly SMALLINT    NOT NULL */
        bool isReadonly;
        
        /** update_dt   VARCHAR(32) NOT NULL */
        string updateDateTime;

        /** create row with default empty field values. */
        WorksetLstRow(void) : 
            setId(0),
            runId(0),
            modelId(0),
            isReadonly(false),
            updateDateTime("")
        { }

        /** create row with supplied primary key field values. */
        WorksetLstRow(int i_setId) : 
            setId(i_setId),
            runId(0),
            modelId(0),
            isReadonly(false),
            updateDateTime("")
        { }

        ~WorksetLstRow(void) throw() { }

        /** less comparator by primary key: set id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: set id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** find row by primary key: set id. */
        static vector<WorksetLstRow>::const_iterator byKey(int i_setId, const vector<WorksetLstRow> & i_rowVec)
        {
            return find_if(
                i_rowVec.cbegin(), 
                i_rowVec.cend(),
                [i_setId](const WorksetLstRow & i_row) -> bool { return i_row.setId == i_setId; }
                );
        }
    };

    /** workset_txt table row. */
    struct WorksetTxtRow : public IRowBase
    {
        /** set_id   INT            NOT NULL */
        int setId;

        /** model_id INT            NOT NULL */
        int modelId;

        /** lang_id  INT            NOT NULL */
        int langId;

        /** descr VARCHAR(255) NOT NULL */
        string descr;

        /** note     VARCHAR(32000)          */
        string note;

        /** create row with default empty field values. */
        WorksetTxtRow(void) : 
            setId(0),
            modelId(0),
            langId(0),
            descr(""),
            note("")
        { }

        /** create row with supplied primary key field values. */
        WorksetTxtRow(int i_setId, int i_langId) : 
            setId(i_setId),
            modelId(0),
            langId(i_langId),
            descr(""),
            note("")
        { }

        ~WorksetTxtRow(void) throw() { }

        /** less comparator by primary key: set id and language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: set id and language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** find row by primary key: set id and language id. */
        static vector<WorksetTxtRow>::const_iterator byKey(int i_setId, int i_langId, const vector<WorksetTxtRow> & i_rowVec)
        {
            return find_if(
                i_rowVec.cbegin(), 
                i_rowVec.cend(),
                [i_setId, i_langId](const WorksetTxtRow & i_row) -> bool { return i_row.setId == i_setId && i_row.langId == i_langId; }
                );
        }
    };

    /** workset_parameter table row. */
    struct WorksetParamRow : public IRowBase
    {
        /** set_id       INT NOT NULL */
        int setId;

        /** model_id     INT NOT NULL */
        int modelId;

        /** parameter_id INT NOT NULL */
        int paramId;

        /** create row with default empty field values. */
        WorksetParamRow(void) : 
            setId(0),
            modelId(0),
            paramId(0)
        { }

        /** create row with supplied primary key field values. */
        WorksetParamRow(int i_setId, int i_paramId) : 
            setId(i_setId),
            modelId(0),
            paramId(i_paramId)
        { }

        ~WorksetParamRow(void) throw() { }

        /** less comparator by primary key: set id and parameter id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: set id and parameter id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** find row by primary key: set id and parameter id. */
        static vector<WorksetParamRow>::const_iterator byKey(int i_setId, int i_paramId, const vector<WorksetParamRow> & i_rowVec)
        {
            return find_if(
                i_rowVec.cbegin(), 
                i_rowVec.cend(),
                [i_setId, i_paramId](const WorksetParamRow & i_row) -> bool { return i_row.setId == i_setId && i_row.paramId == i_paramId; }
                );
        }
    };

    /** workset_parameter_txt table row. */
    struct WorksetParamTxtRow : public IRowBase
    {
        /** set_id   INT            NOT NULL */
        int setId;

        /** model_id INT            NOT NULL */
        int modelId;

        /** parameter_id INT        NOT NULL */
        int paramId;

        /** lang_id  INT            NOT NULL */
        int langId;

        /** note     VARCHAR(32000)          */
        string note;

        /** create row with default empty field values. */
        WorksetParamTxtRow(void) : 
            setId(0),
            modelId(0),
            paramId(0),
            langId(0),
            note("")
        { }

        /** create row with supplied primary key field values. */
        WorksetParamTxtRow(int i_setId, int i_langId, int i_paramId) : 
            setId(i_setId),
            modelId(0),
            paramId(i_paramId),
            langId(i_langId),
            note("")
        { }

        ~WorksetParamTxtRow(void) throw() { }

        /** less comparator by primary key: set id, parameter id, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: set id, parameter id, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** find row by primary key: set id, parameter id, language id. */
        static vector<WorksetParamTxtRow>::const_iterator byKey(
            int i_setId, int i_paramId, int i_langId, const vector<WorksetParamTxtRow> & i_rowVec
            )
        {
            return find_if(
                i_rowVec.cbegin(), 
                i_rowVec.cend(),
                [i_setId, i_paramId, i_langId](const WorksetParamTxtRow & i_row) -> bool 
                { 
                    return i_row.setId == i_setId && i_row.paramId == i_paramId && i_row.langId == i_langId; 
                }
                );
        }
    };

    /** table_dic table row. */
    struct TableDicRow : public IRowBase
    {
        /** model_id INT NOT NULL */
        int modelId;
        
        /** table_id INT NOT NULL */
        int tableId;
        
        /** db_name_suffix VARCHAR(64) NOT NULL */
        string dbNameSuffix;
        
        /** table_name VARCHAR(64) NOT NULL */
        string tableName;
        
        /** is_user SMALLINT NOT NULL */
        bool isUser;
        
        /** table_rank INT NOT NULL */
        int rank;
        
        /** is_sparse SMALLINT NOT NULL */
        bool isSparse;
        
        /** is_hidden SMALLINT NOT NULL */
        bool isHidden;

        /** create row with default empty field values. */
        TableDicRow(void) : 
            modelId(0), 
            tableId(0), 
            dbNameSuffix(""),
            tableName(""),
            isUser(false),
            rank(1),
            isSparse(false),
            isHidden(false)
        { }

        /** create row with supplied primary key field values. */
        TableDicRow(int i_modelId, int i_tableId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            dbNameSuffix(""),
            tableName(""),
            isUser(false),
            rank(1),
            isSparse(false),
            isHidden(false)
        { }

        ~TableDicRow(void) throw() { }

        /** less comparator by primary key: model id, table id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, table id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** table_dic row equal comparator by model id. */
        static bool modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** table_dic row equal comparator by model id and table name. */
        static bool modelIdNameEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** table_dic_txt table row. */
    struct TableDicTxtRow : public IRowBase
    {
        /** model_id       INT NOT NULL */
        int modelId;
        
        /** table_id       INT NOT NULL */
        int tableId;
        
        /** lang_id        INT NOT NULL */
        int langId;
        
        /** descr VARCHAR(255) NOT NULL */
        string descr;
        
        /** note           VARCHAR(32000) */
        string note;
        
        /** unit_descr VARCHAR(255) NOT NULL */
        string unitDescr;
        
        /** unit_note  VARCHAR(32000) */
        string unitNote;
        
        /** create row with default empty field values. */
        TableDicTxtRow(void) : 
            modelId(0), 
            tableId(0), 
            langId(0),
            descr(""), 
            note(""),
            unitDescr(""),
            unitNote("")
        { }

        /** create row with supplied primary key field values. */
        TableDicTxtRow(int i_modelId, int i_tableId, int i_langId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            langId(i_langId),
            descr(""), 
            note(""),
            unitDescr(""),
            unitNote("")
        { }

        ~TableDicTxtRow(void) throw() { }

        /** less comparator by primary key: model id, table id, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, table id, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** table_dic_txt table row and language name. */
    struct TableDicTxtLangRow : public TableDicTxtRow
    {
        /** language name */
        string langName;
    };

    /** table_dims table row. */
    struct TableDimsRow : public IRowBase
    {
        /** model_id    INT NOT NULL */
        int modelId;
        
        /** table_id    INT NOT NULL */
        int tableId;
        
        /** dim_name    VARCHAR(8) NOT NULL */
        string name;
        
        /** dim_pos     INT NOT NULL */
        int pos;
        
        /** mod_type_id INT NOT NULL */
        int typeId;
        
        /** is_total    SMALLINT NOT NULL */
        bool isTotal;
        
        /** dim_size    INT NOT NULL */
        int dimSize;
        
        /** create row with default empty field values. */
        TableDimsRow(void) : 
            modelId(0), 
            tableId(0), 
            name(""),
            pos(0),
            typeId(0),
            isTotal(false),
            dimSize(1)
        { }

        /** create row with supplied primary key field values. */
        TableDimsRow(int i_modelId, int i_tableId, const string & i_name) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            name(i_name),
            pos(0),
            typeId(0),
            isTotal(false),
            dimSize(1)
        { }

        ~TableDimsRow(void) throw() { }

        /** less comparator by primary key: model id, table id, dimension name. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, table id, dimension name. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** table_dims row equal comparator by model id, table id. */
        static bool modelIdTableIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** table_dims_txt table row. */
    struct TableDimsTxtRow : public IRowBase
    {
        /** model_id INT NOT NULL */
        int modelId;
        
        /** table_id INT NOT NULL */
        int tableId;
        
        /** dim_name VARCHAR(8) NOT NULL */
        string name;
        
        /** lang_id  INT NOT NULL */
        int langId;
        
        /** descr VARCHAR(255) NOT NULL */
        string descr;
        
        /** note     VARCHAR(32000) */
        string note;
        
        /** create row with default empty field values. */
        TableDimsTxtRow(void) : 
            modelId(0), 
            tableId(0), 
            name(""),
            langId(0),
            descr(""), 
            note("")
        { }

        /** create row with supplied primary key field values. */
        TableDimsTxtRow(int i_modelId, int i_tableId, const string & i_name, int i_langId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            name(i_name),
            langId(i_langId),
            descr(""), 
            note("")
        { }

        ~TableDimsTxtRow(void) throw() { }

        /** less comparator by primary key: model id, table id, dimension name, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, table id, dimension name, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** table_dims_txt table row and language name. */
    struct TableDimsTxtLangRow : public TableDimsTxtRow
    {
        /** language name */
        string langName;
    };

    /** table_acc table row. */
    struct TableAccRow : public IRowBase
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
        
        /** create row with default empty field values. */
        TableAccRow(void) : 
            modelId(0), 
            tableId(0), 
            accId(0), 
            name(""),
            expr("")
        { }

        /** create row with supplied primary key field values. */
        TableAccRow(int i_modelId, int i_tableId, int i_accid) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            accId(i_accid), 
            name(""),
            expr("")
        { }

        ~TableAccRow(void) throw() { }

        /** less comparator by primary key: model id, table id, accumulator id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, table id, accumulator id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** table_acc row equal comparator by model id. */
        static bool modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** table_acc row equal comparator by model id, table id. */
        static bool modelIdTableIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** table_acc_txt table row. */
    struct TableAccTxtRow : public IRowBase
    {
        /** model_id INT           NOT NULL */
        int modelId;
        
        /** TableID  INT           NOT NULL */
        int tableId;
        
        /** acc_id INT             NOT NULL */
        int accId;
        
        /** lang_id  INT           NOT NULL */
        int langId;
        
        /** descr VARCHAR(255) NOT NULL */
        string descr;
        
        /** note     VARCHAR(32000) */
        string note;

        /** create row with default empty field values. */
        TableAccTxtRow(void) : 
            modelId(0), 
            tableId(0), 
            accId(0),
            langId(0),
            descr(""), 
            note("")
        { }

        /** create row with supplied primary key field values. */
        TableAccTxtRow(int i_modelId, int i_tableId, int i_accId, int i_langId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            accId(i_accId),
            langId(i_langId),
            descr(""), 
            note("")
        { }

        ~TableAccTxtRow(void) throw() { }

        /** less comparator by primary key: model id, table id, accumulator id, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, table id, accumulator id, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** table_acc_txt table row and language name. */
    struct TableAccTxtLangRow : public TableAccTxtRow
    {
        /** language name */
        string langName;
    };

    /** table_unit table row. */
    struct TableUnitRow : public IRowBase
    {
        /** model_id      INT NOT NULL */
        int modelId;
        
        /** table_id      INT NOT NULL */
        int tableId;
        
        /** unit_id       INT NOT NULL */
        int unitId;
        
        /** unit_name     VARCHAR(8) NOT NULL */
        string name;
        
        /** unit_decimals INT NOT NULL */
        int decimals;
        
        /** unit_src      VARCHAR(255) NOT NULL */
        string src;

        /** unit_expr     VARCHAR(4000) NOT NULL */
        string expr;

        /** create row with default empty field values. */
        TableUnitRow(void) : 
            modelId(0), 
            tableId(0), 
            unitId(0), 
            name(""),
            decimals(0),
            src(""),
            expr("")
        { }

        /** create row with supplied primary key field values. */
        TableUnitRow(int i_modelId, int i_tableId, int i_unitId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            unitId(i_unitId), 
            name(""),
            decimals(0),
            src(""),
            expr("")
        { }

        ~TableUnitRow(void) throw() { }

        /** less comparator by primary key: model id, table id, unit id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, table id, unit id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** table_unit row equal comparator by model id. */
        static bool modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** table_unit row equal comparator by model id, table id. */
        static bool modelIdTableIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** table_unit_txt table row. */
    struct TableUnitTxtRow : public IRowBase
    {
        /** model_id INT NOT NULL */
        int modelId;
        
        /** TableID  INT NOT NULL */
        int tableId;
        
        /** unit_id  INT NOT NULL */
        int unitId;
        
        /** lang_id  INT NOT NULL */
        int langId;
        
        /** descr VARCHAR(255) NOT NULL */
        string descr;
        
        /** note     VARCHAR(32000) */
        string note;

        /** create row with default empty field values. */
        TableUnitTxtRow(void) : 
            modelId(0), 
            tableId(0), 
            unitId(0),
            langId(0),
            descr(""), 
            note("")
        { }

        /** create row with supplied primary key field values. */
        TableUnitTxtRow(int i_modelId, int i_tableId, int i_unitId, int i_langId) : 
            modelId(i_modelId), 
            tableId(i_tableId), 
            unitId(i_unitId),
            langId(i_langId),
            descr(""), 
            note("")
        { }

        ~TableUnitTxtRow(void) throw() { }

        /** less comparator by primary key: model id, table id, unit id, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, table id, unit id, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** table_unit_txt table row and language name. */
    struct TableUnitTxtLangRow : public TableUnitTxtRow
    {
        /** language name */
        string langName;
    };

    /** group_lst table row. */
    struct GroupLstRow : public IRowBase
    {
        /** model_id     INT         NOT NULL */
        int modelId;

        /** group_id     INT         NOT NULL */
        int groupId;

        /** is_parameter SMALLINT    NOT NULL */
        bool isParam;

        /** group_name   VARCHAR(64) NOT NULL */
        string name;

        /** is_hidden    SMALLINT    NOT NULL */
        bool isHidden;

        /** is_generated SMALLINT    NOT NULL */
        bool isGenerated;

        /** create row with default empty field values. */
        GroupLstRow(void) : 
            modelId(0), 
            groupId(0), 
            isParam(false),
            name(""),
            isHidden(false),
            isGenerated(false)
        { }

        /** create row with supplied primary key field values. */
        GroupLstRow(int i_modelId, int i_groupId) : 
            modelId(i_modelId), 
            groupId(i_groupId), 
            isParam(false),
            name(""),
            isHidden(false),
            isGenerated(false)
        { }

        ~GroupLstRow(void) throw() { }

        /** less comparator by primary key: model id, group id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, group id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by model id and is parameter group flag. */
        static bool modelIdIsParamEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** group_txt table row. */
    struct GroupTxtRow : public IRowBase
    {
        /** model_id INT          NOT NULL */
        int modelId;
        
        /** group_id INT          NOT NULL */
        int groupId;
        
        /** lang_id  INT          NOT NULL */
        int langId;
        
        /** descr VARCHAR(255) NOT NULL */
        string descr;
        
        /** note     VARCHAR(32000) */
        string note;
        
        /** create row with default empty field values. */
        GroupTxtRow(void) : 
            modelId(0), 
            groupId(0), 
            langId(0),
            descr(""), 
            note("")
        { }

        /** create row with supplied primary key field values. */
        GroupTxtRow(int i_modelId, int i_groupId, int i_langId) : 
            modelId(i_modelId), 
            groupId(i_groupId), 
            langId(i_langId),
            descr(""), 
            note("")
        { }

        ~GroupTxtRow(void) throw() { }

        /** less comparator by primary key: model id, group id, language id. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, group id, language id. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by language id. */
        static bool langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };

    /** group_txt table row and language name. */
    struct GroupTxtLangRow : public GroupTxtRow
    {
        /** language name */
        string langName;
    };

    /** group_pc table row. */
    struct GroupPcRow : public IRowBase
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

        /** create row with default empty field values. */
        GroupPcRow(void) : 
            modelId(0), 
            groupId(0), 
            childPos(0), 
            childGroupId(-1), 
            leafId(-1)
        { }

        /** create row with supplied primary key field values. */
        GroupPcRow(int i_modelId, int i_groupId, int i_childPos) : 
            modelId(i_modelId), 
            groupId(i_groupId), 
            childPos(i_childPos), 
            childGroupId(-1), 
            leafId(-1)
        { }

        ~GroupPcRow(void) throw() { }

        /** less comparator by primary key: model id, group id, child position. */
        static bool keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by primary key: model id, group id, child position. */
        static bool keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);

        /** equal comparator by model id. */
        static bool modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right);
    };
}

#endif  // DB_META_ROW_H
