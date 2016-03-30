// OpenM++ data library: db rows for model metadata
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
using namespace openm;

/** db-exception default error message: "unknown db error" */
const char openm::dbUnknownErrorMessage[] = "unknown db error";

// db-row base class
IRowBase::~IRowBase(void) throw() { }

// lang_lst row less comparator by primary key: language id.
bool LangLstRow::isKeyLess(const LangLstRow & i_left, const LangLstRow & i_right)
{
    return i_left.langId < i_right.langId;
}

// lang_lst row equal comparator by primary key: language id.
bool LangLstRow::isKeyEqual(const LangLstRow & i_left, const LangLstRow & i_right)
{
    return i_left.langId == i_right.langId;
}

// lang_word row less comparator by primary key: language id and word code.
bool LangWordRow::isKeyLess(const LangWordRow & i_left, const LangWordRow & i_right)
{
    return  (i_left.langId < i_right.langId) || (i_left.langId == i_right.langId && i_left.code < i_right.code);
}

// lang_word row equal comparator by primary key: language id and word code.
bool LangWordRow::isKeyEqual(const LangWordRow & i_left, const LangWordRow & i_right)
{
    return i_left.langId == i_right.langId && i_left.code == i_right.code;
}

// model_dic row less comparator by primary key: model id.
bool ModelDicRow::isKeyLess(const ModelDicRow & i_left, const ModelDicRow & i_right)
{
    return i_left.modelId < i_right.modelId;
}

// model_dic row equal comparator by primary key: model id.
bool ModelDicRow::isKeyEqual(const ModelDicRow & i_left, const ModelDicRow & i_right)
{
    return i_left.modelId == i_right.modelId;
}

// model_dic_txt row less comparator by primary key: model id and language id.
bool ModelDicTxtRow::isKeyLess(const ModelDicTxtRow & i_left, const ModelDicTxtRow & i_right)
{
    return (i_left.modelId < i_right.modelId) || (i_left.modelId == i_right.modelId && i_left.langId < i_right.langId);
}

// model_dic_txt row equal comparator by primary key: model id and language id.
bool ModelDicTxtRow::isKeyEqual(const ModelDicTxtRow & i_left, const ModelDicTxtRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.langId == i_right.langId;
}

// model_dic_txt row less comparator by unique key: model id and language name.
bool ModelDicTxtLangRow::uniqueLangKeyLess(const ModelDicTxtLangRow & i_left, const ModelDicTxtLangRow & i_right)
{ 
    return (i_left.modelId < i_right.modelId) || (i_left.modelId == i_right.modelId && i_left.langName < i_right.langName);
}

// model_dic_txt row equal comparator by unique key: model id and language name.
bool ModelDicTxtLangRow::uniqueLangKeyEqual(const ModelDicTxtLangRow & i_left, const ModelDicTxtLangRow & i_right)
{ 
    return i_left.modelId == i_right.modelId && i_left.langName == i_right.langName;
}

// type_dic row less comparator by primary key: model id, type id.
bool TypeDicRow::isKeyLess(const TypeDicRow & i_left, const TypeDicRow & i_right)
{
    return (i_left.modelId < i_right.modelId) || (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId);
}

// type_dic row equal comparator by primary key: model id, type id.
bool TypeDicRow::isKeyEqual(const TypeDicRow & i_left, const TypeDicRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId;
}

// type_dic_txt row less comparator by primary key: model id, type id, language id.
bool TypeDicTxtRow::isKeyLess(const TypeDicTxtRow & i_left, const TypeDicTxtRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.langId < i_right.langId);
}

// type_dic_txt row equal comparator by primary key: model id, type id, language id.
bool TypeDicTxtRow::isKeyEqual(const TypeDicTxtRow & i_left, const TypeDicTxtRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.langId == i_right.langId;
}

// type_dic_txt row less comparator by unique key: model id, type id, language name. 
bool TypeDicTxtLangRow::uniqueLangKeyLess(const TypeDicTxtLangRow & i_left, const TypeDicTxtLangRow & i_right)
{ 
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.langName < i_right.langName);
}

// type_dic_txt row equal comparator by unique key: model id, type id, language name.
bool TypeDicTxtLangRow::uniqueLangKeyEqual(const TypeDicTxtLangRow & i_left, const TypeDicTxtLangRow & i_right)
{ 
    return
        i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.langName == i_right.langName;
}

