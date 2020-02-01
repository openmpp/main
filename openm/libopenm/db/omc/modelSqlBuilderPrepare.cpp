// OpenM++ data library: class to produce new model sql from metadata (prepare portion of the class)
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "modelSqlBuilder.h"

using namespace openm;

// set meta rows values, sort and calculate digests for types, parameters, tables and model
void ModelSqlBuilder::setModelMetaRows(MetaModelHolder & io_metaRows) const
{
    // trim string fields and sort all rows by primary key
    trimModelRows(io_metaRows);
    sortModelRows(io_metaRows);

    // set names for dimension, accumulator, expression columns
    setColumnNames(io_metaRows);

    // set  field values and update digests
    setModelDicRow(io_metaRows.modelDic);
    setTypeDicRows(io_metaRows);
    setParamDicRows(io_metaRows);
    setTableDicRows(io_metaRows);

    // calculate model digest
    io_metaRows.modelDic.digest = makeModelDigest(io_metaRows);
}

// set parameter meta rows and calculate digests
void ModelSqlBuilder::setParamDicRows(MetaModelHolder & io_metaRows) const
{
    // make parameter digest and name db tables
    for (ParamDicRow & paramRow : io_metaRows.paramDic) {
        auto [dgst, impDgst] = makeParamDigest(paramRow, io_metaRows);
        paramRow.digest = dgst;
        paramRow.importDigest = impDgst;
        string p = makeDbNamePrefix(paramRow.paramId, paramRow.paramName);
        string s = makeDbNameSuffix(paramRow.paramId, paramRow.paramName, paramRow.digest);
        paramRow.dbRunTable = p + "_p" + s;
        paramRow.dbSetTable = p + "_w" + s;
    }
}

// set output tables meta rows and calculate digests
void ModelSqlBuilder::setTableDicRows(MetaModelHolder & io_metaRows) const
{
    // make parameter digest and db table parts
    for (TableDicRow & tableRow : io_metaRows.tableDic) {
        auto [dgst, impDgst] = makeOutTableDigest(tableRow, io_metaRows);
        tableRow.digest = dgst;
        tableRow.importDigest = impDgst;
        string p = makeDbNamePrefix(tableRow.tableId, tableRow.tableName);
        string s = makeDbNameSuffix(tableRow.tableId, tableRow.tableName, tableRow.digest);
        tableRow.dbExprTable = p + "_v" + s;
        tableRow.dbAccTable = p + "_a" + s;
        tableRow.dbAccAll = p + "_d" + s;
    }
}

// sort model metadata rows
void ModelSqlBuilder::sortModelRows(MetaModelHolder & io_metaRows)
{ 
    sort(io_metaRows.langLst.begin(), io_metaRows.langLst.end(), LangLstRow::isCodeLess);
    sort(io_metaRows.modelTxt.begin(), io_metaRows.modelTxt.end(), ModelDicTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.typeDic.begin(), io_metaRows.typeDic.end(), TypeDicRow::isKeyLess);
    sort(io_metaRows.typeTxt.begin(), io_metaRows.typeTxt.end(), TypeDicTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.typeEnum.begin(), io_metaRows.typeEnum.end(), TypeEnumLstRow::isKeyLess);
    sort(io_metaRows.typeEnumTxt.begin(), io_metaRows.typeEnumTxt.end(), TypeEnumTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.paramDic.begin(), io_metaRows.paramDic.end(), ParamDicRow::isKeyLess);
    sort(io_metaRows.paramImport.begin(), io_metaRows.paramImport.end(), ParamImportRow::isKeyLess);
    sort(io_metaRows.paramTxt.begin(), io_metaRows.paramTxt.end(), ParamDicTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.paramDims.begin(), io_metaRows.paramDims.end(), ParamDimsRow::isKeyLess);
    sort(io_metaRows.paramDimsTxt.begin(), io_metaRows.paramDimsTxt.end(), ParamDimsTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.tableDic.begin(), io_metaRows.tableDic.end(), TableDicRow::isKeyLess);
    sort(io_metaRows.tableTxt.begin(), io_metaRows.tableTxt.end(), TableDicTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.tableDims.begin(), io_metaRows.tableDims.end(), TableDimsRow::isKeyLess);
    sort(io_metaRows.tableDimsTxt.begin(), io_metaRows.tableDimsTxt.end(), TableDimsTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.tableAcc.begin(), io_metaRows.tableAcc.end(), TableAccRow::isKeyLess);
    sort(io_metaRows.tableAccTxt.begin(), io_metaRows.tableAccTxt.end(), TableAccTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.tableExpr.begin(), io_metaRows.tableExpr.end(), TableExprRow::isKeyLess);
    sort(io_metaRows.tableExprTxt.begin(), io_metaRows.tableExprTxt.end(), TableExprTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.groupLst.begin(), io_metaRows.groupLst.end(), GroupLstRow::isKeyLess);
    sort(io_metaRows.groupTxt.begin(), io_metaRows.groupTxt.end(), GroupTxtLangRow::uniqueLangKeyLess);
    sort(io_metaRows.groupPc.begin(), io_metaRows.groupPc.end(), GroupPcRow::isKeyLess);
}

// sort workset metadata rows
void ModelSqlBuilder::sortWorksetRows(MetaSetLangHolder & io_metaSet)
{
    sort(io_metaSet.worksetTxt.begin(), io_metaSet.worksetTxt.end(), WorksetTxtLangRow::uniqueLangKeyLess);
    sort(io_metaSet.worksetParam.begin(), io_metaSet.worksetParam.end(), WorksetParamRow::isKeyLess);
    sort(io_metaSet.worksetParamTxt.begin(), io_metaSet.worksetParamTxt.end(), WorksetParamTxtLangRow::uniqueLangKeyLess);
}

// trim string fields in model metadata rows
void ModelSqlBuilder::trimModelRows(MetaModelHolder & io_metaRows)
{ 
    locale loc("");

    io_metaRows.modelDic.name = trim(io_metaRows.modelDic.name, loc);
    io_metaRows.modelDic.digest = trim(io_metaRows.modelDic.digest, loc);
    io_metaRows.modelDic.version = trim(io_metaRows.modelDic.version, loc);
    io_metaRows.modelDic.defaultLangCode = trim(io_metaRows.modelDic.defaultLangCode, loc);

    for (auto & row : io_metaRows.langLst) {
        row.code = trim(row.code, loc);
        row.name = trim(row.name, loc);
    }

    for (auto & row : io_metaRows.modelTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
    }

    for (auto & row : io_metaRows.typeDic) {
        row.name = trim(row.name, loc);
        row.digest = trim(row.digest, loc);
    }

    for (auto & row : io_metaRows.typeTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
    }

    for (auto & row : io_metaRows.typeEnum) {
        row.name = trim(row.name, loc);
    }

    for (auto & row : io_metaRows.typeEnumTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
    }

    for (auto & row : io_metaRows.paramDic) {
        row.paramName = trim(row.paramName, loc);
        row.digest = trim(row.digest, loc);
    }

    for (auto & row : io_metaRows.paramImport) {
        row.fromName = trim(row.fromName, loc);
        row.fromModel = trim(row.fromModel, loc);
    }

    for (auto & row : io_metaRows.paramTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
    }

    for (auto & row : io_metaRows.paramDims) {
        row.name = trim(row.name, loc);
    }

    for (auto & row : io_metaRows.paramDimsTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
    }

    for (auto & row : io_metaRows.tableDic) {
        row.tableName = trim(row.tableName, loc);
        row.digest = trim(row.digest, loc);
    }

    for (auto & row : io_metaRows.tableTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
        row.exprDescr = trim(row.exprDescr, loc);
        row.exprNote = trim(row.exprNote, loc);
    }

    for (auto & row : io_metaRows.tableDims) {
        row.name = trim(row.name, loc);
    }

    for (auto & row : io_metaRows.tableDimsTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
    }

    for (auto & row : io_metaRows.tableAcc) {
        row.name = trim(row.name, loc);
        row.accSrc = trim(row.accSrc, loc);
        row.accSql = trim(row.accSql, loc);
    }

    for (auto & row : io_metaRows.tableAccTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
    }

    for (auto & row : io_metaRows.tableExpr) {
        row.name = trim(row.name, loc);
        row.srcExpr = trim(row.srcExpr, loc);
        row.sqlExpr = trim(row.sqlExpr, loc);
    }

    for (auto & row : io_metaRows.tableExprTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
    }

    for (auto & row : io_metaRows.groupLst) {
        row.name = trim(row.name, loc);
    }

    for (auto & row : io_metaRows.groupTxt) {
        row.langCode = trim(row.langCode, loc);
        row.descr = trim(row.descr, loc);
        row.note = trim(row.note, loc);
    }

    // no string fields
    // for (auto & row : io_metaRows.groupPc) { }
}

