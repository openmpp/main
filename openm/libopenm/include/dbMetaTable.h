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
    /** base class for metadata db-tables */
    template<class TRow> struct IMetaTable
    {
    public:
        virtual ~IMetaTable<TRow>(void) noexcept { }

    protected:
        /** get list of table rows. */
        static vector<TRow> rows(const IRowBaseVec & i_rowVec)
        {
            vector<TRow> vec;
            for (IRowBaseVec::const_iterator rowIt = i_rowVec.begin(); rowIt != i_rowVec.end(); ++rowIt) {
                vec.push_back( *dynamic_cast<TRow *>(rowIt->get()) );
            }
            return vec;
        }
    };

    /** base class for preloaded metadata db-tables */
    template<class TRow> struct IMetaLoadedTable : public IMetaTable<TRow>
    {
        /** db table row comparator. */
        typedef function<bool(const TRow & i_dbRow)> RowEqual;

        virtual ~IMetaLoadedTable<TRow>(void) noexcept { }

        /** get const reference to list of all table rows. */
        virtual const IRowBaseVec & rowsCRef(void) const = 0;

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;

        /** return number of rows. */
        IRowBaseVec::difference_type rowCount(void) const { return rowsCRef().size(); }

        /** get list of loaded table rows. */
        vector<TRow> rows(void) const
        {
            vector<TRow> vec;
            for (IRowBaseVec::const_iterator rowIt = rowsCRef().cbegin(); rowIt != rowsCRef().cend(); ++rowIt) {
                vec.push_back(*dynamic_cast<TRow *>(rowIt->get()));
            }
            return vec;
        }

        /** return first table row or NULL if table is empty. */
        const TRow * firstRow(void) const
        {
            return dynamic_cast<TRow *>(!rowsCRef().empty() ? rowsCRef()[0].get() : nullptr);
        }

        /** get list of rows by predicate: all rows where comparator to i_row return true. */
        vector<TRow> findAll(RowEqual i_cmp) const
        {
            vector<TRow> vec;
            for (IRowBaseVec::const_iterator rowIt = rowsCRef().cbegin(); rowIt != rowsCRef().cend(); ++rowIt) {
                if (i_cmp(*dynamic_cast<TRow *>(rowIt->get()))) vec.push_back(*dynamic_cast<TRow *>(rowIt->get()));
            }
            return vec;
        }

        /** get first row by predicate or NULL if not found: first row where comparator to i_row return true. */
        const TRow * findFirst(RowEqual i_cmp) const { return byIndex(indexOf(i_cmp)); }

        /** return row value by index or NULL if out of range. */
        const TRow * byIndex(IRowBaseVec::difference_type i_index) const 
        {
            return (0 <= i_index && i_index < rowCount()) ? dynamic_cast<TRow *>(rowsCRef()[i_index].get()) : nullptr;
        }

        /** get first row by predicate or -1 if not found: first row where comparator to i_row return true. */
        IRowBaseVec::difference_type indexOf(RowEqual i_cmp, IRowBaseVec::difference_type i_startPos = 0) const
        {
            if (i_startPos >= 0) {
                for (IRowBaseVec::size_type pos = i_startPos; pos < rowsCRef().size(); pos++) {
                    if (i_cmp(*dynamic_cast<TRow *>(rowsCRef()[pos].get()))) return pos;
                }
            }
            return -1;
        }

        /** count rows by predicate: where comparator to i_row return true. */
        IRowBaseVec::size_type countOf(RowEqual i_cmp) const
        {
            IRowBaseVec::size_type nCount = 0;
            for (IRowBaseVec::const_iterator rowIt = rowsCRef().cbegin(); rowIt != rowsCRef().cend(); ++rowIt) {
                if (i_cmp(*dynamic_cast<TRow *>(rowIt->get()))) nCount++;
            }
            return nCount;
        }

    protected:
        /** load table: return vector of selected rows sorted by primary key. */
        static IRowBaseVec load(const string & i_sqlSelect, IDbExec * i_dbExec, const IRowAdapter & i_adapter)
        {
            if (i_dbExec == nullptr) throw DbException("invalid (NULL) database connection");

            IRowBaseVec vec = i_dbExec->selectRowVector(i_sqlSelect, i_adapter);
            stable_sort(vec.begin(), vec.end(), TRow::keyLess);
            return vec;
        }

        /** binary search row by primary key, return NULL if not found. */
        const TRow * findKey(const IRowBaseUptr & i_row) const
        {
            IRowBaseVec::const_iterator rowIt = lower_bound(rowsCRef().cbegin(), rowsCRef().cend(), i_row, TRow::keyLess);
            return dynamic_cast<TRow *>((rowIt != rowsCRef().cend() && TRow::keyEqual(*rowIt, i_row)) ? rowIt->get() : nullptr);
        }
    };

    /** lang_lst table public interface. */
    struct ILangLstTable : public IMetaLoadedTable<LangLstRow>
    {
        virtual ~ILangLstTable() noexcept = 0;

        /** create new table object and load table rows sorted by primary key: language id. */
        static ILangLstTable * create(IDbExec * i_dbExec);

        /** binary search row by primary key: language id, return NULL if not found. */
        virtual const LangLstRow * byKey(int i_langId) const = 0;

        /** get first row by language code or NULL if not found. */
        virtual const LangLstRow * byCode(const string & i_code) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ILangLstTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** lang_word table public interface. */
    struct ILangWordTable : public IMetaLoadedTable<LangWordRow>
    {
        virtual ~ILangWordTable() noexcept = 0;

        /** create new table object and load table rows sorted by primary key: language id and word code.
        *
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ILangWordTable * create(IDbExec * i_dbExec, int i_langId = -1);

        /** binary search row by primary key: language id and word code, return NULL if not found. */
        virtual const LangWordRow * byKey(int i_langId, const string & i_code) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ILangWordTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** model_dic table public interface. */
    struct IModelDicTable : public IMetaLoadedTable<ModelDicRow>
    {
        virtual ~IModelDicTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by primary key: model id.  
        * 
        * if name and/or digest specified then select only rows where model_name = i_name and model_digest = i_digest
        */
        static IModelDicTable * create(IDbExec * i_dbExec, const char * i_name = nullptr, const char * i_digest = nullptr);

        /** create new table object and load table row by primary key: model id. */
        static IModelDicTable * create(IDbExec * i_dbExec, int i_modelId);

        /** binary search row by primary key: model id, return NULL if not found. */
        virtual const ModelDicRow * byKey(int i_modelId) const = 0;

        /** get find first row by model name and digest or NULL if not found. */
        virtual const ModelDicRow * byNameDigest(const string & i_name, const string & i_digest) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IModelDicTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** model_dic_txt table public interface. */
    struct IModelDicTxtTable : public IMetaLoadedTable<ModelDicTxtRow>
    {
        virtual ~IModelDicTxtTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id and language id.  
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IModelDicTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id and language, return NULL if not found. */
        virtual const ModelDicTxtRow * byKey(int i_modelId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IModelDicTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** model_word table public interface. */
    struct IModelWordTable : public IMetaLoadedTable<ModelWordRow>
    {
        virtual ~IModelWordTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, language id, word code.  
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IModelWordTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, language and code, return NULL if not found. */
        virtual const ModelWordRow * byKey(int i_modelId, int i_langId, const string & i_code) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IModelWordTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** type_dic table public interface. */
    struct ITypeDicTable : public IMetaLoadedTable<TypeDicRow>
    {
        virtual ~ITypeDicTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by primary key: type_hid.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITypeDicTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id, model type id, return NULL if not found. */
        virtual const TypeDicRow * byKey(int i_modelId, int i_typeId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITypeDicTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** type_dic_txt table public interface. */
    struct ITypeDicTxtTable : public IMetaLoadedTable<TypeDicTxtRow>
    {
        virtual ~ITypeDicTxtTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model type id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITypeDicTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model type id, language id; return NULL if not found. */
        virtual const TypeDicTxtRow * byKey(int i_modelId, int i_typeId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITypeDicTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** type_enum_lst table public interface. */
    struct ITypeEnumLstTable : public IMetaLoadedTable<TypeEnumLstRow>
    {
        virtual ~ITypeEnumLstTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model type id, enum id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITypeEnumLstTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id, model type id, enum id return NULL if not found. */
        virtual const TypeEnumLstRow * byKey(int i_modelId, int i_typeId, int i_enumId) const = 0;

        /** get list of rows by model id. */
        virtual vector<TypeEnumLstRow> byModelId(int i_modelId) const = 0;

        /** get list of rows by model id, model type id. */
        virtual vector<TypeEnumLstRow> byModelIdTypeId(int i_modelId, int i_typeId) const = 0;

        /** find row by unique key: model id, model type id, enum name. */
        virtual const TypeEnumLstRow * byName(int i_modelId, int i_typeId, const char * i_name) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITypeEnumLstTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** type_enum_txt table public interface. */
    struct ITypeEnumTxtTable : public IMetaLoadedTable<TypeEnumTxtRow>
    {
        virtual ~ITypeEnumTxtTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model type id, enum id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITypeEnumTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model type id, enum id, language id; return NULL if not found. */
        virtual const TypeEnumTxtRow * byKey(int i_modelId, int i_typeId, int i_enumId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITypeEnumTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** parameter_dic table public interface. */
    struct IParamDicTable : public IMetaLoadedTable<ParamDicRow>
    {
        virtual ~IParamDicTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by unique key: model id and model parameter id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IParamDicTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id and model parameter id, return NULL if not found. */
        virtual const ParamDicRow * byKey(int i_modelId, int i_paramId) const = 0;

        /** get list of rows by model id. */
        virtual vector<ParamDicRow> byModelId(int i_modelId) const = 0;

        /** get first row by model id and parameter name or NULL if not found. */
        virtual const ParamDicRow * byModelIdName(int i_modelId, const string & i_name) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamDicTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** model_parameter_import table public interface. */
    struct IParamImportTable : public IMetaLoadedTable<ParamImportRow>
    {
        virtual ~IParamImportTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by unique key: model id, model parameter id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IParamImportTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id, model parameter id, return NULL if not found. */
        virtual const ParamImportRow * byKey(int i_modelId, int i_paramId) const = 0;

        /** get list of rows by model id. */
        virtual vector<ParamImportRow> byModelId(int i_modelId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamImportTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** parameter_dic_txt table public interface. */
    struct IParamDicTxtTable : public IMetaLoadedTable<ParamDicTxtRow>
    {
        virtual ~IParamDicTxtTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model parameter id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IParamDicTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model parameter id, language id; return NULL if not found. */
        virtual const ParamDicTxtRow * byKey(int i_modelId, int i_paramId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamDicTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** parameter_dims table public interface. */
    struct IParamDimsTable : public IMetaLoadedTable<ParamDimsRow>
    {
        virtual ~IParamDimsTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model parameter id, dimension id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IParamDimsTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id, model parameter id, dimension id; return NULL if not found. */
        virtual const ParamDimsRow * byKey(int i_modelId, int i_paramId, int i_dimId) const = 0;

        /** get list of rows by model id and parameter id. */
        virtual vector<ParamDimsRow> byModelIdParamId(int i_modelId, int i_paramId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamDimsTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** parameter_dims_txt table public interface. */
    struct IParamDimsTxtTable : public IMetaLoadedTable<ParamDimsTxtRow>
    {
        virtual ~IParamDimsTxtTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by unique key: model id, model parameter id, dimension id, language id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IParamDimsTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model parameter id, dimension id, language id; return NULL if not found. */
        virtual const ParamDimsTxtRow * byKey(int i_modelId, int i_paramId, int i_dimId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IParamDimsTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_dic table public interface. */
    struct ITableDicTable : public IMetaLoadedTable<TableDicRow>
    {
        virtual ~ITableDicTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model table id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITableDicTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id, model table id, return NULL if not found. */
        virtual const TableDicRow * byKey(int i_modelId, int i_tableId) const = 0;

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
    struct ITableDicTxtTable : public IMetaLoadedTable<TableDicTxtRow>
    {
        virtual ~ITableDicTxtTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model table id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITableDicTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model table id, language id; return NULL if not found. */
        virtual const TableDicTxtRow * byKey(int i_modelId, int i_tableId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableDicTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_dims table public interface. */
    struct ITableDimsTable : public IMetaLoadedTable<TableDimsRow>
    {
        virtual ~ITableDimsTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model table id, dimension id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITableDimsTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id, model table id, dimension id; return NULL if not found. */
        virtual const TableDimsRow * byKey(int i_modelId, int i_tableId, int i_dimId) const = 0;

        /** get list of rows by model id and table id. */
        virtual vector<TableDimsRow> byModelIdTableId(int i_modelId, int i_tableId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableDimsTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_dims_txt table public interface. */
    struct ITableDimsTxtTable : public IMetaLoadedTable<TableDimsTxtRow>
    {
        virtual ~ITableDimsTxtTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model table id, dimension id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITableDimsTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model table id, dimension id, language id; return NULL if not found. */
        virtual const TableDimsTxtRow * byKey(int i_modelId, int i_tableId, int i_dimId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableDimsTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_acc table public interface. */
    struct ITableAccTable : public IMetaLoadedTable<TableAccRow>
    {
        virtual ~ITableAccTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model table id, accumulator id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_isIncludeDerived then select all acumulators else exclude derived
        */
        static ITableAccTable * create(IDbExec * i_dbExec, int i_modelId = 0, bool i_isIncludeDerived = false);

        /** binary search row by unique key: model id, model table id, accumulator id; return NULL if not found. */
        virtual const TableAccRow * byKey(int i_modelId, int i_tableId, int i_accId) const = 0;

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
    struct ITableAccTxtTable : public IMetaLoadedTable<TableAccTxtRow>
    {
        virtual ~ITableAccTxtTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model table id, accumulator id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITableAccTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model table id, accumulator id, language id; return NULL if not found. */
        virtual const TableAccTxtRow * byKey(int i_modelId, int i_tableId, int i_accId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableAccTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** table_expr table public interface. */
    struct ITableExprTable : public IMetaLoadedTable<TableExprRow>
    {
        virtual ~ITableExprTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model table id, expr id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static ITableExprTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id, model table id, expr id; return NULL if not found. */
        virtual const TableExprRow * byKey(int i_modelId, int i_tableId, int i_exprId) const = 0;

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
    struct ITableExprTxtTable : public IMetaLoadedTable<TableExprTxtRow>
    {
        virtual ~ITableExprTxtTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by unique key: model id, model table id, expr id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static ITableExprTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model table id, expr id, language id; return NULL if not found. */
        virtual const TableExprTxtRow * byKey(int i_modelId, int i_tableId, int i_exprId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static ITableExprTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** entity_dic table public interface. */
    struct IEntityDicTable : public IMetaLoadedTable<EntityDicRow>
    {
        virtual ~IEntityDicTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by unique key: model id, model entity id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IEntityDicTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id, model entity id, return NULL if not found. */
        virtual const EntityDicRow * byKey(int i_modelId, int i_entityId) const = 0;

        /** get list of rows by model id. */
        virtual vector<EntityDicRow> byModelId(int i_modelId) const = 0;

        /** get first row by model id and entity name or NULL if not found. */
        virtual const EntityDicRow * byModelIdName(int i_modelId, const string & i_name) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IEntityDicTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** entity_dic_txt table public interface. */
    struct IEntityDicTxtTable : public IMetaLoadedTable<EntityDicTxtRow>
    {
        virtual ~IEntityDicTxtTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by unique key: model id, model entity id, language id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IEntityDicTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model entity id, language id; return NULL if not found. */
        virtual const EntityDicTxtRow * byKey(int i_modelId, int i_entityId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IEntityDicTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** entity_attr table public interface. */
    struct IEntityAttrTable : public IMetaLoadedTable<EntityAttrRow>
    {
        virtual ~IEntityAttrTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by unique key: model id, model entity id, attribute id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IEntityAttrTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by unique key: model id, model entity id, attribute id; return NULL if not found. */
        virtual const EntityAttrRow * byKey(int i_modelId, int i_entityId, int i_attrId) const = 0;

        /** get list of rows by model id and entity id. */
        virtual vector<EntityAttrRow> byModelIdEntityId(int i_modelId, int i_entityId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IEntityAttrTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** entity_attr_txt table public interface. */
    struct IEntityAttrTxtTable : public IMetaLoadedTable<EntityAttrTxtRow>
    {
        virtual ~IEntityAttrTxtTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by unique key: model id, model entity id, attribute id, language id.
        *
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IEntityAttrTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by unique key: model id, model entity id, attribute id, language id; return NULL if not found. */
        virtual const EntityAttrTxtRow * byKey(int i_modelId, int i_entityId, int i_attrId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IEntityAttrTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** group_lst table public interface. */
    struct IGroupLstTable : public IMetaLoadedTable<GroupLstRow>
    {
        virtual ~IGroupLstTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id and group id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IGroupLstTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id and group id, return NULL if not found. */
        virtual const GroupLstRow * byKey(int i_modelId, int i_groupId) const = 0;

        /** get list of rows by model id and is parameter group flag. */
        virtual vector<GroupLstRow> byModelId(int i_modelId, bool i_isParam) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IGroupLstTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** group_txt table public interface. */
    struct IGroupTxtTable : public IMetaLoadedTable<GroupTxtRow>
    {
        virtual ~IGroupTxtTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, group id, language id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static IGroupTxtTable * create(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);

        /** binary search row by primary key: model id, group id, language id; return NULL if not found. */
        virtual const GroupTxtRow * byKey(int i_modelId, int i_groupId, int i_langId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IGroupTxtTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** group_pc table public interface. */
    struct IGroupPcTable : public IMetaLoadedTable<GroupPcRow>
    {
        virtual ~IGroupPcTable() noexcept = 0;

        /** 
        * create new table object and load table rows sorted by primary key: model id, group id, child position.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static IGroupPcTable * create(IDbExec * i_dbExec, int i_modelId = 0);

        /** binary search row by primary key: model id, group id, child position; return NULL if not found. */
        virtual const GroupPcRow * byKey(int i_modelId, int i_groupId, int i_chidPos) const = 0;

        /** get list of rows by model id. */
        virtual vector<GroupPcRow> byModelId(int i_modelId) const = 0;

        /** get list of parameter id's or table id's for the group: list of bottom level group members. */
        virtual vector<int> groupLeafs(int i_modelId, int i_groupId) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IGroupPcTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** profile_lst table public interface. */
    struct IProfileLstTable : public IMetaLoadedTable<ProfileLstRow>
    {
        virtual ~IProfileLstTable() noexcept = 0;

        /** create new table object and load table rows sorted by primary key: profile name. */
        static IProfileLstTable * create(IDbExec * i_dbExec);

        /** binary search row by primary key: profile name, return NULL if not found. */
        virtual const ProfileLstRow * byKey(const string & i_name) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IProfileLstTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** profile_option table public interface. */
    struct IProfileOptionTable : public IMetaLoadedTable<ProfileOptionRow>
    {
        virtual ~IProfileOptionTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by primary key: profile name and option key.
        *
        * if i_name != "" then select only rows where profile_name = i_name
        */
        static IProfileOptionTable * create(IDbExec * i_dbExec, const string & i_name = "");

        /** binary search row by primary key: profile name and option key, return NULL if not found. */
        virtual const ProfileOptionRow * byKey(const string & i_name, const string & i_key) const = 0;

        /** get list of rows by profile name. */
        virtual vector<ProfileOptionRow> byName(const string & i_name) const = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IProfileOptionTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** run_lst table public interface. */
    struct IRunLstTable : public IMetaTable<RunLstRow>
    {
        virtual ~IRunLstTable() noexcept = 0;

        /** 
        * select table rows sorted by primary key: run id.
        * 
        * if i_modelId > 0 then select only rows where model_id = i_modelId
        */
        static vector<RunLstRow> select(IDbExec * i_dbExec, int i_modelId = 0);

        /** select table row by primary key: run id. */
        static vector<RunLstRow> byKey(IDbExec * i_dbExec, int i_runId);

        /** calculate run value digest, including run parameters and output table values digest */
        static string digestRunValue(IDbExec * i_dbExec, int i_modelId, int i_runId);

        /** calculate run metadata digest: model digest, run name, sub count, created date-time, run stamp */
        static string digestRunMeta(const string & i_modelDigest, const RunLstRow & i_runRow);
    };

    /** run_option table public interface. */
    struct IRunOptionTable : public IMetaLoadedTable<RunOptionRow>
    {
        virtual ~IRunOptionTable() noexcept = 0;

        /**
        * create new table object and load table rows sorted by primary key: run id and option key.
        *
        * if i_runId > 0 then select only rows where run_id = i_runId
        */
        static IRunOptionTable * create(IDbExec * i_dbExec, int i_runId = 0);

        /** binary search row by primary key: run id and option key, return NULL if not found. */
        virtual const RunOptionRow * byKey(int i_runId, const string & i_key) const = 0;

        /** return true if primary key (run id, option key) found. */
        virtual bool isExist(int i_runId, const char * i_key) const noexcept = 0;

        /** return string value by primary key (run id, option key) or default value if not found. */
        virtual string strValue(int i_runId, const char * i_key, const string & i_default = "") const noexcept = 0;

        /** return boolean value by primary key (run id, option key) or false if not found or value not "yes", "1", "true" or empty "" string. */
        virtual bool boolValue(int i_runId, const char * i_key) const noexcept = 0;

        /** search for boolean value by primary key (run id, option key) and return one of: \n
        * return  1 if key found and value is one of: "yes", "1", "true" or empty value,    \n
        * return  0 if key found and value is one of: "no", "0", "false",                   \n
        * return -1 if key not found,                                                       \n
        * return -2 otherwise.
        */
        virtual int boolValueToInt(int i_runId, const char * i_key) const noexcept = 0;

        /** return int value by primary key (run id, option key) or default if not found or can not be converted to int. */
        virtual int intValue(int i_runId, const char * i_key, int i_default) const noexcept = 0;

        /** return long value by primary key (run id, option key) or default if not found or can not be converted to long. */
        virtual long long longValue(int i_runId, const char * i_key, long long i_default) const noexcept = 0;

        /** return double value by primary key (run id, option key) or default if not found or can not be converted to double. */
        virtual double doubleValue(int i_runId, const char * i_key, double i_default) const noexcept = 0;

        /** create new table rows by swap with supplied vector of rows. */
        static IRunOptionTable * create(IRowBaseVec & io_rowVec);

        /** get reference to list of all table rows. */
        virtual IRowBaseVec & rowsRef(void) = 0;
    };

    /** workset_lst table public interface. */
    struct IWorksetLstTable : public IMetaTable<WorksetLstRow>
    {
        virtual ~IWorksetLstTable() noexcept = 0;

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
        virtual ~IWorksetTxtTable() noexcept = 0;

        /**
        * select table rows and sorted by primary key: set id and language id.
        *
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static vector<WorksetTxtRow> select(IDbExec * i_dbExec, int i_setId, int i_langId = -1);

        /** select table row by primary key: set id and language id. */
        static vector<WorksetTxtRow> byKey(IDbExec * i_dbExec, int i_setId, int i_langId);
    };

    /** workset_parameter table public interface. */
    struct IWorksetParamTable : public IMetaTable<WorksetParamRow>
    {
        virtual ~IWorksetParamTable() noexcept = 0;

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
        virtual ~IWorksetParamTxtTable() noexcept = 0;

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
        virtual ~ITaskLstTable() noexcept = 0;

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
        virtual ~ITaskTxtTable() noexcept = 0;

        /**
        * select table rows and sorted by primary key: task id and language id.
        *
        * if i_langId >= 0 then select only rows where lang_id = i_langId
        */
        static vector<TaskTxtRow> select(IDbExec * i_dbExec, int i_langId = -1);

        /** select table row by primary key: task id and language id. */
        static vector<TaskTxtRow> byKey(IDbExec * i_dbExec, int i_taskId, int i_langId);
    };

    /** task_set table public interface. */
    struct ITaskSetTable : public IMetaTable<TaskSetRow>
    {
        virtual ~ITaskSetTable() noexcept = 0;

        /**
        * select table rows sorted by primary key: task id and set id.
        *
        * if i_taskId > 0 then select only rows where task_id = i_taskId
        */
        static vector<TaskSetRow> select(IDbExec * i_dbExec, int i_taskId = 0);

        /** select table row by primary key: task id and set id. */
        static vector<TaskSetRow> byKey(IDbExec * i_dbExec, int i_taskId, int i_setId);
    };

    /** task_run_lst table public interface. */
    struct ITaskRunLstTable : public IMetaTable<TaskRunLstRow>
    {
        virtual ~ITaskRunLstTable() noexcept = 0;

        /**
        * select table rows sorted by primary key: task run id.
        *
        * if i_taskId > 0 then select only rows where task_id = i_taskId
        */
        static vector<TaskRunLstRow> select(IDbExec * i_dbExec, int i_taskId = 0);

        /** select table row by primary key: task run id. */
        static vector<TaskRunLstRow> byKey(IDbExec * i_dbExec, int i_taskRunId);
    };

    /** task_run_set table public interface. */
    struct ITaskRunSetTable : public IMetaTable<TaskRunSetRow>
    {
        virtual ~ITaskRunSetTable() noexcept = 0;

        /**
        * select table rows sorted by primary key: task run id, run id.
        *
        * if i_taskRunId > 0 then select only rows where task_run_id = i_taskRunId
        */
        static vector<TaskRunSetRow> select(IDbExec * i_dbExec, int i_taskRunId = 0);

        /** select table row by primary key: task run id, run id. */
        static vector<TaskRunSetRow> byKey(IDbExec * i_dbExec, int i_taskRunId, int i_runId);
    };
}

#endif  // DB_META_TABLE_H