// type_enum_lst row less comparator by primary key: model id, type id, enum id.
bool TypeEnumLstRow::isKeyLess(const TypeEnumLstRow & i_left, const TypeEnumLstRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.enumId < i_right.enumId);
}

// type_enum_lst row equal comparator by primary key: model id, type id, enum id.
bool TypeEnumLstRow::isKeyEqual(const TypeEnumLstRow & i_left, const TypeEnumLstRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.enumId == i_right.enumId;
}

// type_enum_txt row less comparator by primary key: model id, type id, enum id, language id.
bool TypeEnumTxtRow::isKeyLess(const TypeEnumTxtRow & i_left, const TypeEnumTxtRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.enumId < i_right.enumId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.enumId == i_right.enumId && i_left.langId < i_right.langId);
}

// type_enum_txt row equal comparator by primary key: model id, type id, enum id, language id.
bool TypeEnumTxtRow::isKeyEqual(const TypeEnumTxtRow & i_left, const TypeEnumTxtRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.enumId == i_right.enumId && i_left.langId == i_right.langId;
}

// type_enum_txt row less comparator by unique key: model id, type id, enum id, language name. 
bool TypeEnumTxtLangRow::uniqueLangKeyLess(const TypeEnumTxtLangRow & i_left, const TypeEnumTxtLangRow & i_right)
{ 
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.enumId < i_right.enumId) ||
        (i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.enumId == i_right.enumId && i_left.langName < i_right.langName);
}

// type_enum_txt row equal comparator by unique key: model id, type id, enum id, language name. 
bool TypeEnumTxtLangRow::uniqueLangKeyEqual(const TypeEnumTxtLangRow & i_left, const TypeEnumTxtLangRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.enumId == i_right.enumId && i_left.langName == i_right.langName;
}

// parameter_dic row less comparator by primary key: model id, parameter id.
bool ParamDicRow::isKeyLess(const ParamDicRow & i_left, const ParamDicRow & i_right)
{
    return(i_left.modelId < i_right.modelId) || (i_left.modelId == i_right.modelId && i_left.paramId < i_right.paramId);
}

// parameter_dic row equal comparator by primary key: model id, parameter id.
bool ParamDicRow::isKeyEqual(const ParamDicRow & i_left, const ParamDicRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId;
}

// parameter_dic_txt row less comparator by primary key: model id, parameter id, language id.
bool ParamDicTxtRow::isKeyLess(const ParamDicTxtRow & i_left, const ParamDicTxtRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId < i_right.paramId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.langId < i_right.langId);
}

// parameter_dic_txt row equal comparator by primary key: model id, parameter id, language id.
bool ParamDicTxtRow::isKeyEqual(const ParamDicTxtRow & i_left, const ParamDicTxtRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.langId == i_right.langId;
}

// parameter_dic_txt row less comparator by unique key: model id, parameter id, language name.
bool ParamDicTxtLangRow::uniqueLangKeyLess(const ParamDicTxtLangRow & i_left, const ParamDicTxtLangRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId < i_right.paramId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.langName < i_right.langName);
}

// parameter_dic_txt row equal comparator by unique key: model id, parameter id, language name.
bool ParamDicTxtLangRow::uniqueLangKeyEqual(const ParamDicTxtLangRow & i_left, const ParamDicTxtLangRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.langName == i_right.langName;
}

// parameter_dims row less comparator by primary key: model id, parameter id, dimension id.
bool ParamDimsRow::isKeyLess(const ParamDimsRow & i_left, const ParamDimsRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId < i_right.paramId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.dimId < i_right.dimId);
}

// parameter_dims row equal comparator by primary key: model id, parameter id, dimension id.
bool ParamDimsRow::isKeyEqual(const ParamDimsRow & i_left, const ParamDimsRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.dimId == i_right.dimId;
}

// parameter_dims_txt row less comparator by primary key: model id, parameter id, dimension id, language id.
bool ParamDimsTxtRow::isKeyLess(const ParamDimsTxtRow & i_left, const ParamDimsTxtRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId < i_right.paramId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.dimId < i_right.dimId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.dimId == i_right.dimId && i_left.langId < i_right.langId);
}

