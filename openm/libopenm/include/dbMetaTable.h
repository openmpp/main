/**
 * @file
 * OpenM++ data library: db tables for model metadata
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_META_TABLE_H
#define DB_META_TABLE_H

#include <algorithm>
#include <functional>
#include <mutex>
using namespace std;

#include "dbExec.h"
#include "libopenm/db/dbMetaRow.h"

namespace openm
{
    /** base class for model metadata db-tables */
    template<class TRow> struct IMetaTable
    {
    public:
        /** db table row comparator. */
        typedef function<bool(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)> RowComparator;

        virtual ~IMetaTable<TRow>(void) throw() { }

    protected:
        /** load table: return vector of selected rows sorted by primary key. */
        static IRowBaseVec load(const string & i_sqlSelect, IDbExec * i_dbExec, const IRowAdapter & i_adapter)
        {
            if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

            IRowBaseVec vec = i_dbExec->selectRowList(i_sqlSelect, i_adapter);
            stable_sort(vec.begin(), vec.end(), TRow::keyLess);
            return vec;
        }

        /** binary search row by primary key, return NULL if not found. */
        static const TRow * byKey(const IRowBaseUptr & i_row, const IRowBaseVec & i_rowVec)
        {
            IRowBaseVec::const_iterator rowIt = lower_bound(i_rowVec.begin(), i_rowVec.end(), i_row, TRow::keyLess);
            return dynamic_cast<TRow *>( (rowIt != i_rowVec.end() && TRow::keyEqual(*rowIt, i_row)) ? rowIt->get() : NULL );
        }

        /** return first table row or NULL if table is empty. */
        static const TRow * firstRow(const IRowBaseVec & i_rowVec)
        {
            return dynamic_cast<TRow *>(!i_rowVec.empty() ? i_rowVec[0].get() : NULL);
        }

        /** get list of loaded table rows. */
        static vector<TRow> rows(const IRowBaseVec & i_rowVec)
        {
            vector<TRow> vec;
            for (IRowBaseVec::const_iterator rowIt = i_rowVec.begin(); rowIt != i_rowVec.end(); ++rowIt) {
                vec.push_back( *dynamic_cast<TRow *>(rowIt->get()) );
            }
            return vec;
        }

        /** get list of rows by predicate: all rows where comparator to i_row return true. */
        static vector<TRow> findAll(const IRowBaseUptr & i_row, const IRowBaseVec & i_rowVec, RowComparator i_cmp)
        {
            vector<TRow> vec;
            for (IRowBaseVec::const_iterator rowIt = i_rowVec.begin(); rowIt != i_rowVec.end(); ++rowIt) {
                if (i_cmp(i_row, *rowIt)) vec.push_back( *dynamic_cast<TRow *>(rowIt->get()) );
            }
            return vec;
        }

        /** get first row by predicate or NULL if not found: first row where comparator to i_row return true. */
        static const TRow * findFirst(const IRowBaseUptr & i_row, const IRowBaseVec & i_rowVec, RowComparator i_cmp)
        {
            for (IRowBaseVec::const_iterator rowIt = i_rowVec.begin(); rowIt != i_rowVec.end(); ++rowIt) {
                if (i_cmp(i_row, *rowIt)) return dynamic_cast<TRow *>(rowIt->get());
            }
            return NULL;
        }
    };

    /** lang_lst table public interface. */
    struct ILangLstTable : public IMetaTable<LangLstRow>
    {
        virtual ~ILangLstTable() throw() = 0;

        /** create new table object and load table rows sorted by primary key: language id. */
        static ILangLstTable * create(IDbExec * i_dbExec);

        /** binary search row by primary key: language id, return NULL if not found. */
        virtual const LangLstRow * byKey(int i_langId) const = 0;

        /** return first table row or NULL if table is empty. */
        virtual const LangLstRow * firstRow(void) const = 0;

        /** get list of loaded table rows. */
        virtual vector<LangLstRow> rows(void) const = 0;

        /** get first row by language code or NULL if not found. */
        virtual const LangLstRow * byCode(const string & i_code) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ILangLstTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** lang_word table public interface. */
    struct ILangWordTable : public IMetaTable<LangWordRow>
    {
        virtual ~ILangWordTable() throw() = 0;

        /** create new table object and load table rows sorted by primary key: language id and word code. */
        static ILangWordTable * create(IDbExec * i_dbExec);

        /** binary search row by primary key: language id and word code, return NULL if not found. */
        virtual const LangWordRow * byKey(int i_langId, const string & i_code) const = 0;

        /** get list of loaded table rows. */
        virtual vector<LangWordRow> rows(void) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ILangWordTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** model_dic table public interface. */
    struct IModelDicTable : public IMetaTable<ModelDicRow>
    {
        virtual ~IModelDicTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id.  
        * 
        * if name and/or timestamp specified then select only rows where model_name = i_name and model_ts = i_timestamp
        */
        static IModelDicTable * create(IDbExec * i_dbExec, const char * i_name = NULL, const char * i_timestamp = NULL);

        /** binary search row by primary key: model id, return NULL if not found. */
        virtual const ModelDicRow * byKey(int i_modelId) const = 0;

        /** return first table row or NULL if table is empty. */
        virtual const ModelDicRow * firstRow(void) const = 0;

        /** get list of loaded table rows. */
        virtual vector<ModelDicRow> rows(void) const = 0;

        /** get first row by model name and timestamp or NULL if not found. */
        virtual const ModelDicRow * byNameTimeStamp(const string & i_name, const string & i_timestamp) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IModelDicTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** model_dic_txt table public interface. */
    struct IModelDicTxtTable : public IMetaTable<ModelDicTxtRow>
    {
        virtual ~IModelDicTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id and language id.  
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IModelDicTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id and language, return NULL if not found. */
        virtual const ModelDicTxtRow * byKey(int i_modelId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<ModelDicTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<ModelDicTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IModelDicTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** run_lst table public interface. */
    struct IRunLstTable : public IMetaTable<RunLstRow>
    {
        virtual ~IRunLstTable() throw() = 0;

        /** 
        * select table rows sorted by primary key: run id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static vector<RunLstRow> select(IDbExec * i_dbExec, int i_modelId = 0);

        /** select table row by primary key: run id. */
        static vector<RunLstRow> byKey(IDbExec * i_dbExec, int i_runId);
    };

    /** run_txt table public interface. */
    struct IRunTxtTable : public IMetaTable<RunTxtRow>
    {
        virtual ~IRunTxtTable() throw() = 0;

        /** 
        * select table rows and sorted by primary key: run id and language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static vector<RunTxtRow> select(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** select table row by primary key: run id and language id. */
        static vector<RunTxtRow> byKey(IDbExec * i_dbExec, int i_runId, int i_langId);
    };

    /** type_dic table public interface. */
    struct ITypeDicTable : public IMetaTable<TypeDicRow>
    {
        virtual ~ITypeDicTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id and type id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITypeDicTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id and type id, return NULL if not found. */
        virtual const TypeDicRow * byKey(int i_modelId, int i_typeId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TypeDicRow> rows(void) const = 0;

        /** get list of rows by model id. */
        virtual vector<TypeDicRow> byModelId(int i_modelId) const = 0;

        /** get first row by model id and type name or NULL if not found. */
        virtual const TypeDicRow * byModelIdName(int i_modelId, const string & i_name) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITypeDicTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** type_dic_txt table public interface. */
    struct ITypeDicTxtTable : public IMetaTable<TypeDicTxtRow>
    {
        virtual ~ITypeDicTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, type id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITypeDicTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, type id, language id; return NULL if not found. */
        virtual const TypeDicTxtRow * byKey(int i_modelId, int i_typeId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TypeDicTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<TypeDicTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITypeDicTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** type_enum_lst table public interface. */
    struct ITypeEnumLstTable : public IMetaTable<TypeEnumLstRow>
    {
        virtual ~ITypeEnumLstTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, type id, enum id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITypeEnumLstTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id, type id, enum id return NULL if not found. */
        virtual const TypeEnumLstRow * byKey(int i_modelId, int i_typeId, int i_enumId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TypeEnumLstRow> rows(void) const = 0;

        /** get list of rows by model id. */
        virtual vector<TypeEnumLstRow> byModelId(int i_modelId) const = 0;

        /** get list of rows by model id and type id. */
        virtual vector<TypeEnumLstRow> byModelIdTypeId(int i_modelId, int i_typeId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITypeEnumLstTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** type_enum_txt table public interface. */
    struct ITypeEnumTxtTable : public IMetaTable<TypeEnumTxtRow>
    {
        virtual ~ITypeEnumTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, type id, enum id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITypeEnumTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, type id, enum id, language id; return NULL if not found. */
        virtual const TypeEnumTxtRow * byKey(int i_modelId, int i_typeId, int i_enumId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TypeEnumTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<TypeEnumTxtRow> byLang(int i_langId) const = 0;

        /** get list of rows by model id and type id. */
        virtual vector<TypeEnumTxtRow> byModelIdTypeId(int i_modelId, int i_typeId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITypeEnumTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** parameter_dic table public interface. */
    struct IParamDicTable : public IMetaTable<ParamDicRow>
    {
        virtual ~IParamDicTable() throw() = 0;

        /**
        * create new table object and load table rows sorted by primary key: model id and parameter id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IParamDicTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id and parameter id, return NULL if not found. */
        virtual const ParamDicRow * byKey(int i_modelId, int i_paramId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<ParamDicRow> rows(void) const = 0;

        /** get list of rows by model id. */
        virtual vector<ParamDicRow> byModelId(int i_modelId) const = 0;

        /** get first row by model id and parameter name or NULL if not found. */
        virtual const ParamDicRow * byModelIdName(int i_modelId, const string & i_name) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamDicTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** parameter_dic_txt table public interface. */
    struct IParamDicTxtTable : public IMetaTable<ParamDicTxtRow>
    {
        virtual ~IParamDicTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, parameter id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IParamDicTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, parameter id, language id; return NULL if not found. */
        virtual const ParamDicTxtRow * byKey(int i_modelId, int i_paramId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<ParamDicTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<ParamDicTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamDicTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** parameter_run_txt table public interface. */
    struct IParamRunTxtTable : public IMetaTable<ParamRunTxtRow>
    {
        virtual ~IParamRunTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: run id, parameter id, language id.
        * 
        * if i_runId > 0 then select only rows where run_id = i_runId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IParamRunTxtTable * create(IDbExec * i_dbExec, int i_runId = 0, int i_langId = -1);

        /** binary search row by primary key: run id, parameter id, language id; return NULL if not found. */
        virtual const ParamRunTxtRow * byKey(int i_runId, int i_paramId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<ParamRunTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<ParamRunTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamRunTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** parameter_dims table public interface. */
    struct IParamDimsTable : public IMetaTable<ParamDimsRow>
    {
        virtual ~IParamDimsTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, parameter id, dimension id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IParamDimsTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id, parameter id, dimension id; return NULL if not found. */
        virtual const ParamDimsRow * byKey(int i_modelId, int i_paramId, int i_dimId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<ParamDimsRow> rows(void) const = 0;

        /** get list of rows by model id and parameter id. */
        virtual vector<ParamDimsRow> byModelIdParamId(int i_modelId, int i_paramId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamDimsTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** parameter_dims_txt table public interface. */
    struct IParamDimsTxtTable : public IMetaTable<ParamDimsTxtRow>
    {
        virtual ~IParamDimsTxtTable() throw() = 0;

        /**
        * create new table object and load table rows sorted by primary key: model id, parameter id, dimension id, language id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IParamDimsTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, parameter id, dimension id, language id; return NULL if not found. */
        virtual const ParamDimsTxtRow * byKey(int i_modelId, int i_paramId, int i_dimId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<ParamDimsTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<ParamDimsTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamDimsTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_dic table public interface. */
    struct ITableDicTable : public IMetaTable<TableDicRow>
    {
        virtual ~ITableDicTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id and table id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITableDicTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id and table id, return NULL if not found. */
        virtual const TableDicRow * byKey(int i_modelId, int i_tableId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TableDicRow> rows(void) const = 0;

        /** get list of rows by model id. */
        virtual vector<TableDicRow> byModelId(int i_modelId) const = 0;

        /** get first row by model id and table name or NULL if not found. */
        virtual const TableDicRow * byModelIdName(int i_modelId, const string & i_name) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableDicTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_dic_txt table public interface. */
    struct ITableDicTxtTable : public IMetaTable<TableDicTxtRow>
    {
        virtual ~ITableDicTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, table id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITableDicTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, table id, language id; return NULL if not found. */
        virtual const TableDicTxtRow * byKey(int i_modelId, int i_tableId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TableDicTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<TableDicTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableDicTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_dims table public interface. */
    struct ITableDimsTable : public IMetaTable<TableDimsRow>
    {
        virtual ~ITableDimsTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, table id, dimension id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITableDimsTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id, table id, dimension id; return NULL if not found. */
        virtual const TableDimsRow * byKey(int i_modelId, int i_tableId, int i_dimId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TableDimsRow> rows(void) const = 0;

        /** get list of rows by model id and table id. */
        virtual vector<TableDimsRow> byModelIdTableId(int i_modelId, int i_tableId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableDimsTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_dims_txt table public interface. */
    struct ITableDimsTxtTable : public IMetaTable<TableDimsTxtRow>
    {
        virtual ~ITableDimsTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, table id, dimension id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITableDimsTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, table id, dimension id, language id; return NULL if not found. */
        virtual const TableDimsTxtRow * byKey(int i_modelId, int i_tableId, int i_dimId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TableDimsTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<TableDimsTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableDimsTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_acc table public interface. */
    struct ITableAccTable : public IMetaTable<TableAccRow>
    {
        virtual ~ITableAccTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, table id, accumulator id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITableAccTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id, table id, accumulator id; return NULL if not found. */
        virtual const TableAccRow * byKey(int i_modelId, int i_tableId, int i_accId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TableAccRow> rows(void) const = 0;

        /** get list of rows by model id. */
        virtual vector<TableAccRow> byModelId(int i_modelId) const = 0;

        /** get list of rows by model id and table id. */
        virtual vector<TableAccRow> byModelIdTableId(int i_modelId, int i_tableId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableAccTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_acc_txt table public interface. */
    struct ITableAccTxtTable : public IMetaTable<TableAccTxtRow>
    {
        virtual ~ITableAccTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, table id, accumulator id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITableAccTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, table id, accumulator id, language id; return NULL if not found. */
        virtual const TableAccTxtRow * byKey(int i_modelId, int i_tableId, int i_accId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TableAccTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<TableAccTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableAccTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_expr table public interface. */
    struct ITableExprTable : public IMetaTable<TableExprRow>
    {
        virtual ~ITableExprTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, table id, expr id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITableExprTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id, table id, expr id; return NULL if not found. */
        virtual const TableExprRow * byKey(int i_modelId, int i_tableId, int i_exprId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TableExprRow> rows(void) const = 0;

        /** get list of rows by model id. */
        virtual vector<TableExprRow> byModelId(int i_modelId) const = 0;

        /** get list of rows by model id and table id. */
        virtual vector<TableExprRow> byModelIdTableId(int i_modelId, int i_tableId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableExprTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_expr_txt table public interface. */
    struct ITableExprTxtTable : public IMetaTable<TableExprTxtRow>
    {
        virtual ~ITableExprTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, table id, expr id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITableExprTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, table id, expr id, language id; return NULL if not found. */
        virtual const TableExprTxtRow * byKey(int i_modelId, int i_tableId, int i_exprId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<TableExprTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<TableExprTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableExprTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** group_lst table public interface. */
    struct IGroupLstTable : public IMetaTable<GroupLstRow>
    {
        virtual ~IGroupLstTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id and group id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IGroupLstTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id and group id, return NULL if not found. */
        virtual const GroupLstRow * byKey(int i_modelId, int i_groupId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<GroupLstRow> rows(void) const = 0;

        /** get list of rows by model id and is parameter group flag. */
        virtual vector<GroupLstRow> byModelId(int i_modelId, bool i_isParam) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IGroupLstTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** group_txt table public interface. */
    struct IGroupTxtTable : public IMetaTable<GroupTxtRow>
    {
        virtual ~IGroupTxtTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, group id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IGroupTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, group id, language id; return NULL if not found. */
        virtual const GroupTxtRow * byKey(int i_modelId, int i_groupId, int i_langId) const = 0;

        /** get list of loaded table rows. */
        virtual vector<GroupTxtRow> rows(void) const = 0;

        /** get list of rows by language. */
        virtual vector<GroupTxtRow> byLang(int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IGroupTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** group_pc table public interface. */
    struct IGroupPcTable : public IMetaTable<GroupPcRow>
    {
        virtual ~IGroupPcTable() throw() = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, group id, child position.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IGroupPcTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id, group id, child position; return NULL if not found. */
        virtual const GroupPcRow * byKey(int i_modelId, int i_groupId, int i_chidPos) const = 0;

        /** get list of loaded table rows. */
        virtual vector<GroupPcRow> rows(void) const = 0;

        /** get list of rows by model id. */
        virtual vector<GroupPcRow> byModelId(int i_modelId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IGroupPcTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** profile_lst table public interface. */
    struct IProfileLstTable : public IMetaTable<ProfileLstRow>
    {
        virtual ~IProfileLstTable() throw() = 0;

        /** create new table object and load table rows sorted by primary key: profile name. */
        static IProfileLstTable * create(IDbExec * i_dbExec);

        /** binary search row by primary key: profile name, return NULL if not found. */
        virtual const ProfileLstRow * byKey(const string & i_name) const = 0;

        /** get list of loaded table rows. */
        virtual vector<ProfileLstRow> rows(void) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IProfileLstTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** profile_option table public interface. */
    struct IProfileOptionTable : public IMetaTable<ProfileOptionRow>
    {
        virtual ~IProfileOptionTable() throw() = 0;

        /**
        * create new table object and load table rows sorted by primary key: profile name and option key.
        *
        * if i_name != "" then select only rows where profile_name = i_name
        */
        static IProfileOptionTable * create(IDbExec * i_dbExec, const string & i_name = "");

        /** binary search row by primary key: profile name and option key, return NULL if not found. */
        virtual const ProfileOptionRow * byKey(const string & i_name, const string & i_key) const = 0;

        /** get list of loaded table rows. */
        virtual vector<ProfileOptionRow> rows(void) const = 0;

        /** get list of rows by profile name. */
        virtual vector<ProfileOptionRow> byName(const string & i_name) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IProfileOptionTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** run_option table public interface. */
    struct IRunOptionTable : public IMetaTable<RunOptionRow>
    {
        virtual ~IRunOptionTable() throw() = 0;

        /**
        * create new table object and load table rows sorted by primary key: run id and option key.
        *
        * if i_runId > 0 then select only rows where run_id = i_runId
        */
        static IRunOptionTable * create(IDbExec * i_dbExec, int i_runId = 0);

        /** binary search row by primary key: run id and option key, return NULL if not found. */
        virtual const RunOptionRow * byKey(int i_runId, const string & i_key) const = 0;

        /** get list of loaded table rows. */
        virtual vector<RunOptionRow> rows(void) const = 0;

        /** get list of rows by run id. */
        virtual vector<RunOptionRow> byRunId(int i_runId) const = 0;

        /** return true if primary key (run id, option key) found. */
        virtual bool isExist(int i_runId, const char * i_key) const throw() = 0;

        /** return true if option key found. */
        virtual bool isExist(const char * i_key) const throw() = 0;

        /** return string value by primary key (run id, option key) or default value if not found. */
        virtual string strValue(int i_runId, const char * i_key, const string & i_default = "") const throw() = 0;

        /** return string value by option key (first found) or default value if not found. */
        virtual string strValue(const char * i_key, const string & i_default = "") const throw() = 0;

        /** return boolean value by primary key (run id, option key) or false if not found or value not "yes", "1", "true". */
        virtual bool boolValue(int i_runId, const char * i_key) const throw() = 0;

        /** return booleanvalue by option key (first found) or false if not found or value not "yes", "1", "true". */
        virtual bool boolValue(const char * i_key) const throw() = 0;

        /** return long value by primary key (run id, option key) or default if not found or can not be converted to long. */
        virtual long long longValue(int i_runId, const char * i_key, long long i_default) const throw() = 0;

        /** return long value by option key (first found) or default if not found or can not be converted to long. */
        virtual long long longValue(const char * i_key, long long i_default) const throw() = 0;

        /** return double value by primary key (run id, option key) or default if not found or can not be converted to double. */
        virtual double doubleValue(int i_runId, const char * i_key, double i_default) const throw() = 0;

        /** return double value by option key (first found) or default if not found or can not be converted to double. */
        virtual double doubleValue(const char * i_key, double i_default) const throw() = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IRunOptionTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** workset_lst table public interface. */
    struct IWorksetLstTable : public IMetaTable<WorksetLstRow>
    {
        virtual ~IWorksetLstTable() throw() = 0;

        /**
        * select table rows sorted by primary key: set id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static vector<WorksetLstRow> select(IDbExec * i_dbExec, int i_modelId = 0);

        /** select table row by primary key: set id. */
        static vector<WorksetLstRow> byKey(IDbExec * i_dbExec, int i_setId);
    };

    /** workset_txt table public interface. */
    struct IWorksetTxtTable : public IMetaTable<WorksetTxtRow>
    {
        virtual ~IWorksetTxtTable() throw() = 0;

        /**
        * select table rows and sorted by primary key: set id and language id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static vector<WorksetTxtRow> select(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** select table row by primary key: set id and language id. */
        static vector<WorksetTxtRow> byKey(IDbExec * i_dbExec, int i_setId, int i_langId);
    };

    /** workset_parameter table public interface. */
    struct IWorksetParamTable : public IMetaTable<WorksetParamRow>
    {
        virtual ~IWorksetParamTable() throw() = 0;

        /**
        * select table rows sorted by primary key: set id and parameter id.
        *
        * if i_setId > 0 then select only rows where set_id = i_setId
        */
        static vector<WorksetParamRow> select(IDbExec * i_dbExec, int i_setId = 0);

        /** select table row by primary key: set id and parameter id. */
        static vector<WorksetParamRow> byKey(IDbExec * i_dbExec, int i_setId, int i_paramId);
    };

    /** workset_parameter_txt table public interface. */
    struct IWorksetParamTxtTable : public IMetaTable<WorksetParamTxtRow>
    {
        virtual ~IWorksetParamTxtTable() throw() = 0;

        /**
        * select table rows and sorted by primary key: set id, parameter id, language id.
        *
        * if i_setId > 0 then select only rows where set_id = i_setId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static vector<WorksetParamTxtRow> select(IDbExec * i_dbExec, int i_setId = 0, int i_langId = -1);

        /** select table row by primary key: set id, parameter id, language id. */
        static vector<WorksetParamTxtRow> byKey(IDbExec * i_dbExec, int i_setId, int i_paramId, int i_langId);
    };


    /** task_lst table public interface. */
    struct ITaskLstTable : public IMetaTable<TaskLstRow>
    {
        virtual ~ITaskLstTable() throw() = 0;

        /**
        * select table rows sorted by primary key: task id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static vector<TaskLstRow> select(IDbExec * i_dbExec, int i_modelId = 0);

        /** select table row by primary key: task id. */
        static vector<TaskLstRow> byKey(IDbExec * i_dbExec, int i_taskId);
    };

    /** task_txt table public interface. */
    struct ITaskTxtTable : public IMetaTable<TaskTxtRow>
    {
        virtual ~ITaskTxtTable() throw() = 0;

        /**
        * select table rows and sorted by primary key: task id and language id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static vector<TaskTxtRow> select(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** select table row by primary key: task id and language id. */
        static vector<TaskTxtRow> byKey(IDbExec * i_dbExec, int i_taskId, int i_langId);
    };

    /** task_set table public interface. */
    struct ITaskSetTable : public IMetaTable<TaskSetRow>
    {
        virtual ~ITaskSetTable() throw() = 0;

        /**
        * select table rows sorted by primary key: task id and set id.
        *
        * if i_taskId > 0 then select only rows where task_id = i_taskId
        */
        static vector<TaskSetRow> select(IDbExec * i_dbExec, int i_taskId = 0);

        /** select table row by primary key: task id and set id. */
        static vector<TaskSetRow> byKey(IDbExec * i_dbExec, int i_taskId, int i_setId);
    };

    /** task_log table public interface. */
    struct ITaskLogTable : public IMetaTable<TaskLogRow>
    {
        virtual ~ITaskLogTable() throw() = 0;

        /**
        * select table rows sorted by primary key: task log id.
        *
        * if i_taskId > 0 then select only rows where task_id = i_taskId
        */
        static vector<TaskLogRow> select(IDbExec * i_dbExec, int i_taskId = 0);

        /** select table row by primary key: task log id. */
        static vector<TaskLogRow> byKey(IDbExec * i_dbExec, int i_taskLogId);
    };

    /** task_run table public interface. */
    struct ITaskRunTable : public IMetaTable<TaskRunRow>
    {
        virtual ~ITaskRunTable() throw() = 0;

        /**
        * select table rows sorted by primary key: task id, set id, run id.
        *
        * if i_taskLogId > 0 then select only rows where task_log_id = i_taskLogId
        */
        static vector<TaskRunRow> select(IDbExec * i_dbExec, int i_taskLogId = 0);

        /** select table row by primary key: task id, set id, run id. */
        static vector<TaskRunRow> byKey(IDbExec * i_dbExec, int i_taskId, int i_setId, int i_runId);
    };
}

#endif  // DB_META_TABLE_H
