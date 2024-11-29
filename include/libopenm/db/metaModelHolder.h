/**
 * @file
 * OpenM++ modeling library: holder struct for model metadata.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_META_MODEL_HOLDER_H
#define OM_META_MODEL_HOLDER_H

using namespace std;

#include "dbMetaRow.h"

namespace openm
{
    /** Holder for model metadata db rows. */
    struct MetaModelHolder
    {
    public:
        /** model_dic table row and default model language code */
        ModelDicLangRow modelDic;

        /** lang_lst table rows */
        vector<LangLstRow> langLst;

        /** model_dic_txt table rows and language code */
        vector<ModelDicTxtLangRow> modelTxt;

        /** type_dic table rows */
        vector<TypeDicRow> typeDic;

        /** type_dic_txt table rows and language code */
        vector<TypeDicTxtLangRow> typeTxt;

        /** type_enum_lst table rows */
        vector<TypeEnumLstRow> typeEnum;

        /** type_enum_txt table rows and language code */
        vector<TypeEnumTxtLangRow> typeEnumTxt;

        /** parameter_dic table rows */
        vector<ParamDicRow> paramDic;

        /** model_parameter_import table rows */
        vector<ParamImportRow> paramImport;

        /** parameter_dic_txt table rows and language code */
        vector<ParamDicTxtLangRow> paramTxt;

        /** parameter_dims table rows */
        vector<ParamDimsRow> paramDims;

        /** parameter_dims_txt table rows and language code */
        vector<ParamDimsTxtLangRow> paramDimsTxt;

        /** table_dic table rows */
        vector<TableDicRow> tableDic;

        /** table_dic_txt table rows and language code */
        vector<TableDicTxtLangRow> tableTxt;

        /** table_dims table rows */
        vector<TableDimsRow> tableDims;

        /** table_dims_txt table rows and language code */
        vector<TableDimsTxtLangRow> tableDimsTxt;

        /** table_acc table rows */
        vector<TableAccRow> tableAcc;

        /** table_acc_txt table rows and language code */
        vector<TableAccTxtLangRow> tableAccTxt;

        /** table_expr table rows */
        vector<TableExprRow> tableExpr;

        /** table_expr_txt table rows and language code */
        vector<TableExprTxtLangRow> tableExprTxt;

        /** entity_dic table rows */
        vector<EntityDicRow> entityDic;

        /** entity_dic_txt table rows and language code */
        vector<EntityDicTxtLangRow> entityTxt;

        /** entity_attr table rows */
        vector<EntityAttrRow> entityAttr;

        /** entity_attr_txt table rows and language code */
        vector<EntityAttrTxtLangRow> entityAttrTxt;

        /** group_lst table rows */
        vector<GroupLstRow> groupLst;

        /** group_txt table rows and language code */
        vector<GroupTxtLangRow> groupTxt;

        /** group_pc table rows */
        vector<GroupPcRow> groupPc;

        /** entity_group_lst table rows */
        vector<EntityGroupLstRow> entityGroupLst;

        /** entity_group_txt table rows and language code */
        vector<EntityGroupTxtLangRow> entityGroupTxt;

        /** entity_group_pc table rows */
        vector<EntityGroupPcRow> entityGroupPc;

        /** default model profile name */
        string profileName;

        /** default model proflie option rows: key, value pairs */
        NoCaseMap profileRows;

        /** memory prediction model profile option: value of the option memory_MB_constant_per_instance */
        static constexpr const char * constInstanceMemoryOption = "Memory.ProcessConstant";

        /** memory prediction model profile option: value of the option memory_MB_constant_per_sub */
        static constexpr const char * constSubMemoryOption = "Memory.SubValueConstant";

        /** memory prediction model profile option: parameter name containing population size */
        static constexpr const char * popSizeMemoryOption = "Memory.ScaleParameter";

        /** memory prediction model profile option: value of the option memory_MB_popsize_coefficient */
        static constexpr const char * popCoeffMemoryOption = "Memory.ScaleMultiplier";

        /** memory prediction model profile option: value of the option memory_safety_factor */
        static constexpr const char * safetyFactorMemoryOption = "Memory.SafetyMultiplier";
    };
}

#endif  // OM_META_MODEL_HOLDER_H