// parameter_dims_txt row equal comparator by primary key: model id, parameter id, dimension id, language id.
bool ParamDimsTxtRow::isKeyEqual(const ParamDimsTxtRow & i_left, const ParamDimsTxtRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.dimId == i_right.dimId && i_left.langId == i_right.langId;
}

// parameter_dims_txt row less comparator by unique key : model id, parameter id, dimension id, language name.
bool ParamDimsTxtLangRow::uniqueLangKeyLess(const ParamDimsTxtLangRow & i_left, const ParamDimsTxtLangRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId < i_right.paramId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.dimId < i_right.dimId) ||
        (i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.dimId == i_right.dimId && i_left.langName < i_right.langName);
}

// parameter_dims_txt row equal comparator by unique key : model id, parameter id, dimension id, language name.
bool ParamDimsTxtLangRow::uniqueLangKeyEqual(const ParamDimsTxtLangRow & i_left, const ParamDimsTxtLangRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.paramId == i_right.paramId && i_left.dimId == i_right.dimId && i_left.langName == i_right.langName;
}

// table_dic row less comparator by primary key: model id, table id.
bool TableDicRow::isKeyLess(const TableDicRow & i_left, const TableDicRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) || (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId);
}

// table_dic row equal comparator by primary key: model id, table id.
bool TableDicRow::isKeyEqual(const TableDicRow & i_left, const TableDicRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId;
}

// table_dic_txt row less comparator by primary key: model id, table id, language id.
bool TableDicTxtRow::isKeyLess(const TableDicTxtRow & i_left, const TableDicTxtRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.langId < i_right.langId);
}

// table_dic_txt row equal comparator by primary key: model id, table id, language id.
bool TableDicTxtRow::isKeyEqual(const TableDicTxtRow & i_left, const TableDicTxtRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.langId == i_right.langId;
}

// table_dic_txt row less comparator by unique key: model id, table id, language name.
bool TableDicTxtLangRow::uniqueLangKeyLess(const TableDicTxtLangRow & i_left, const TableDicTxtLangRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.langName < i_right.langName);
}

// table_dic_txt row equal comparator by unique key: model id, table id, language name.
bool TableDicTxtLangRow::uniqueLangKeyEqual(const TableDicTxtLangRow & i_left, const TableDicTxtLangRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.langName == i_right.langName;
}

// table_dims row less comparator by primary key: model id, table id, dimension id.
bool TableDimsRow::isKeyLess(const TableDimsRow & i_left, const TableDimsRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.dimId < i_right.dimId);
}

// table_dims row equal comparator by primary key: model id, table id, dimension id.
bool TableDimsRow::isKeyEqual(const TableDimsRow & i_left, const TableDimsRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.dimId == i_right.dimId;
}

// table_dims_txt row less comparator by primary key: model id, table id, dimension id, language id.
bool TableDimsTxtRow::isKeyLess(const TableDimsTxtRow & i_left, const TableDimsTxtRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.dimId < i_right.dimId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.dimId == i_right.dimId && i_left.langId < i_right.langId);
}

// table_dims_txt row equal comparator by primary key: model id, table id, dimension id, language id.
bool TableDimsTxtRow::isKeyEqual(const TableDimsTxtRow & i_left, const TableDimsTxtRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.dimId == i_right.dimId && i_left.langId == i_right.langId;
}

// table_dims_txt row less comparator by unique key : model id, table id, dimension id, language name.
bool TableDimsTxtLangRow::uniqueLangKeyLess(const TableDimsTxtLangRow & i_left, const TableDimsTxtLangRow & i_right)
{ 
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.dimId < i_right.dimId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.dimId == i_right.dimId && i_left.langName < i_right.langName);
}

// table_dims_txt row equal comparator by unique key : model id, table id, dimension id, language name.
bool TableDimsTxtLangRow::uniqueLangKeyEqual(const TableDimsTxtLangRow & i_left, const TableDimsTxtLangRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.dimId == i_right.dimId && i_left.langName == i_right.langName;
}

// table_acc row less comparator by primary key: model id, table id, accumulator id.
bool TableAccRow::isKeyLess(const TableAccRow & i_left, const TableAccRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.accId < i_right.accId);
}