// sort and validate metadata rows for uniqueness and referential integrity
void ModelSqlBuilder::prepare(MetaModelHolder & io_metaRows) const
{
    int mId = io_metaRows.modelDic.modelId;

    // default model language is a foreign key, search for language code
    LangLstRow lr(io_metaRows.modelDic.defaultLangCode);
    if (!std::binary_search(
        io_metaRows.langLst.cbegin(),
        io_metaRows.langLst.cend(),
        lr,
        LangLstRow::isCodeLess
    )) throw DbException(LT("invalid model default language: %s: not found in lang_lst"), lr.code.c_str());


    // model_dic_txt table
    // unique: model id and language name; master key: model id; foreign key: language code;
    // cleanup cr or lf in description and notes
    for (vector<ModelDicTxtLangRow>::iterator rowIt = io_metaRows.modelTxt.begin(); rowIt != io_metaRows.modelTxt.end(); ++rowIt) {

        if (rowIt->modelId != mId)
            throw DbException(LT("in model_dic_txt invalid model id: %d, expected: %d in row with language id: %d and name: %s"), rowIt->modelId, mId, rowIt->langId, rowIt->langCode.c_str());

        vector<ModelDicTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.modelTxt.cend() && ModelDicTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in model_dic_txt not unique model id: %d and language: %s"), rowIt->modelId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in model_dic_txt invalid model id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // type_dic table
    // unique: model id and type id; unique: model id and type name; master key: model id
    for (vector<TypeDicRow>::const_iterator rowIt = io_metaRows.typeDic.cbegin(); rowIt != io_metaRows.typeDic.cend(); ++rowIt) {

        if (rowIt->modelId != mId)
            throw DbException(LT("in type_dic invalid model id: %d, expected: %d in row with type id: %d"), rowIt->modelId, mId, rowIt->typeId);

        vector<TypeDicRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.typeDic.cend() && TypeDicRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in type_dic not unique model id: %d and type id: %d"), rowIt->modelId, rowIt->typeId);

        if (std::any_of(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            [rowIt](const TypeDicRow & i_row) -> bool {
            return
                i_row.modelId == rowIt->modelId && i_row.typeId != rowIt->typeId && i_row.name == rowIt->name;
        }
        )) throw DbException(LT("in type_dic not unique model id: %d and type name: %s"), rowIt->modelId, rowIt->name.c_str());
    }

    // type_dic_txt table
    // unique: model id, type id, language name; master key: model id, type id; foreign key: language code;
    // cleanup cr or lf in description and notes
    for (vector<TypeDicTxtLangRow>::iterator rowIt = io_metaRows.typeTxt.begin(); rowIt != io_metaRows.typeTxt.end(); ++rowIt) {

        TypeDicRow mkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            mkRow,
            TypeDicRow::isKeyLess
        )) throw DbException(LT("in type_dic_txt invalid model id: %d and type id: %d: not found in type_dic"), rowIt->modelId, rowIt->typeId);

        vector<TypeDicTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.typeTxt.cend() && TypeDicTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in type_dic_txt not unique model id: %d, type id: %d and language: %s"), rowIt->modelId, rowIt->typeId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in type_dic_txt invalid  model id: %d, type id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->typeId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // type_enum_lst table
    // unique: model id, type id, enum id; master key: model id, type id
    for (vector<TypeEnumLstRow>::const_iterator rowIt = io_metaRows.typeEnum.cbegin(); rowIt != io_metaRows.typeEnum.cend(); ++rowIt) {

        TypeDicRow mkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            mkRow,
            TypeDicRow::isKeyLess
        )) throw DbException(LT("in type_enum_lst invalid model id: %d and type id: %d: not found in type_dic"), rowIt->modelId, rowIt->typeId);

        vector<TypeEnumLstRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.typeEnum.cend() && TypeEnumLstRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in type_enum_lst not unique model id: %d, type id: %d and enum id: %d"), rowIt->modelId, rowIt->typeId, rowIt->enumId);
    }

    // sort type enum by unique key: model id, type id, enum name
    sort(
        io_metaRows.typeEnum.begin(),
        io_metaRows.typeEnum.end(),
        [](const TypeEnumLstRow & i_left, const TypeEnumLstRow & i_right) -> bool {
        return
            (i_left.modelId < i_right.modelId) ||
            (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId) ||
            (i_left.modelId == i_right.modelId && i_left.typeId == i_right.typeId && i_left.name < i_right.name);
    });

    // type_enum_lst table
    // unique: model id, type id, enum name; 
    for (vector<TypeEnumLstRow>::const_iterator rowIt = io_metaRows.typeEnum.cbegin(); rowIt != io_metaRows.typeEnum.cend(); ++rowIt) {

        vector<TypeEnumLstRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.typeEnum.cend() &&
            rowIt->modelId == nextIt->modelId && rowIt->typeId == nextIt->typeId && rowIt->name == nextIt->name)
            throw DbException(LT("in type_enum_lst not unique model id: %d, type id: %d and enum name: %s"), rowIt->modelId, rowIt->typeId, rowIt->name.c_str());
    }

    // restore primary key sort order
    sort(io_metaRows.typeEnum.begin(), io_metaRows.typeEnum.end(), TypeEnumLstRow::isKeyLess);

    // type_enum_txt table
    // unique: model id, type id, enum id, language; master key: model id, type id, enum id; foreign key: language code;
    // cleanup cr or lf in description and notes
    for (vector<TypeEnumTxtLangRow>::iterator rowIt = io_metaRows.typeEnumTxt.begin(); rowIt != io_metaRows.typeEnumTxt.end(); ++rowIt) {

        TypeEnumLstRow mkRow(rowIt->modelId, rowIt->typeId, rowIt->enumId);
        if (!std::binary_search(
            io_metaRows.typeEnum.cbegin(),
            io_metaRows.typeEnum.cend(),
            mkRow,
            TypeEnumLstRow::isKeyLess
        )) throw DbException(LT("in type_enum_txt invalid model id: %d, type id: %d and enum id: %d not found in type_enum_lst"), rowIt->modelId, rowIt->typeId, rowIt->enumId);

        vector<TypeEnumTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.typeEnumTxt.cend() && TypeEnumTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in type_enum_txt not unique model id: %d, type id: %d enum id: %d and language: %s"), rowIt->modelId, rowIt->typeId, rowIt->enumId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in type_enum_txt invalid model id: %d, type id: %d enum id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->typeId, rowIt->enumId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // parameter_dic table
    // unique: model id, parameter id; unique: model id, parameter name; 
    // master key: model id; foreign key: model id, type id;
    for (vector<ParamDicRow>::const_iterator rowIt = io_metaRows.paramDic.cbegin(); rowIt != io_metaRows.paramDic.cend(); ++rowIt) {

        if (rowIt->modelId != mId)
            throw DbException(LT("in parameter_dic invalid model id: %d, expected: %d in row with type id: %d"), rowIt->modelId, mId, rowIt->typeId);

        TypeDicRow fkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            fkRow,
            TypeDicRow::isKeyLess
        )) throw DbException(LT("in parameter_dic invalid model id: %d and type id: %d: not found in type_dic"), rowIt->modelId, rowIt->typeId);

        vector<ParamDicRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.paramDic.cend() && ParamDicRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in parameter_dic not unique model id: %d and parameter id: %d"), rowIt->modelId, rowIt->paramId);

        if (std::any_of(
            io_metaRows.paramDic.cbegin(),
            io_metaRows.paramDic.cend(),
            [rowIt](const ParamDicRow & i_row) -> bool {
            return
                i_row.modelId == rowIt->modelId && i_row.paramId != rowIt->paramId && i_row.paramName == rowIt->paramName;
        }
        )) throw DbException(LT("in parameter_dic not unique model id: %d and parameter name: %s"), rowIt->modelId, rowIt->paramName.c_str());
    }

    // model_parameter_import table
    // unique: model id, parameter id, from_name, from_model_name
    // master key: model id, parameter id;
    for (vector<ParamImportRow>::const_iterator rowIt = io_metaRows.paramImport.cbegin(); rowIt != io_metaRows.paramImport.cend(); ++rowIt) {

        if (rowIt->modelId != mId)
            throw DbException(LT("in model_parameter_import invalid model id: %d, expected: %d in row with parameter id: %d"), rowIt->modelId, mId, rowIt->paramId);

        ParamDicRow mkRow(rowIt->modelId, rowIt->paramId);
        if (!std::binary_search(
            io_metaRows.paramDic.cbegin(),
            io_metaRows.paramDic.cend(),
            mkRow,
            ParamDicRow::isKeyLess
        )) throw DbException(LT("in model_parameter_import invalid model id: %d and parameter id: %d: not found in parameter_dic"), rowIt->modelId, rowIt->paramId);

        vector<ParamImportRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.paramImport.cend() && ParamImportRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in model_parameter_import not unique model id: %d and parameter id: %d"), rowIt->modelId, rowIt->paramId);
    }

    // parameter_dic_txt table
    // unique: model id, parameter id, language; master key: model id, parameter id; foreign key: language code;
    // cleanup cr or lf in description and notes
    for (vector<ParamDicTxtLangRow>::iterator rowIt = io_metaRows.paramTxt.begin(); rowIt != io_metaRows.paramTxt.end(); ++rowIt) {

        ParamDicRow mkRow(rowIt->modelId, rowIt->paramId);
        if (!std::binary_search(
            io_metaRows.paramDic.cbegin(),
            io_metaRows.paramDic.cend(),
            mkRow,
            ParamDicRow::isKeyLess
        )) throw DbException(LT("in parameter_dic_txt invalid model id: %d and parameter id: %d: not found in parameter_dic"), rowIt->modelId, rowIt->paramId);

        vector<ParamDicTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.paramTxt.cend() && ParamDicTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in parameter_dic_txt not unique model id: %d, parameter id: %d and language: %s"), rowIt->modelId, rowIt->paramId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in parameter_dic_txt invalid model id: %d, parameter id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->paramId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // parameter_dims table
    // unique: model id, parameter id, dimension name; master key: model id, parameter id; foreign key: model id, type id;
    for (vector<ParamDimsRow>::const_iterator rowIt = io_metaRows.paramDims.cbegin(); rowIt != io_metaRows.paramDims.cend(); ++rowIt) {

        ParamDicRow mkRow(rowIt->modelId, rowIt->paramId);
        if (!std::binary_search(
            io_metaRows.paramDic.cbegin(),
            io_metaRows.paramDic.cend(),
            mkRow,
            ParamDicRow::isKeyLess
        )) throw DbException(LT("in parameter_dims invalid model id: %d and parameter id: %d: not found in parameter_dic"), rowIt->modelId, rowIt->paramId);

        TypeDicRow fkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            fkRow,
            TypeDicRow::isKeyLess
        )) throw DbException(LT("in parameter_dims invalid model id: %d and type id: %d: not found in type_dic"), rowIt->modelId, rowIt->typeId);

        vector<ParamDimsRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.paramDims.cend() && ParamDimsRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in parameter_dims not unique model id: %d, parameter id: %d and dimension name: %s"), rowIt->modelId, rowIt->paramId, rowIt->name.c_str());
    }

    // parameter_dims_txt table
    // unique: model id, parameter id, dimension id, language; master key: model id, parameter id, dimension name
    // foreign key: language code;
    // cleanup cr or lf in description and notes
    for (vector<ParamDimsTxtLangRow>::iterator rowIt = io_metaRows.paramDimsTxt.begin(); rowIt != io_metaRows.paramDimsTxt.end(); ++rowIt) {

        ParamDimsRow mkRow(rowIt->modelId, rowIt->paramId, rowIt->dimId);
        if (!std::binary_search(
            io_metaRows.paramDims.cbegin(),
            io_metaRows.paramDims.cend(),
            mkRow,
            ParamDimsRow::isKeyLess
        )) throw DbException(LT("in parameter_dims_txt invalid model id: %d parameter id: %d and dimension id: %s: not found in parameter_dims"), rowIt->modelId, rowIt->paramId, rowIt->dimId);

        vector<ParamDimsTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.paramDimsTxt.cend() && ParamDimsTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in parameter_dims_txt not unique model id: %d, parameter id: %d, dimension id: %d and language: %s"), rowIt->modelId, rowIt->paramId, rowIt->dimId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in parameter_dims_txt invalid model id: %d, parameter id: %d, dimension id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->paramId, rowIt->dimId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // table_dic table
    // unique: model id, table id; unique: model id, table name; master key: model id
    for (vector<TableDicRow>::const_iterator rowIt = io_metaRows.tableDic.cbegin(); rowIt != io_metaRows.tableDic.cend(); ++rowIt) {

        if (rowIt->modelId != mId)
            throw DbException(LT("in table_dic invalid model id: %d, expected: %d in row with table id: %d, name: %s"), rowIt->modelId, mId, rowIt->tableId, rowIt->tableName.c_str());

        vector<TableDicRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableDic.cend() && TableDicRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in table_dic not unique model id: %d and table id: %d"), rowIt->modelId, rowIt->tableId);

        if (std::any_of(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            [rowIt](const TableDicRow & i_row) -> bool {
            return
                i_row.modelId == rowIt->modelId && i_row.tableId != rowIt->tableId && i_row.tableName == rowIt->tableName;
        }
        )) throw DbException(LT("in table_dic not unique model id: %d and table name: %s"), rowIt->modelId, rowIt->tableName.c_str());
    }

    // table_dic_txt table
    // unique: model id, table id, language; master key: model id, table id; foreign key: language code;
    // cleanup cr or lf in description, notes and expression dimension description, notes
    for (vector<TableDicTxtLangRow>::iterator rowIt = io_metaRows.tableTxt.begin(); rowIt != io_metaRows.tableTxt.end(); ++rowIt) {

        TableDicRow mkRow(rowIt->modelId, rowIt->tableId);
        if (!std::binary_search(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            mkRow,
            TableDicRow::isKeyLess
        )) throw DbException(LT("in table_dic_txt invalid model id: %d and table id: %d: not found in table_dic"), rowIt->modelId, rowIt->tableId);

        vector<TableDicTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableTxt.cend() && TableDicTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in table_dic_txt not unique model id: %d, table id: %d and language: %s"), rowIt->modelId, rowIt->tableId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in table_dic_txt invalid model id: %d, table id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->tableId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
        blankCrLf(rowIt->exprDescr);
        blankCrLf(rowIt->exprNote);
    }

    // table_dims table
    // unique: model id, table id, dimension id; unique: model id, table id, dimension name;
    // master key: model id, table id; foreign key: model id, type id;
    for (vector<TableDimsRow>::const_iterator rowIt = io_metaRows.tableDims.cbegin(); rowIt != io_metaRows.tableDims.cend(); ++rowIt) {

        TableDicRow mkRow(rowIt->modelId, rowIt->tableId);
        if (!std::binary_search(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            mkRow,
            TableDicRow::isKeyLess
        )) throw DbException(LT("in table_dims invalid model id: %d and table id: %d: not found in table_dic"), rowIt->modelId, rowIt->tableId);

        TypeDicRow fkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            fkRow,
            TypeDicRow::isKeyLess
        )) throw DbException(LT("in table_dims invalid model id: %d and type id: %d: not found in type_dic"), rowIt->modelId, rowIt->typeId);

        vector<TableDimsRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableDims.cend() && TableDimsRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in table_dims not unique model id: %d, table id: %d and dimension id: %d"), rowIt->modelId, rowIt->tableId, rowIt->dimId);

        if (std::any_of(
            io_metaRows.tableDims.cbegin(),
            io_metaRows.tableDims.cend(),
            [rowIt](const TableDimsRow & i_row) -> bool {
            return
                i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId && i_row.dimId != rowIt->dimId &&
                i_row.name == rowIt->name;
        }
        )) throw DbException(LT("in table_dims not unique model id: %d, table id: %d and dimension name: %s"), rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
    }

    // table_dims_txt table
    // unique: model id, table id, dimension id, language; master key: model id, table id, dimension name;
    // foreign key: language code;
    // cleanup cr or lf in description and notes
    for (vector<TableDimsTxtLangRow>::iterator rowIt = io_metaRows.tableDimsTxt.begin(); rowIt != io_metaRows.tableDimsTxt.end(); ++rowIt) {

        TableDimsRow mkRow(rowIt->modelId, rowIt->tableId, rowIt->dimId);
        if (!std::binary_search(
            io_metaRows.tableDims.cbegin(),
            io_metaRows.tableDims.cend(),
            mkRow,
            TableDimsRow::isKeyLess
        )) throw DbException(LT("in table_dims_txt invalid model id: %d table id: %d and dimension id: %s: not found in table_dims"), rowIt->modelId, rowIt->tableId, rowIt->dimId);

        vector<TableDimsTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableDimsTxt.cend() && TableDimsTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in table_dims_txt not unique model id: %d, table id: %d, dimension id: %d and language: %s"), rowIt->modelId, rowIt->tableId, rowIt->dimId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in table_dims_txt invalid model id: %d, table id: %d, dimension id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->tableId, rowIt->dimId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // table_acc table
    // master key: model id, table id;
    // unique: model id, table id, accumulator id; unique: model id, table id, accumulator name; 
    for (vector<TableAccRow>::const_iterator rowIt = io_metaRows.tableAcc.cbegin(); rowIt != io_metaRows.tableAcc.cend(); ++rowIt) {

        TableDicRow mkRow(rowIt->modelId, rowIt->tableId);
        if (!std::binary_search(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            mkRow,
            TableDicRow::isKeyLess
        )) throw DbException(LT("in table_acc invalid model id: %d and table id: %d: not found in table_dic"), rowIt->modelId, rowIt->tableId);

        vector<TableAccRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableAcc.cend() && TableAccRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in table_acc not unique model id: %d, table id: %d and accumulator id: %d"), rowIt->modelId, rowIt->tableId, rowIt->accId);

        if (std::any_of(
            io_metaRows.tableAcc.cbegin(),
            io_metaRows.tableAcc.cend(),
            [rowIt](const TableAccRow & i_row) -> bool {
            return
                i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId && i_row.accId != rowIt->accId &&
                i_row.name == rowIt->name;
        }
        )) throw DbException(LT("in table_acc not unique model id: %d, table id: %d and accumulator name: %s"), rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
    }

    // table_acc_txt table
    // unique: model id, table id, accumulator id, language; master key: model id, table id, accumulator id;
    // foreign key: language code;
    // cleanup cr or lf in description and notes
    for (vector<TableAccTxtLangRow>::iterator rowIt = io_metaRows.tableAccTxt.begin(); rowIt != io_metaRows.tableAccTxt.end(); ++rowIt) {

        TableAccRow mkRow(rowIt->modelId, rowIt->tableId, rowIt->accId);
        if (!std::binary_search(
            io_metaRows.tableAcc.cbegin(),
            io_metaRows.tableAcc.cend(),
            mkRow,
            TableAccRow::isKeyLess
        )) throw DbException(LT("in table_acc_txt invalid model id: %d table id: %d and accumulator id: %d: not found in table_acc"), rowIt->modelId, rowIt->tableId, rowIt->accId);

        vector<TableAccTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableAccTxt.cend() && TableAccTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in table_acc_txt not unique model id: %d, table id: %d, accumulator id: %d and language: %s"), rowIt->modelId, rowIt->tableId, rowIt->accId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in table_acc_txt invalid model id: %d, table id: %d, accumulator id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->tableId, rowIt->accId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // table_expr table
    // unique: model id, table id, expr id; unique: model id, table id, expr name;
    // master key: model id, table id;
    for (vector<TableExprRow>::const_iterator rowIt = io_metaRows.tableExpr.cbegin(); rowIt != io_metaRows.tableExpr.cend(); ++rowIt) {

        TableDicRow mkRow(rowIt->modelId, rowIt->tableId);
        if (!std::binary_search(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            mkRow,
            TableDicRow::isKeyLess
        )) throw DbException(LT("in table_expr invalid model id: %d and table id: %d: not found in table_dic"), rowIt->modelId, rowIt->tableId);

        vector<TableExprRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableExpr.cend() && TableExprRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in table_expr not unique model id: %d, table id: %d and expr id: %d"), rowIt->modelId, rowIt->tableId, rowIt->exprId);

        if (std::any_of(
            io_metaRows.tableExpr.cbegin(),
            io_metaRows.tableExpr.cend(),
            [rowIt](const TableExprRow & i_row) -> bool {
            return
                i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId && i_row.exprId != rowIt->exprId &&
                i_row.name == rowIt->name;
        }
        )) throw DbException(LT("in table_expr not unique model id: %d, table id: %d and expr name: %s"), rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
    }

    // table_expr_txt table
    // unique: model id, table id, expr id, language; master key: model id, table id, expr id;
    // foreign key: language code;
    // cleanup cr or lf in description and notes
    for (vector<TableExprTxtLangRow>::iterator rowIt = io_metaRows.tableExprTxt.begin(); rowIt != io_metaRows.tableExprTxt.end(); ++rowIt) {

        TableExprRow mkRow(rowIt->modelId, rowIt->tableId, rowIt->exprId);
        if (!std::binary_search(
            io_metaRows.tableExpr.cbegin(),
            io_metaRows.tableExpr.cend(),
            mkRow,
            TableExprRow::isKeyLess
        )) throw DbException(LT("in table_expr_txt invalid model id: %d table id: %d and expr id: %d: not found in table_expr"), rowIt->modelId, rowIt->tableId, rowIt->exprId);

        vector<TableExprTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableExprTxt.cend() && TableExprTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in table_expr_txt not unique model id: %d, table id: %d, expr id: %d and language: %s"), rowIt->modelId, rowIt->tableId, rowIt->exprId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in table_expr_txt invalid model id: %d, table id: %d, expr id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->tableId, rowIt->exprId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // group_lst table
    // unique: model id, group id; master key: model id
    for (vector<GroupLstRow>::const_iterator rowIt = io_metaRows.groupLst.cbegin(); rowIt != io_metaRows.groupLst.cend(); ++rowIt) {

        if (rowIt->modelId != mId)
            throw DbException(LT("in group_lst invalid model id: %d, expected: %d in row with group id: %d, name: %s"), rowIt->modelId, mId, rowIt->groupId, rowIt->name.c_str());

        vector<GroupLstRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.groupLst.cend() && GroupLstRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in group_lst not unique model id: %d and group id: %d"), rowIt->modelId, rowIt->groupId);
    }

    // group_txt table
    // unique: model id, group id, language; master key: model id, group id; foreign key: language code;
    // cleanup cr or lf in description and notes
    for (vector<GroupTxtLangRow>::iterator rowIt = io_metaRows.groupTxt.begin(); rowIt != io_metaRows.groupTxt.end(); ++rowIt) {

        GroupLstRow mkRow(rowIt->modelId, rowIt->groupId);
        if (!std::binary_search(
            io_metaRows.groupLst.cbegin(),
            io_metaRows.groupLst.cend(),
            mkRow,
            GroupLstRow::isKeyLess
        )) throw DbException(LT("in group_txt invalid model id: %d and group id: %d: not found in group_lst"), rowIt->modelId, rowIt->groupId);

        vector<GroupTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.groupTxt.cend() && GroupTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in group_txt not unique model id: %d, group id: %d and language: %s"), rowIt->modelId, rowIt->groupId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        if (!std::binary_search(
            io_metaRows.langLst.cbegin(),
            io_metaRows.langLst.cend(),
            langRow,
            LangLstRow::isCodeLess
        )) throw DbException(LT("in group_txt invalid model id: %d, group id: %d and language: %s: not found in lang_lst"), rowIt->modelId, rowIt->groupId, rowIt->langCode.c_str());

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // group_pc table
    // unique: model id, group id, child position; master key: model id, group id
    for (vector<GroupPcRow>::const_iterator rowIt = io_metaRows.groupPc.cbegin(); rowIt != io_metaRows.groupPc.cend(); ++rowIt) {

        GroupLstRow mkRow(rowIt->modelId, rowIt->groupId);
        if (!std::binary_search(
            io_metaRows.groupLst.cbegin(),
            io_metaRows.groupLst.cend(),
            mkRow,
            GroupLstRow::isKeyLess
        )) throw DbException(LT("in group_pc invalid model id: %d and group id: %d: not found in group_lst"), rowIt->modelId, rowIt->groupId);

        vector<GroupPcRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.groupPc.cend() && GroupPcRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in group_pc not unique model id: %d, group id: %d and child position: %d"), rowIt->modelId, rowIt->groupId, rowIt->childPos);
    }
}

// validate workset metadata: uniqueness and referential integrity
void ModelSqlBuilder::prepareWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet) const
{
    // trim workset name and set workset_lst row field values: readonly status and creation date-time
    locale loc("");
    io_metaSet.worksetRow.name = trim(io_metaSet.worksetRow.name, loc);

    setWorksetRow(io_metaSet.worksetRow);

    int mId = i_metaRows.modelDic.modelId;
    int setId = io_metaSet.worksetRow.setId;

    // workset_lst table, update model id with actual db value
    io_metaSet.worksetRow.modelId = mId;

    // workset_txt table
    // unique: set id and language name; master key: set id; 
    // set language id by language code
    // cleanup cr or lf in description and notes
    for (vector<WorksetTxtLangRow>::iterator rowIt = io_metaSet.worksetTxt.begin(); rowIt != io_metaSet.worksetTxt.end(); ++rowIt) {

        rowIt->langCode = trim(rowIt->langCode, loc);
        rowIt->descr = trim(rowIt->descr, loc);
        rowIt->note = trim(rowIt->note, loc);

        if (rowIt->setId != setId)
            throw DbException(LT("in workset_txt invalid set id: %d, expected: %d in row with language id: %d and name: %s"), rowIt->setId, setId, rowIt->langId, rowIt->langCode.c_str());

        vector<WorksetTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaSet.worksetTxt.cend() && WorksetTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in workset_txt not unique set id: %d and language: %s"), rowIt->setId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        const auto langIt = std::lower_bound(
            i_metaRows.langLst.cbegin(), i_metaRows.langLst.cend(), langRow, LangLstRow::isCodeLess
        );
        if (langIt == i_metaRows.langLst.cend() || langIt->code != rowIt->langCode)
            throw DbException(LT("in workset_txt invalid set id: %d and language: %s: not found in lang_lst"), rowIt->setId, rowIt->langCode.c_str());

        rowIt->langId = langIt->langId;

        blankCrLf(rowIt->descr);
        blankCrLf(rowIt->note);
    }

    // workset_parameter table
    // unique: set id, parameter id; master key: set id; 
    // foreign key: model id, parameter id;
    for (vector<WorksetParamRow>::iterator rowIt = io_metaSet.worksetParam.begin(); rowIt != io_metaSet.worksetParam.end(); ++rowIt) {

        rowIt->modelId = mId;   // update model id with actual db value

        if (rowIt->setId != setId)
            throw DbException(LT("in workset_parameter invalid set id: %d, expected: %d in row with parameter id: %d"), rowIt->setId, setId, rowIt->paramId);

        ParamDicRow fkRow(rowIt->modelId, rowIt->paramId);
        if (!std::binary_search(
            i_metaRows.paramDic.cbegin(),
            i_metaRows.paramDic.cend(),
            fkRow,
            ParamDicRow::isKeyLess
            ))
            throw DbException(LT("in workset_parameter invalid model id: %d and parameter id: %d: not found in parameter_dic"), rowIt->modelId, rowIt->paramId);

        vector<WorksetParamRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaSet.worksetParam.cend() && WorksetParamRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in workset_parameter not unique set id: %d and parameter id: %d"), rowIt->setId, rowIt->paramId);
    }

    // workset_parameter_txt table
    // unique: set id, parameter id, language; master key: set id, parameter id;
    // set language id by language code
    // cleanup cr or lf in parameter value notes
    for (vector<WorksetParamTxtLangRow>::iterator rowIt = io_metaSet.worksetParamTxt.begin(); rowIt != io_metaSet.worksetParamTxt.end(); ++rowIt) {

        rowIt->langCode = trim(rowIt->langCode, loc);
        rowIt->note = trim(rowIt->note, loc);

        rowIt->modelId = mId;   // update model id with actual db value

        WorksetParamRow mkRow(rowIt->setId, rowIt->paramId);
        if (!std::binary_search(
            io_metaSet.worksetParam.cbegin(),
            io_metaSet.worksetParam.cend(),
            mkRow,
            WorksetParamRow::isKeyLess
            ))
            throw DbException(LT("in workset_parameter_txt invalid set id: %d and parameter id: %d: not found in workset_parameter"), rowIt->setId, rowIt->paramId);

        vector<WorksetParamTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaSet.worksetParamTxt.cend() && WorksetParamTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException(LT("in workset_parameter_txt not unique set id: %d, parameter id: %d and language: %s"), rowIt->setId, rowIt->paramId, rowIt->langCode.c_str());

        LangLstRow langRow(rowIt->langCode);
        const auto langIt = std::lower_bound(
            i_metaRows.langLst.cbegin(), i_metaRows.langLst.cend(), langRow, LangLstRow::isCodeLess
        );
        if (langIt == i_metaRows.langLst.cend() || langIt->code != rowIt->langCode)
            throw DbException(LT("in workset_txt invalid set id: %d and language: %s: not found in lang_lst"), rowIt->setId, rowIt->langCode.c_str());

        rowIt->langId = langIt->langId;

        blankCrLf(rowIt->note);
    }
}

// setup names for dimension and accumulator columns, i.e.: "dim0", "acc1", "expr2"
void ModelSqlBuilder::setColumnNames(MetaModelHolder & io_metaRows)
{
    // set name for parameter dimension columns if not specified
    for (ParamDimsRow & dimRow : io_metaRows.paramDims) {
        if (dimRow.name.empty()) dimRow.name = "dim" + to_string(dimRow.dimId);
    }

    // set name for parameter dimension columns if not specified
    for (TableDimsRow & dimRow : io_metaRows.tableDims) {
        if (dimRow.name.empty()) dimRow.name = "dim" + to_string(dimRow.dimId);
    }
    
    // set name for output table accumulator columns if not specified
    for (TableAccRow & accRow : io_metaRows.tableAcc) {
        if (accRow.name.empty()) accRow.name = "acc" + to_string(accRow.accId);
    }

    // set name for output table accumulator columns if not specified
    for (TableExprRow & exprRow : io_metaRows.tableExpr) {
        if (exprRow.name.empty()) exprRow.name = "expr" + to_string(exprRow.exprId);
    }
}

// set field values for model_dic table row
void ModelSqlBuilder::setModelDicRow(ModelDicRow & io_mdRow) const
{
    // validate model name
    if (io_mdRow.name.empty()) throw DbException(LT("invalid (empty) model name"));
    if (io_mdRow.name.length() > 255) throw DbException(LT("invalid (too long) model name: %s"), io_mdRow.name.c_str());

    // set model create date-time, if required
    if (io_mdRow.createDateTime.empty()) io_mdRow.createDateTime = toDateTimeString(theLog->timeStamp());
}

// set workset_lst row field values: readonly status and creation date-time
void ModelSqlBuilder::setWorksetRow(WorksetLstRow & io_wsRow) const
{ 
    // validate workset name
    if (io_wsRow.name.empty()) throw DbException(LT("invalid (empty) workset name"));
    if (io_wsRow.name.length() > 255) throw DbException(LT("invalid (too long) workset name: %s"), io_wsRow.name.c_str());

    // validate workset digest
    /* that check will done at insert time
    if (io_wsRow.digest.empty()) throw DbException(LT("invalid (or empty) workset digest"));
    if (io_wsRow.digest.length() > 32) throw DbException(LT("invalid (too long) workset digest: %s"), io_wsRow.digest.c_str());
    */

    // mark workset as read-write and set workset creation date-time
    io_wsRow.isReadonly = false;
    io_wsRow.updateDateTime = toDateTimeString(theLog->timeStamp());
}

// setup type rows, i.e. type digest
void ModelSqlBuilder::setTypeDicRows(MetaModelHolder & io_metaRows) const
{
    // make digest for all types
    for (TypeDicRow & typeRow : io_metaRows.typeDic) {
        typeRow.digest = makeTypeDigest(typeRow, io_metaRows);
    }
}

// collect info for db parameter tables
void ModelSqlBuilder::setParamTableInfo(MetaModelHolder & io_metaRows)
{
    // collect table information for all parameters
    for (ParamDicRow & paramRow : io_metaRows.paramDic) {

        ParamTblInfo tblInf;
        tblInf.modelId = io_metaRows.modelDic.modelId;
        tblInf.id = paramRow.paramId;
        tblInf.name = paramRow.paramName;
        tblInf.isNullable = paramRow.isExtendable;

        // collect dimension names
        tblInf.dimNameVec.clear();
        for (const ParamDimsRow & dimRow : io_metaRows.paramDims) {
            if (dimRow.paramId == tblInf.id) tblInf.dimNameVec.push_back(dimRow.name);
        }

        // find parameter value type
        tblInf.valueTypeIt = TypeDicRow::byKey(paramRow.modelId, paramRow.typeId, io_metaRows.typeDic);
        if (tblInf.valueTypeIt == io_metaRows.typeDic.cend())
            throw DbException(LT("invalid type id: %d for parameter id: %d, name: %s"), paramRow.typeId, paramRow.paramId, paramRow.paramName.c_str());

        paramInfoVec.push_back(tblInf);     // add to parameters info vector
    }
}

// collect info for db accumulator tables and value views
void ModelSqlBuilder::setOutTableInfo(MetaModelHolder & io_metaRows)
{
    // collect information for all output tables
    for (TableDicRow & tableRow : io_metaRows.tableDic) {

        OutTblInfo tblInf;
        tblInf.modelId = io_metaRows.modelDic.modelId;
        tblInf.id = tableRow.tableId;
        tblInf.name = tableRow.tableName;

        // collect dimension names
        tblInf.dimNameVec.clear();
        for (const TableDimsRow & dimRow : io_metaRows.tableDims) {
            if (dimRow.tableId == tblInf.id) tblInf.dimNameVec.push_back(dimRow.name);
        }

        // collect accumulator names
        tblInf.accNameVec.clear();
        for (const TableAccRow & accRow : io_metaRows.tableAcc) {
            if (accRow.tableId == tblInf.id) {
                tblInf.accIdVec.push_back(accRow.accId);
                tblInf.accNameVec.push_back(accRow.name);
            }
        }
        if (tblInf.accNameVec.empty()) 
            throw DbException(LT("output table accumulators not found for table: %s"), tableRow.tableName.c_str());

        // translate native accumulators into sql subqueries
        ModelAccumulatorSql ae(tableRow.dbAccTable, tblInf.dimNameVec, tblInf.accIdVec, tblInf.accNameVec);

        map<string, string> nm;     // native accumulators map of (name, sql subquery)
        bool isFirst = true;
        for (TableAccRow & accRow : io_metaRows.tableAcc) {
            if (accRow.tableId == tblInf.id && !accRow.isDerived) {
                accRow.accSql = ae.translateNativeAccExpr(accRow.accId, isFirst);
                isFirst = false;
                nm[accRow.name] = accRow.accSql;
            }
        }

        // translate derived accumulators into sql subqueries
        for (TableAccRow & accRow : io_metaRows.tableAcc) {
            if (accRow.tableId == tblInf.id && accRow.isDerived) {
                accRow.accSql = ae.translateDerivedAccExpr(accRow.name, accRow.accSrc, nm);
            }
        }

        // translate expressions into sql
        ModelAggregationSql aggr(tableRow.dbAccTable, tblInf.dimNameVec, tblInf.accIdVec, tblInf.accNameVec);

        for (TableExprRow & exprRow : io_metaRows.tableExpr) {
            if (exprRow.tableId == tblInf.id) {
                exprRow.sqlExpr = aggr.translateAggregationExpr(exprRow.name, exprRow.srcExpr); // translate expression to sql aggregation
            }
        }
        // if (tblInf.exprVec.empty()) 
        //    throw DbException(LT("output table aggregation expressions not found for table: %s"), tableRow.tableName.c_str());

        outInfoVec.push_back(tblInf);   // add to output tables info vector
    }
}

// make prefix part of db table name by shorten source name, ie: ageSexProvince => ageSexPr
const string ModelSqlBuilder::makeDbNamePrefix(int i_id, const string & i_src) const
{
    if (i_src.empty()) throw DbException(LT("invalid (empty) source name, id: %d"), i_id);

    // in db table name use only [A-Z,a-z,0-9] and _ underscore
    // make sure name size not longer than (32 - max prefix size)
    return toAlphaNumeric(i_src, dbPrefixSize);
}

// make unique part of db table name by using digest or crc32(digest)
const string ModelSqlBuilder::makeDbNameSuffix(int i_id, const string & i_src, const string i_digest) const
{
    if (i_digest.empty()) throw DbException(LT("invalid (empty) digest for: %s, id: %d"), i_src.c_str(), i_id);

    return isCrc32Name ? crc32String(i_digest) : i_digest;
}

// calculate type digest
const string ModelSqlBuilder::makeTypeDigest(const TypeDicRow & i_typeRow, const MetaModelHolder & i_metaRows)
{
    // for built-in types use _name_ as digest, ie: _int_ or _Time_
    if (i_typeRow.isBuiltIn()) return "_" + i_typeRow.name + "_";

    // make type digest header as type name and kind of type
    MD5 md5;

    md5.add("type_name,dic_id,total_enum_id\n", strlen("type_name,dic_id,total_enum_id\n"));
    string sLine = i_typeRow.name + "," + to_string(i_typeRow.dicId) + "," + to_string(i_typeRow.totalEnumId) + "\n";
    md5.add(sLine.c_str(), sLine.length());

    // add to digest list of all enum id, enum value
    md5.add("enum_id,enum_name\n", strlen("enum_id,enum_name\n"));

    TypeEnumLstRow fkRow(i_typeRow.modelId, i_typeRow.typeId, 0);
    const auto enumRowIt = std::lower_bound(
        i_metaRows.typeEnum.cbegin(), i_metaRows.typeEnum.cend(), fkRow, TypeEnumLstRow::isKeyLess
    );
    if (enumRowIt == i_metaRows.typeEnum.cend() || enumRowIt->modelId != i_typeRow.modelId || enumRowIt->typeId != i_typeRow.typeId) 
        throw DbException(LT("in type_enum_lst invalid model id: %d and type id: %d: not found in type_dic"), i_typeRow.modelId, i_typeRow.typeId);

    for (vector<TypeEnumLstRow>::const_iterator rowIt = enumRowIt; 
        rowIt != i_metaRows.typeEnum.cend() && rowIt->modelId == i_typeRow.modelId && rowIt->typeId == i_typeRow.typeId;
        ++rowIt) {

        // add enum to type digest
        sLine = to_string(rowIt->enumId) + "," + rowIt->name + "\n";
        md5.add(sLine.c_str(), sLine.length());
    }

    return md5.getHash();
}

// calculate parameter metadata digest and parameter import digest
const tuple<string, string> ModelSqlBuilder::makeParamDigest(const ParamDicRow & i_paramRow, const MetaModelHolder & i_metaRows)
{ 
    // find parameter type
    TypeDicRow typeFkRow(i_paramRow.modelId, i_paramRow.typeId);
    auto typeRowIt = std::lower_bound(
        i_metaRows.typeDic.cbegin(), i_metaRows.typeDic.cend(), typeFkRow, TypeDicRow::isKeyLess
    );
    if (typeRowIt == i_metaRows.typeDic.cend() || typeRowIt->modelId != i_paramRow.modelId || typeRowIt->typeId != i_paramRow.typeId)
        throw DbException(LT("in parameter_dic invalid model id: %d and type id: %d: not found in type_dic"), i_paramRow.modelId, i_paramRow.typeId);

    // make parameter digest header as name, rank and parameter type digest
    MD5 md5Full;
    md5Full.add("parameter_name,parameter_rank,type_digest\n", strlen("parameter_name,parameter_rank,type_digest\n"));
    string sLine = i_paramRow.paramName + "," + to_string(i_paramRow.rank)  + "," + typeRowIt->digest + "\n";
    md5Full.add(sLine.c_str(), sLine.length());

    // import digest same as full parameter digest but does not include parameter name
    MD5 md5Imp;
    md5Imp.add("parameter_rank,type_digest\n", strlen("parameter_rank,type_digest\n"));
    sLine = to_string(i_paramRow.rank) + "," + typeRowIt->digest + "\n";
    md5Imp.add(sLine.c_str(), sLine.length());

    // add dimensions: id, name, size and dimension type digest
    sLine = "dim_id,dim_name,dim_size,type_digest\n";
    md5Full.add(sLine.c_str(), sLine.length());
    md5Imp.add(sLine.c_str(), sLine.length());

    if (i_paramRow.rank > 0) {

        ParamDimsRow dimFkRow(i_paramRow.modelId, i_paramRow.paramId, 0);
        const auto dimRowIt = std::lower_bound(
            i_metaRows.paramDims.cbegin(),
            i_metaRows.paramDims.cend(),
            dimFkRow,
            [](const ParamDimsRow & i_left, const ParamDimsRow & i_right) -> bool { // first parameter dimension (lowest dim_id)
                return
                    (i_left.modelId < i_right.modelId) ||
                    (i_left.modelId == i_right.modelId && i_left.paramId < i_right.paramId);
            }
        );
        if (dimRowIt == i_metaRows.paramDims.cend() || dimRowIt->modelId != i_paramRow.modelId || dimRowIt->paramId != i_paramRow.paramId)
            throw DbException(LT("in parameter_dims invalid model id: %d and parameter id: %d: not found in parameter_dic"), i_paramRow.modelId, i_paramRow.paramId);

        for (vector<ParamDimsRow>::const_iterator rowIt = dimRowIt; 
            rowIt != i_metaRows.paramDims.cend() && rowIt->modelId == i_paramRow.modelId && rowIt->paramId == i_paramRow.paramId; 
            ++rowIt) {

            // find dimension type
            TypeDicRow tRow(rowIt->modelId, rowIt->typeId);
            typeRowIt = std::lower_bound(
                i_metaRows.typeDic.cbegin(), i_metaRows.typeDic.cend(), tRow, TypeDicRow::isKeyLess
            );
            if (typeRowIt == i_metaRows.typeDic.cend() || typeRowIt->modelId != rowIt->modelId || typeRowIt->typeId != rowIt->typeId)
                throw DbException(LT("in parameter_dims invalid model id: %d and type id: %d: not found in type_dic"), rowIt->modelId, rowIt->typeId);

            // find first enum of that type 
            TypeEnumLstRow enumFkRow(typeRowIt->modelId, typeRowIt->typeId, 0);
            const auto enumRowIt = std::lower_bound(
                i_metaRows.typeEnum.cbegin(),
                i_metaRows.typeEnum.cend(),
                enumFkRow,
                [](const TypeEnumLstRow & i_left, const TypeEnumLstRow & i_right) -> bool { // find first enum of the type (lowest enum id)
                    return
                        (i_left.modelId < i_right.modelId) ||
                        (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId);
                }
            );

            int dimSize = 0;
            for (vector<TypeEnumLstRow>::const_iterator it = enumRowIt;
                it != i_metaRows.typeEnum.cend() && it->modelId == typeRowIt->modelId && it->typeId == typeRowIt->typeId;
                ++it) {
                dimSize++;
            }

            // add dimension to digest: id, name, size, type digest
            sLine = to_string(rowIt->dimId) + "," + rowIt->name + "," + to_string(dimSize) + "," + typeRowIt->digest + "\n";
            md5Full.add(sLine.c_str(), sLine.length());
            md5Imp.add(sLine.c_str(), sLine.length());
        }
    }

    return { md5Full.getHash(), md5Imp.getHash() };
}

// calculate output table metadata digest and table import digest
const tuple<string, string> ModelSqlBuilder::makeOutTableDigest(const TableDicRow i_tableRow, const MetaModelHolder & i_metaRows)
{
    // make digest header as name and rank
    MD5 md5Full;
    md5Full.add("table_name,table_rank\n", strlen("table_name,table_rank\n"));
    string sLine = i_tableRow.tableName + "," + to_string(i_tableRow.rank) + "\n";
    md5Full.add(sLine.c_str(), sLine.length());

    // table import digest calculated as import digest of parameter with double type for that table
    MD5 md5Imp;
    md5Imp.add("parameter_rank,type_digest\n", strlen("parameter_rank,type_digest\n"));
    sLine = to_string(i_tableRow.rank) + ",_double_\n";
    md5Imp.add(sLine.c_str(), sLine.length());
    
    // add dimensions: id, name, size and dimension type digest
    sLine = "dim_id,dim_name,dim_size,type_digest\n";
    md5Full.add(sLine.c_str(), sLine.length());
    md5Imp.add(sLine.c_str(), sLine.length());

    if (i_tableRow.rank > 0) {

        TableDimsRow dimFkRow(i_tableRow.modelId, i_tableRow.tableId, 0);
        const auto dimRowIt = std::lower_bound(
            i_metaRows.tableDims.cbegin(), i_metaRows.tableDims.cend(), dimFkRow, TableDimsRow::isKeyLess
        );
        if (dimRowIt == i_metaRows.tableDims.cend() || dimRowIt->modelId != i_tableRow.modelId || dimRowIt->tableId != i_tableRow.tableId)
            throw DbException(LT("in table_dims invalid model id: %d and table id: %d: not found in table_dic"), i_tableRow.modelId, i_tableRow.tableId);

        for (vector<TableDimsRow>::const_iterator rowIt = dimRowIt; 
            rowIt != i_metaRows.tableDims.cend() && rowIt->modelId == i_tableRow.modelId && rowIt->tableId == i_tableRow.tableId;
            ++rowIt) {

            // find dimension type
            TypeDicRow tRow(rowIt->modelId, rowIt->typeId);
            auto typeRowIt = std::lower_bound(
                i_metaRows.typeDic.cbegin(), i_metaRows.typeDic.cend(), tRow, TypeDicRow::isKeyLess
            );
            if (typeRowIt == i_metaRows.typeDic.cend() || typeRowIt->modelId != rowIt->modelId || typeRowIt->typeId != rowIt->typeId)
                throw DbException(LT("in table_dims invalid model id: %d and type id: %d: not found in type_dic"), rowIt->modelId, rowIt->typeId);

            // add dimension to digest: id, name, size, type digest
            sLine = to_string(rowIt->dimId) + "," + rowIt->name + "," + to_string(rowIt->dimSize) + "," + typeRowIt->digest + "\n";
            md5Full.add(sLine.c_str(), sLine.length());
            md5Imp.add(sLine.c_str(), sLine.length());
        }
    }

    // add accumulators: id, name, source expression
    md5Full.add("acc_id,acc_name,acc_src\n", strlen("acc_id,acc_name,acc_src\n"));

    TableAccRow accFkRow(i_tableRow.modelId, i_tableRow.tableId, 0);
    const auto accRowIt = std::lower_bound(
        i_metaRows.tableAcc.cbegin(), i_metaRows.tableAcc.cend(), accFkRow, TableAccRow::isKeyLess
    );
    if (accRowIt == i_metaRows.tableAcc.cend() || accRowIt->modelId != i_tableRow.modelId || accRowIt->tableId != i_tableRow.tableId)
        throw DbException(LT("in table_acc invalid model id: %d and table id: %d: not found in table_dic"), i_tableRow.modelId, i_tableRow.tableId);

    for (vector<TableAccRow>::const_iterator rowIt = accRowIt; 
        rowIt != i_metaRows.tableAcc.cend() && rowIt->modelId == i_tableRow.modelId && rowIt->tableId == i_tableRow.tableId;
        ++rowIt) {

        // add accumulator to digest: id, name, expression
        sLine = to_string(rowIt->accId) + "," + rowIt->name + "," + rowIt->accSrc + "\n";
        md5Full.add(sLine.c_str(), sLine.length());
    }

    // add output table expressions: id, name, source expression
    md5Full.add("expr_id,expr_name,expr_src\n", strlen("expr_id,expr_name,expr_src\n"));

    TableExprRow exprFkRow(i_tableRow.modelId, i_tableRow.tableId, 0);
    const auto exprRowIt = std::lower_bound(
        i_metaRows.tableExpr.cbegin(), i_metaRows.tableExpr.cend(), exprFkRow, TableExprRow::isKeyLess
    );
    if (exprRowIt == i_metaRows.tableExpr.cend() || exprRowIt->modelId != i_tableRow.modelId || exprRowIt->tableId != i_tableRow.tableId)
        throw DbException(LT("in table_expr invalid model id: %d and table id: %d: not found in table_dic"), i_tableRow.modelId, i_tableRow.tableId);

    for (vector<TableExprRow>::const_iterator rowIt = exprRowIt; 
        rowIt != i_metaRows.tableExpr.cend() && rowIt->modelId == i_tableRow.modelId && rowIt->tableId == i_tableRow.tableId;
        ++rowIt) {

        // add table expression to output table digest: id, name, source expression
        sLine = to_string(rowIt->exprId) + "," + rowIt->name + "," + rowIt->srcExpr + "\n";
        md5Full.add(sLine.c_str(), sLine.length());
    }

    return { md5Full.getHash(), md5Imp.getHash() };
}

// calculate model metadata digest
const string ModelSqlBuilder::makeModelDigest(const MetaModelHolder & i_metaRows)
{
    // make digest header as name and kind of model
    MD5 md5;
    md5.add("model_name,model_type\n", strlen("model_name,model_type\n"));
    string sLine = i_metaRows.modelDic.name + "," + to_string(i_metaRows.modelDic.type) + "\n";
    md5.add(sLine.c_str(), sLine.length());

    // add digests of all model types
    md5.add("type_digest\n", strlen("type_digest\n"));

    for (const auto & typeRow : i_metaRows.typeDic) {
        if (typeRow.modelId == i_metaRows.modelDic.modelId) {
            sLine = typeRow.digest + "\n";
            md5.add(sLine.c_str(), sLine.length());
        }
    }

    // add digests of all model parameters
    md5.add("parameter_digest\n", strlen("parameter_digest\n"));

    for (const auto & paramRow : i_metaRows.paramDic) {
        if (paramRow.modelId == i_metaRows.modelDic.modelId) {
            sLine = paramRow.digest + "\n";
            md5.add(sLine.c_str(), sLine.length());
        }
    }

    // add digests of all model output tables
    md5.add("table_digest\n", strlen("table_digest\n"));

    for (const auto & tableRow : i_metaRows.tableDic) {
        if (tableRow.modelId == i_metaRows.modelDic.modelId) {
            sLine = tableRow.digest + "\n";
            md5.add(sLine.c_str(), sLine.length());
        }
    }

    return md5.getHash();
}