// table_acc row equal comparator by primary key: model id, table id, accumulator id.
bool TableAccRow::isKeyEqual(const TableAccRow & i_left, const TableAccRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.accId == i_right.accId;
}

// table_acc_txt row less comparator by primary key: model id, table id, accumulator id, language id.
bool TableAccTxtRow::isKeyLess(const TableAccTxtRow & i_left, const TableAccTxtRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.accId < i_right.accId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.accId == i_right.accId && i_left.langId < i_right.langId);
}

// table_acc_txt row equal comparator by primary key: model id, table id, accumulator id, language id.
bool TableAccTxtRow::isKeyEqual(const TableAccTxtRow & i_left, const TableAccTxtRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.accId == i_right.accId && i_left.langId == i_right.langId;
}

// table_acc_txt row less comparator by unique key: model id, table id, accumulator id, language name.
bool TableAccTxtLangRow::uniqueLangKeyLess(const TableAccTxtLangRow & i_left, const TableAccTxtLangRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.accId < i_right.accId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.accId == i_right.accId && i_left.langName < i_right.langName);
}

// table_acc_txt row equal comparator by unique key: model id, table id, accumulator id, language name. 
bool TableAccTxtLangRow::uniqueLangKeyEqual(const TableAccTxtLangRow & i_left, const TableAccTxtLangRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.accId == i_right.accId && i_left.langName == i_right.langName;
}

// table_expr row less comparator by primary key: model id, table id, expr id.
bool TableExprRow::isKeyLess(const TableExprRow & i_left, const TableExprRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.exprId < i_right.exprId);
}

// table_expr row equal comparator by primary key: model id, table id, expr id.
bool TableExprRow::isKeyEqual(const TableExprRow & i_left, const TableExprRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.exprId == i_right.exprId;
}

// table_expr_txt row less comparator by primary key: model id, table id, expr id, language id.
bool TableExprTxtRow::isKeyLess(const TableExprTxtRow & i_left, const TableExprTxtRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.exprId < i_right.exprId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.exprId == i_right.exprId && i_left.langId < i_right.langId);
}

// table_expr_txt row equal comparator by primary key: model id, table id, expr id, language id.
bool TableExprTxtRow::isKeyEqual(const TableExprTxtRow & i_left, const TableExprTxtRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.exprId == i_right.exprId && i_left.langId == i_right.langId;
}

// table_expr_txt row less comparator by unique key: model id, table id, expr id, language name. 
bool TableExprTxtLangRow::uniqueLangKeyLess(const TableExprTxtLangRow & i_left, const TableExprTxtLangRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId < i_right.tableId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.exprId < i_right.exprId) ||
        (i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.exprId == i_right.exprId && i_left.langName < i_right.langName);
}

// table_expr_txt row equal comparator by unique key: model id, table id, expr id, language name. 
bool TableExprTxtLangRow::uniqueLangKeyEqual(const TableExprTxtLangRow & i_left, const TableExprTxtLangRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.tableId == i_right.tableId && i_left.exprId == i_right.exprId && i_left.langName == i_right.langName;
}

// group_lst row less comparator by primary key: model id, group id.
bool GroupLstRow::isKeyLess(const GroupLstRow & i_left, const GroupLstRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) || (i_left.modelId == i_right.modelId && i_left.groupId < i_right.groupId);
}

// group_lst row equal comparator by primary key: model id, group id.
bool GroupLstRow::isKeyEqual(const GroupLstRow & i_left, const GroupLstRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.groupId == i_right.groupId;
}

// group_txt row less comparator by primary key: model id, group id, language id.
bool GroupTxtRow::isKeyLess(const GroupTxtRow & i_left, const GroupTxtRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.groupId < i_right.groupId) ||
        (i_left.modelId == i_right.modelId && i_left.groupId == i_right.groupId && i_left.langId < i_right.langId);
}

// group_txt row equal comparator by primary key: model id, group id, language id.
bool GroupTxtRow::isKeyEqual(const GroupTxtRow & i_left, const GroupTxtRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.groupId == i_right.groupId && i_left.langId == i_right.langId;
}

// group_txt row less comparator by unique key: model id, group id, language name. 
bool GroupTxtLangRow::uniqueLangKeyLess(const GroupTxtLangRow & i_left, const GroupTxtLangRow & i_right)
{
    return i_left.modelId == i_right.modelId && i_left.groupId == i_right.groupId && i_left.langName == i_right.langName;
}

// group_txt row equal comparator by unique key: model id, group id, language name. 
bool GroupTxtLangRow::uniqueLangKeyEqual(const GroupTxtLangRow & i_left, const GroupTxtLangRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.groupId < i_right.groupId) ||
        (i_left.modelId == i_right.modelId && i_left.groupId == i_right.groupId && i_left.langName < i_right.langName);
}

// group_pc row less comparator by primary key: model id, group id, child position.
bool GroupPcRow::isKeyLess(const GroupPcRow & i_left, const GroupPcRow & i_right)
{
    return
        (i_left.modelId < i_right.modelId) ||
        (i_left.modelId == i_right.modelId && i_left.groupId < i_right.groupId) ||
        (i_left.modelId == i_right.modelId && i_left.groupId == i_right.groupId && i_left.childPos < i_right.childPos);
}

// group_pc row equal comparator by primary key: model id, group id, child position.
bool GroupPcRow::isKeyEqual(const GroupPcRow & i_left, const GroupPcRow & i_right)
{
    return
        i_left.modelId == i_right.modelId && i_left.groupId == i_right.groupId && i_left.childPos == i_right.childPos;
}

// profile_lst row less comparator by primary key: profile name.
bool ProfileLstRow::isKeyLess(const ProfileLstRow & i_left, const ProfileLstRow & i_right)
{
    return i_left.name < i_right.name;
}

// profile_lst row equal comparator by primary key: profile name.
bool ProfileLstRow::isKeyEqual(const ProfileLstRow & i_left, const ProfileLstRow & i_right)
{
    return i_left.name == i_right.name;
}

// profile_option row less comparator by primary key: profile name, option key.
bool ProfileOptionRow::isKeyLess(const ProfileOptionRow & i_left, const ProfileOptionRow & i_right)
{
    return (i_left.name < i_right.name) || (i_left.name == i_right.name && i_left.key < i_right.key);
}

// profile_option row equal comparator by primary key: profile name, option key.
bool ProfileOptionRow::isKeyEqual(const ProfileOptionRow & i_left, const ProfileOptionRow & i_right)
{
    return i_left.name == i_right.name && i_left.key == i_right.key;
}

// run_lst row less comparator by primary key: run id.
bool RunLstRow::isKeyLess(const RunLstRow & i_left, const RunLstRow & i_right)
{
    return i_left.runId < i_right.runId;
}

// run_lst row equal comparator by primary key: run id.
bool RunLstRow::isKeyEqual(const RunLstRow & i_left, const RunLstRow & i_right)
{
    return i_left.runId == i_right.runId;
}

// run_lst table find row by primary key: run id.
vector<RunLstRow>::const_iterator RunLstRow::byKey(int i_runId, const vector<RunLstRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_runId](const RunLstRow & i_row) -> bool { return i_row.runId == i_runId; }
    );
}

// run_txt row less comparator by primary key: run id and language id.
bool RunTxtRow::isKeyLess(const RunTxtRow & i_left, const RunTxtRow & i_right)
{
    return (i_left.runId < i_right.runId) || (i_left.runId == i_right.runId && i_left.langId < i_right.langId);
}

// run_txt row equal comparator by primary key: run id and language id.
bool RunTxtRow::isKeyEqual(const RunTxtRow & i_left, const RunTxtRow & i_right)
{
    return i_left.runId == i_right.runId && i_left.langId == i_right.langId;
}

// run_txt table find row by primary key: run id and language id.
vector<RunTxtRow>::const_iterator RunTxtRow::byKey(int i_runId, int i_langId, const vector<RunTxtRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_runId, i_langId](const RunTxtRow & i_row) -> bool { return i_row.runId == i_runId && i_row.langId == i_langId; }
    );
}

// run_option row less comparator by primary key: run id, option key.
bool RunOptionRow::isKeyLess(const RunOptionRow & i_left, const RunOptionRow & i_right)
{
    return (i_left.runId < i_right.runId) || (i_left.runId == i_right.runId && i_left.key < i_right.key);
}

// run_option row equal comparator by primary key: run id, option key.
bool RunOptionRow::isKeyEqual(const RunOptionRow & i_left, const RunOptionRow & i_right)
{
    return i_left.runId == i_right.runId && i_left.key == i_right.key;
}

// run_parameter row less comparator by primary key: run id, parameter id.
bool RunParamRow::isKeyLess(const RunParamRow & i_left, const RunParamRow & i_right)
{
    return (i_left.runId < i_right.runId) || (i_left.runId == i_right.runId && i_left.paramId < i_right.paramId);
}

// run_parameter row equal comparator by primary key: run id, parameter id.
bool RunParamRow::isKeyEqual(const RunParamRow & i_left, const RunParamRow & i_right)
{
    return i_left.runId == i_right.runId && i_left.paramId == i_right.paramId;
}

// find row by primary key: run id, parameter id. 
vector<RunParamRow>::const_iterator RunParamRow::byKey(int i_runId, int i_paramId, const vector<RunParamRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_runId, i_paramId](const RunParamRow & i_row) -> bool { return i_row.runId == i_runId && i_row.paramId == i_paramId; }
    );
}

// select base run id by run id and parameter id, return zero if not exist
int RunParamRow::findBaseRunId(int i_runId, int i_paramId, const vector<RunParamRow> & i_rowVec)
{
    int baseRunId = 0;

    auto rpIt = byKey(i_runId, i_paramId, i_rowVec);
    if (rpIt != i_rowVec.cend()) baseRunId = rpIt->baseRunId;

    return baseRunId;
}

// parameter_run_txt row less comparator by primary key: run id, parameter id, language id.
bool ParamRunTxtRow::isKeyLess(const ParamRunTxtRow & i_left, const ParamRunTxtRow & i_right)
{
    return
        (i_left.runId < i_right.runId) ||
        (i_left.runId == i_right.runId && i_left.paramId < i_right.paramId) ||
        (i_left.runId == i_right.runId && i_left.paramId == i_right.paramId && i_left.langId < i_right.langId);
}

// parameter_run_txt row equal comparator by primary key: run id, parameter id, language id.
bool ParamRunTxtRow::isKeyEqual(const ParamRunTxtRow & i_left, const ParamRunTxtRow & i_right)
{
    return i_left.runId == i_right.runId && i_left.paramId == i_right.paramId && i_left.langId == i_right.langId;
}

// workset_lst row less comparator by primary key: set id.
bool WorksetLstRow::isKeyLess(const WorksetLstRow & i_left, const WorksetLstRow & i_right)
{
    return i_left.setId < i_right.setId;
}

// workset_lst row equal comparator by primary key: set id.
bool WorksetLstRow::isKeyEqual(const WorksetLstRow & i_left, const WorksetLstRow & i_right)
{
    return i_left.setId == i_right.setId;
}

// workset_lst table find row by primary key: set id.
vector<WorksetLstRow>::const_iterator WorksetLstRow::byKey(int i_setId, const vector<WorksetLstRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_setId](const WorksetLstRow & i_row) -> bool { return i_row.setId == i_setId; }
    );
}

// workset_txt row less comparator by primary key: set id and language id.
bool WorksetTxtRow::isKeyLess(const WorksetTxtRow & i_left, const WorksetTxtRow & i_right)
{
    return (i_left.setId < i_right.setId) || (i_left.setId == i_right.setId && i_left.langId < i_right.langId);
}

// workset_txt row equal comparator by primary key: set id and language id.
bool WorksetTxtRow::isKeyEqual(const WorksetTxtRow & i_left, const WorksetTxtRow & i_right)
{
    return i_left.setId == i_right.setId && i_left.langId == i_right.langId;
}

// workset_txt table find row by primary key: set id and language id.
vector<WorksetTxtRow>::const_iterator WorksetTxtRow::byKey(int i_setId, int i_langId, const vector<WorksetTxtRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_setId, i_langId](const WorksetTxtRow & i_row) -> bool { return i_row.setId == i_setId && i_row.langId == i_langId; }
    );
}

// workset_txt row less comparator by unique key: set id, language name.
bool WorksetTxtLangRow::uniqueLangKeyLess(const WorksetTxtLangRow & i_left, const WorksetTxtLangRow & i_right)
{
    return
        (i_left.setId < i_right.setId) ||
        (i_left.setId == i_right.setId && i_left.langName < i_right.langName);
}

// workset_txt row equal comparator by unique key: set id, language name.
bool WorksetTxtLangRow::uniqueLangKeyEqual(const WorksetTxtLangRow & i_left, const WorksetTxtLangRow & i_right)
{
    return i_left.setId == i_right.setId && i_left.langName == i_right.langName;
}

// workset_parameter row less comparator by primary key: set id and parameter id.
bool WorksetParamRow::isKeyLess(const WorksetParamRow & i_left, const WorksetParamRow & i_right)
{
    return (i_left.setId < i_right.setId) || (i_left.setId == i_right.setId && i_left.paramId < i_right.paramId);
}

// workset_parameter row equal comparator by primary key: set id and parameter id.
bool WorksetParamRow::isKeyEqual(const WorksetParamRow & i_left, const WorksetParamRow & i_right)
{
    return i_left.setId == i_right.setId && i_left.paramId == i_right.paramId;
}

// workset_parameter table find row by primary key: set id and parameter id.
vector<WorksetParamRow>::const_iterator WorksetParamRow::byKey(int i_setId, int i_paramId, const vector<WorksetParamRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_setId, i_paramId](const WorksetParamRow & i_row) -> bool { return i_row.setId == i_setId && i_row.paramId == i_paramId; }
    );
}

// workset_parameter_txt row less comparator by primary key: set id, parameter id, language id.
bool WorksetParamTxtRow::isKeyLess(const WorksetParamTxtRow & i_left, const WorksetParamTxtRow & i_right)
{
    return
        (i_left.setId < i_right.setId) ||
        (i_left.setId == i_right.setId && i_left.paramId < i_right.paramId) ||
        (i_left.setId == i_right.setId && i_left.paramId == i_right.paramId && i_left.langId < i_right.langId);
}

// workset_parameter_txt row equal comparator by primary key: set id, parameter id, language id.
bool WorksetParamTxtRow::isKeyEqual(const WorksetParamTxtRow & i_left, const WorksetParamTxtRow & i_right)
{
    return i_left.setId == i_right.setId && i_left.paramId == i_right.paramId && i_left.langId == i_right.langId;
}

// workset_parameter_txt table find row by primary key: set id, parameter id, language id.
vector<WorksetParamTxtRow>::const_iterator WorksetParamTxtRow::byKey(
    int i_setId, int i_paramId, int i_langId, const vector<WorksetParamTxtRow> & i_rowVec
    )
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_setId, i_paramId, i_langId](const WorksetParamTxtRow & i_row) -> bool {
        return i_row.setId == i_setId && i_row.paramId == i_paramId && i_row.langId == i_langId;
    }
    );
}

// workset_parameter_txt row less comparator by unique key: set id, parameter id, language name.
bool WorksetParamTxtLangRow::uniqueLangKeyLess(const WorksetParamTxtLangRow & i_left, const WorksetParamTxtLangRow & i_right)
{
    return
        (i_left.setId < i_right.setId) ||
        (i_left.setId == i_right.setId && i_left.paramId < i_right.paramId) ||
        (i_left.setId == i_right.setId && i_left.paramId == i_right.paramId && i_left.langName < i_right.langName);
}

// workset_parameter_txt row equal comparator by unique key: set id, parameter id, language name.
bool WorksetParamTxtLangRow::uniqueLangKeyEqual(const WorksetParamTxtLangRow & i_left, const WorksetParamTxtLangRow & i_right)
{
    return i_left.setId == i_right.setId && i_left.paramId == i_right.paramId && i_left.langName == i_right.langName;
}

/** less comparator by primary key: task id. */
bool TaskLstRow::isKeyLess(const TaskLstRow & i_left, const TaskLstRow & i_right)
{
    return i_left.taskId < i_right.taskId;
}

/** equal comparator by primary key: task id. */
bool TaskLstRow::isKeyEqual(const TaskLstRow & i_left, const TaskLstRow & i_right)
{
    return i_left.taskId == i_right.taskId;
}

/** find row by primary key: task id. */
vector<TaskLstRow>::const_iterator TaskLstRow::byKey(int i_taskId, const vector<TaskLstRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_taskId](const TaskLstRow & i_row) -> bool { return i_row.taskId == i_taskId; }
    );
}

/** less comparator by primary key: task id and language id. */
bool TaskTxtRow::isKeyLess(const TaskTxtRow & i_left, const TaskTxtRow & i_right)
{
    return (i_left.taskId < i_right.taskId) || (i_left.taskId == i_right.taskId && i_left.langId < i_right.langId);
}

/** equal comparator by primary key: task id and language id. */
bool TaskTxtRow::isKeyEqual(const TaskTxtRow & i_left, const TaskTxtRow & i_right)
{
    return i_left.taskId == i_right.taskId && i_left.langId == i_right.langId;
}

/** find row by primary key: task id and language id. */
vector<TaskTxtRow>::const_iterator TaskTxtRow::byKey(int i_taskId, int i_langId, const vector<TaskTxtRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_taskId, i_langId](const TaskTxtRow & i_row) -> bool { return i_row.taskId == i_taskId && i_row.langId == i_langId; }
    );
}

/** less comparator by unique key: task id, language name. */
bool TaskTxtLangRow::uniqueLangKeyLess(const TaskTxtLangRow & i_left, const TaskTxtLangRow & i_right)
{
    return
        (i_left.taskId < i_right.taskId) ||
        (i_left.taskId == i_right.taskId && i_left.langName < i_right.langName);
}

/** equal comparator by unique key: task id, language name. */
bool TaskTxtLangRow::uniqueLangKeyEqual(const TaskTxtLangRow & i_left, const TaskTxtLangRow & i_right)
{
    return i_left.taskId == i_right.taskId && i_left.langName == i_right.langName;
}

/** less comparator by primary key: task id and set id. */
bool TaskSetRow::isKeyLess(const TaskSetRow & i_left, const TaskSetRow & i_right)
{
    return (i_left.taskId < i_right.taskId) || (i_left.taskId == i_right.taskId && i_left.setId < i_right.setId);
}

/** equal comparator by primary key: task id and set id. */
bool TaskSetRow::isKeyEqual(const TaskSetRow & i_left, const TaskSetRow & i_right)
{
    return i_left.taskId == i_right.taskId && i_left.setId == i_right.setId;
}

/** find row by primary key: task id and set id. */
vector<TaskSetRow>::const_iterator TaskSetRow::byKey(int i_taskId, int i_setId, const vector<TaskSetRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_taskId, i_setId](const TaskSetRow & i_row) -> bool { return i_row.taskId == i_taskId && i_row.setId == i_setId; }
    );
}

/** less comparator by primary key: task run id. */
bool TaskRunLstRow::isKeyLess(const TaskRunLstRow & i_left, const TaskRunLstRow & i_right)
{
    return i_left.taskRunId < i_right.taskRunId;
}

/** equal comparator by primary key: task run id. */
bool TaskRunLstRow::isKeyEqual(const TaskRunLstRow & i_left, const TaskRunLstRow & i_right)
{
    return i_left.taskRunId == i_right.taskRunId;
}

/** find row by primary key: task run id. */
vector<TaskRunLstRow>::const_iterator TaskRunLstRow::byKey(int i_taskRunId, const vector<TaskRunLstRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_taskRunId](const TaskRunLstRow & i_row) -> bool { return i_row.taskRunId == i_taskRunId; }
    );
}

/** less comparator by primary key: task run id, run id. */
bool TaskRunSetRow::isKeyLess(const TaskRunSetRow & i_left, const TaskRunSetRow & i_right)
{
    return (i_left.taskRunId < i_right.taskRunId) || (i_left.taskRunId == i_right.taskRunId && i_left.runId < i_right.runId);
}

/** equal comparator by primary key: task run id, run id. */
bool TaskRunSetRow::isKeyEqual(const TaskRunSetRow & i_left, const TaskRunSetRow & i_right)
{
    return i_left.taskRunId == i_right.taskRunId && i_left.runId == i_right.runId;
}

/** find row by primary key: task run id, run id. */
vector<TaskRunSetRow>::const_iterator TaskRunSetRow::byKey(int i_taskRunId, int i_runId, const vector<TaskRunSetRow> & i_rowVec)
{
    return find_if(
        i_rowVec.cbegin(),
        i_rowVec.cend(),
        [i_taskRunId, i_runId](const TaskRunSetRow & i_row) -> bool { return i_row.taskId == i_taskRunId && i_row.runId == i_runId; }
    );
}
