// OpenM++ data library: db rows for model metadata
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
using namespace openm;

/** db-exception default error message: "unknown db error" */
const char openm::dbUnknownErrorMessage[] = "unknown db error";

// db-row base class
IRowBase::~IRowBase(void) throw() { }

// lang_lst row less comparator by primary key: language id
bool LangLstRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<LangLstRow *>(i_left.get())->langId < dynamic_cast<LangLstRow *>(i_right.get())->langId;
}

// lang_lst row equal comparator by primary key: language id
bool LangLstRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<LangLstRow *>(i_left.get())->langId == dynamic_cast<LangLstRow *>(i_right.get())->langId;
}

// lang_lst row row equal comparator by language code
bool LangLstRow::codeEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<LangLstRow *>(i_left.get())->code == dynamic_cast<LangLstRow *>(i_right.get())->code;
}

// lang_word row less comparator by primary key: language id and word code
bool LangWordRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const LangWordRow * left = dynamic_cast<LangWordRow *>(i_left.get());
    const LangWordRow * right = dynamic_cast<LangWordRow *>(i_right.get());
    return 
        (left->langId < right->langId) || 
        (left->langId == right->langId && left->code < right->code);
}

// lang_word row equal comparator by primary key: language id and word code
bool LangWordRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const LangWordRow * left = dynamic_cast<LangWordRow *>(i_left.get());
    const LangWordRow * right = dynamic_cast<LangWordRow *>(i_right.get());
    return 
        left->langId == right->langId && left->code == right->code;
}

// model_dic row less comparator by primary key: model id
bool ModelDicRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<ModelDicRow *>(i_left.get())->modelId < dynamic_cast<ModelDicRow *>(i_right.get())->modelId;
}

// model_dic row equal comparator by primary key: model id
bool ModelDicRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<ModelDicRow *>(i_left.get())->modelId == dynamic_cast<ModelDicRow *>(i_right.get())->modelId;
}

// model_dic row equal comparator by model name and timestamp
bool ModelDicRow::nameTimeStampEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ModelDicRow * left = dynamic_cast<ModelDicRow *>(i_left.get());
    const ModelDicRow * right = dynamic_cast<ModelDicRow *>(i_right.get());
    return 
        left->name == right->name && left->timestamp == right->timestamp;
}

// model_dic_txt row less comparator by primary key: model id and language id
bool ModelDicTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ModelDicTxtRow * left = dynamic_cast<ModelDicTxtRow *>(i_left.get());
    const ModelDicTxtRow * right = dynamic_cast<ModelDicTxtRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || (left->modelId == right->modelId && left->langId < right->langId);
}

// model_dic_txt row equal comparator by primary key: model id and language id
bool ModelDicTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ModelDicTxtRow * left = dynamic_cast<ModelDicTxtRow *>(i_left.get());
    const ModelDicTxtRow * right = dynamic_cast<ModelDicTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->langId == right->langId;
}

// model_dic_txt row equal comparator by language id
bool ModelDicTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<ModelDicTxtRow *>(i_left.get())->langId == dynamic_cast<ModelDicTxtRow *>(i_right.get())->langId;
}

// run_lst row less comparator by primary key: run id
bool RunLstRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<RunLstRow *>(i_left.get())->runId < dynamic_cast<RunLstRow *>(i_right.get())->runId;
}

// run_lst row equal comparator by primary key: run id
bool RunLstRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<RunLstRow *>(i_left.get())->runId == dynamic_cast<RunLstRow *>(i_right.get())->runId;
}

// run_txt row less comparator by primary key: run id and language id
bool RunTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const RunTxtRow * left = dynamic_cast<RunTxtRow *>(i_left.get());
    const RunTxtRow * right = dynamic_cast<RunTxtRow *>(i_right.get());
    return 
        (left->runId < right->runId) || (left->runId == right->runId && left->langId < right->langId);
}

// run_txt row equal comparator by primary key: run id and language id
bool RunTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const RunTxtRow * left = dynamic_cast<RunTxtRow *>(i_left.get());
    const RunTxtRow * right = dynamic_cast<RunTxtRow *>(i_right.get());
    return 
        left->runId == right->runId && left->langId == right->langId;
}

// run_txt row equal comparator by language id
bool RunTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<RunTxtRow *>(i_left.get())->langId == dynamic_cast<RunTxtRow *>(i_right.get())->langId;
}

// profile_lst row less comparator by primary key: profile name
bool ProfileLstRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<ProfileLstRow *>(i_left.get())->name < dynamic_cast<ProfileLstRow *>(i_right.get())->name;
}

// profile_lst row equal comparator by primary key: profile name
bool ProfileLstRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<ProfileLstRow *>(i_left.get())->name == dynamic_cast<ProfileLstRow *>(i_right.get())->name;
}

// profile_option row less comparator by primary key: profile name, option key
bool ProfileOptionRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ProfileOptionRow * left = dynamic_cast<ProfileOptionRow *>(i_left.get());
    const ProfileOptionRow * right = dynamic_cast<ProfileOptionRow *>(i_right.get());
    return 
        (left->name < right->name) || (left->name == right->name && left->key < right->key);
}

// profile_option row equal comparator by primary key: profile name, option key
bool ProfileOptionRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ProfileOptionRow * left = dynamic_cast<ProfileOptionRow *>(i_left.get());
    const ProfileOptionRow * right = dynamic_cast<ProfileOptionRow *>(i_right.get());
    return 
        left->name == right->name && left->key == right->key;
}

// profile_option row equal comparator by profile name
bool ProfileOptionRow::nameEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<ProfileOptionRow *>(i_left.get())->name == dynamic_cast<ProfileOptionRow *>(i_right.get())->name;
}

// run_option row less comparator by primary key: run id, option key
bool RunOptionRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const RunOptionRow * left = dynamic_cast<RunOptionRow *>(i_left.get());
    const RunOptionRow * right = dynamic_cast<RunOptionRow *>(i_right.get());
    return 
        (left->runId < right->runId) || (left->runId == right->runId && left->key < right->key);
}

// run_option row equal comparator by primary key: run id, option key
bool RunOptionRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const RunOptionRow * left = dynamic_cast<RunOptionRow *>(i_left.get());
    const RunOptionRow * right = dynamic_cast<RunOptionRow *>(i_right.get());
    return 
        left->runId == right->runId && left->key == right->key;
}

// run_option row equal comparator by run id
bool RunOptionRow::runIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<RunOptionRow *>(i_left.get())->runId == dynamic_cast<RunOptionRow *>(i_right.get())->runId;
}

// type_dic row less comparator by primary key: model id, type id
bool TypeDicRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeDicRow * left = dynamic_cast<TypeDicRow *>(i_left.get());
    const TypeDicRow * right = dynamic_cast<TypeDicRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || (left->modelId == right->modelId && left->typeId < right->typeId);
}

// type_dic row equal comparator by primary key: model id, type id
bool TypeDicRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeDicRow * left = dynamic_cast<TypeDicRow *>(i_left.get());
    const TypeDicRow * right = dynamic_cast<TypeDicRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->typeId == right->typeId;
}

// type_dic row equal comparator by model id
bool TypeDicRow::modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TypeDicRow *>(i_left.get())->modelId == dynamic_cast<TypeDicRow *>(i_right.get())->modelId;
}

// type_dic row equal comparator by model id and type name
bool TypeDicRow::modelIdNameEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeDicRow * left = dynamic_cast<TypeDicRow *>(i_left.get());
    const TypeDicRow * right = dynamic_cast<TypeDicRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->name == right->name;
}

// type_dic_txt row less comparator by primary key: model id, type id, language id
bool TypeDicTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeDicTxtRow * left = dynamic_cast<TypeDicTxtRow *>(i_left.get());
    const TypeDicTxtRow * right = dynamic_cast<TypeDicTxtRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->typeId < right->typeId) ||
        (left->modelId == right->modelId && left->typeId == right->typeId && left->langId < right->langId);
}

// type_dic_txt row equal comparator by primary key: model id, type id, language id
bool TypeDicTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeDicTxtRow * left = dynamic_cast<TypeDicTxtRow *>(i_left.get());
    const TypeDicTxtRow * right = dynamic_cast<TypeDicTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->typeId == right->typeId && left->langId == right->langId;
}

// type_dic_txt row equal comparator by language id
bool TypeDicTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TypeDicTxtRow *>(i_left.get())->langId == dynamic_cast<TypeDicTxtRow *>(i_right.get())->langId;
}

// type_enum_lst row less comparator by primary key: model id, type id, enum id
bool TypeEnumLstRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeEnumLstRow * left = dynamic_cast<TypeEnumLstRow *>(i_left.get());
    const TypeEnumLstRow * right = dynamic_cast<TypeEnumLstRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->typeId < right->typeId) ||
        (left->modelId == right->modelId && left->typeId == right->typeId && left->enumId < right->enumId);
}

// type_enum_lst row equal comparator by primary key: model id, type id, enum id
bool TypeEnumLstRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeEnumLstRow * left = dynamic_cast<TypeEnumLstRow *>(i_left.get());
    const TypeEnumLstRow * right = dynamic_cast<TypeEnumLstRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->typeId == right->typeId && left->enumId == right->enumId;
}

// type_enum_lst row equal comparator by model id
bool TypeEnumLstRow::modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TypeEnumLstRow *>(i_left.get())->modelId == dynamic_cast<TypeEnumLstRow *>(i_right.get())->modelId;
}

// type_enum_lst row equal comparator by model id and type id
bool TypeEnumLstRow::modelIdTypeIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeEnumLstRow * left = dynamic_cast<TypeEnumLstRow *>(i_left.get());
    const TypeEnumLstRow * right = dynamic_cast<TypeEnumLstRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->typeId == right->typeId;
}

// type_enum_txt row less comparator by primary key: model id, type id, enum id, language id
bool TypeEnumTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeEnumTxtRow * left = dynamic_cast<TypeEnumTxtRow *>(i_left.get());
    const TypeEnumTxtRow * right = dynamic_cast<TypeEnumTxtRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->typeId < right->typeId) ||
        (left->modelId == right->modelId && left->typeId == right->typeId && left->enumId < right->enumId) ||
        (left->modelId == right->modelId && left->typeId == right->typeId && left->enumId == right->enumId && left->langId < right->langId);
}

// type_enum_txt row equal comparator by primary key: model id, type id, enum id, language id
bool TypeEnumTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeEnumTxtRow * left = dynamic_cast<TypeEnumTxtRow *>(i_left.get());
    const TypeEnumTxtRow * right = dynamic_cast<TypeEnumTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->typeId == right->typeId && left->enumId == right->enumId && left->langId == right->langId;
}

// type_enum_txt row equal comparator by language id
bool TypeEnumTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TypeEnumTxtRow *>(i_left.get())->langId == dynamic_cast<TypeEnumTxtRow *>(i_right.get())->langId;
}

// type_enum_txt row equal comparator by model id and type id
bool TypeEnumTxtRow::modelIdTypeIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TypeEnumTxtRow * left = dynamic_cast<TypeEnumTxtRow *>(i_left.get());
    const TypeEnumTxtRow * right = dynamic_cast<TypeEnumTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->typeId == right->typeId;
}

// parameter_dic row less comparator by primary key: model id, parameter id
bool ParamDicRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamDicRow * left = dynamic_cast<ParamDicRow *>(i_left.get());
    const ParamDicRow * right = dynamic_cast<ParamDicRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || (left->modelId == right->modelId && left->paramId < right->paramId);
}

// parameter_dic row equal comparator by primary key: model id, parameter id
bool ParamDicRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamDicRow * left = dynamic_cast<ParamDicRow *>(i_left.get());
    const ParamDicRow * right = dynamic_cast<ParamDicRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->paramId == right->paramId;
}

// parameter_dic row equal comparator by model id
bool ParamDicRow::modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<ParamDicRow *>(i_left.get())->modelId == dynamic_cast<ParamDicRow *>(i_right.get())->modelId;
}

// parameter_dic row equal comparator by model id and parameter name
bool ParamDicRow::modelIdNameEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamDicRow * left = dynamic_cast<ParamDicRow *>(i_left.get());
    const ParamDicRow * right = dynamic_cast<ParamDicRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->paramName == right->paramName;
}

// parameter_dic_txt row less comparator by primary key: model id, parameter id, language id
bool ParamDicTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamDicTxtRow * left = dynamic_cast<ParamDicTxtRow *>(i_left.get());
    const ParamDicTxtRow * right = dynamic_cast<ParamDicTxtRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->paramId < right->paramId) ||
        (left->modelId == right->modelId && left->paramId == right->paramId && left->langId < right->langId);
}

// parameter_dic_txt row equal comparator by primary key: model id, parameter id, language id
bool ParamDicTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamDicTxtRow * left = dynamic_cast<ParamDicTxtRow *>(i_left.get());
    const ParamDicTxtRow * right = dynamic_cast<ParamDicTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->paramId == right->paramId && left->langId == right->langId;
}

// parameter_dic_txt row equal comparator by language id
bool ParamDicTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<ParamDicTxtRow *>(i_left.get())->langId == dynamic_cast<ParamDicTxtRow *>(i_right.get())->langId;
}

// parameter_run_txt row less comparator by primary key: run id, parameter id, language id
bool ParamRunTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamRunTxtRow * left = dynamic_cast<ParamRunTxtRow *>(i_left.get());
    const ParamRunTxtRow * right = dynamic_cast<ParamRunTxtRow *>(i_right.get());
    return 
        (left->runId < right->runId) || 
        (left->runId == right->runId && left->paramId < right->paramId) ||
        (left->runId == right->runId && left->paramId == right->paramId && left->langId < right->langId);
}

// parameter_run_txt row equal comparator by primary key: run id, parameter id, language id
bool ParamRunTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamRunTxtRow * left = dynamic_cast<ParamRunTxtRow *>(i_left.get());
    const ParamRunTxtRow * right = dynamic_cast<ParamRunTxtRow *>(i_right.get());
    return 
        left->runId == right->runId && left->paramId == right->paramId && left->langId == right->langId;
}

// parameter_run_txt row equal comparator by language id
bool ParamRunTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<ParamRunTxtRow *>(i_left.get())->langId == dynamic_cast<ParamRunTxtRow *>(i_right.get())->langId;
}

// parameter_dims row less comparator by primary key: model id, parameter id, dimension name
bool ParamDimsRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamDimsRow * left = dynamic_cast<ParamDimsRow *>(i_left.get());
    const ParamDimsRow * right = dynamic_cast<ParamDimsRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->paramId < right->paramId) ||
        (left->modelId == right->modelId && left->paramId == right->paramId && left->name < right->name);
}

// parameter_dims row equal comparator by primary key: model id, parameter id, dimension name
bool ParamDimsRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamDimsRow * left = dynamic_cast<ParamDimsRow *>(i_left.get());
    const ParamDimsRow * right = dynamic_cast<ParamDimsRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->paramId == right->paramId && left->name == right->name;
}

// parameter_dims row equal comparator by model id, parameter id
bool ParamDimsRow::modelIdParamIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const ParamDimsRow * left = dynamic_cast<ParamDimsRow *>(i_left.get());
    const ParamDimsRow * right = dynamic_cast<ParamDimsRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->paramId == right->paramId;
}

// workset_lst row less comparator by primary key: set id
bool WorksetLstRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<WorksetLstRow *>(i_left.get())->setId < dynamic_cast<WorksetLstRow *>(i_right.get())->setId;
}

// workset_lst row equal comparator by primary key: set id
bool WorksetLstRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<WorksetLstRow *>(i_left.get())->setId == dynamic_cast<WorksetLstRow *>(i_right.get())->setId;
}

// workset_txt row less comparator by primary key: set id and language id
bool WorksetTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const WorksetTxtRow * left = dynamic_cast<WorksetTxtRow *>(i_left.get());
    const WorksetTxtRow * right = dynamic_cast<WorksetTxtRow *>(i_right.get());
    return 
        (left->setId < right->setId) || (left->setId == right->setId && left->langId < right->langId);
}

// workset_txt row equal comparator by primary key: set id and language id
bool WorksetTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const WorksetTxtRow * left = dynamic_cast<WorksetTxtRow *>(i_left.get());
    const WorksetTxtRow * right = dynamic_cast<WorksetTxtRow *>(i_right.get());
    return 
        left->setId == right->setId && left->langId == right->langId;
}

// workset_txt row equal comparator by language id
bool WorksetTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<WorksetTxtRow *>(i_left.get())->langId == dynamic_cast<WorksetTxtRow *>(i_right.get())->langId;
}

// workset_parameter row less comparator by primary key: set id, parameter id
bool WorksetParamRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const WorksetParamRow * left = dynamic_cast<WorksetParamRow *>(i_left.get());
    const WorksetParamRow * right = dynamic_cast<WorksetParamRow *>(i_right.get());
    return 
        left->setId == right->setId && left->paramId < right->paramId;
}

// workset_parameter row equal comparator by primary key: set id, parameter id
bool WorksetParamRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const WorksetParamRow * left = dynamic_cast<WorksetParamRow *>(i_left.get());
    const WorksetParamRow * right = dynamic_cast<WorksetParamRow *>(i_right.get());
    return 
        left->setId == right->setId && left->paramId == right->paramId;
}

// workset_parameter_txt row less comparator by primary key: set id, parameter id, language id
bool WorksetParamTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const WorksetParamTxtRow * left = dynamic_cast<WorksetParamTxtRow *>(i_left.get());
    const WorksetParamTxtRow * right = dynamic_cast<WorksetParamTxtRow *>(i_right.get());
    return 
        (left->setId < right->setId) || 
        (left->setId == right->setId && left->paramId < right->paramId) ||
        (left->setId == right->setId && left->paramId == right->paramId && left->langId < right->langId);
}

// workset_parameter_txt row equal comparator by primary key: set id, parameter id, language id
bool WorksetParamTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const WorksetParamTxtRow * left = dynamic_cast<WorksetParamTxtRow *>(i_left.get());
    const WorksetParamTxtRow * right = dynamic_cast<WorksetParamTxtRow *>(i_right.get());
    return 
        left->setId == right->setId && left->paramId == right->paramId && left->langId == right->langId;
}

// workset_parameter_txt row equal comparator by language id
bool WorksetParamTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<WorksetParamTxtRow *>(i_left.get())->langId == dynamic_cast<WorksetParamTxtRow *>(i_right.get())->langId;
}

// table_dic row less comparator by primary key: model id, table id
bool TableDicRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDicRow * left = dynamic_cast<TableDicRow *>(i_left.get());
    const TableDicRow * right = dynamic_cast<TableDicRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || (left->modelId == right->modelId && left->tableId < right->tableId);
}

// table_dic row equal comparator by primary key: model id, table id
bool TableDicRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDicRow * left = dynamic_cast<TableDicRow *>(i_left.get());
    const TableDicRow * right = dynamic_cast<TableDicRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId;
}

// table_dic row equal comparator by model id
bool TableDicRow::modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TableDicRow *>(i_left.get())->modelId == dynamic_cast<TableDicRow *>(i_right.get())->modelId;
}

// table_dic row equal comparator by model id and table name
bool TableDicRow::modelIdNameEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDicRow * left = dynamic_cast<TableDicRow *>(i_left.get());
    const TableDicRow * right = dynamic_cast<TableDicRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableName == right->tableName;
}

// table_dic_txt row less comparator by primary key: model id, table id, language id
bool TableDicTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDicTxtRow * left = dynamic_cast<TableDicTxtRow *>(i_left.get());
    const TableDicTxtRow * right = dynamic_cast<TableDicTxtRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->tableId < right->tableId) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->langId < right->langId);
}

// table_dic_txt row equal comparator by primary key: model id, table id, language id
bool TableDicTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDicTxtRow * left = dynamic_cast<TableDicTxtRow *>(i_left.get());
    const TableDicTxtRow * right = dynamic_cast<TableDicTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId && left->langId == right->langId;
}

// table_dic_txt row equal comparator by language id
bool TableDicTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TableDicTxtRow *>(i_left.get())->langId == dynamic_cast<TableDicTxtRow *>(i_right.get())->langId;
}

// table_dims row less comparator by primary key: model id, table id, dimension name
bool TableDimsRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDimsRow * left = dynamic_cast<TableDimsRow *>(i_left.get());
    const TableDimsRow * right = dynamic_cast<TableDimsRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->tableId < right->tableId) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->name < right->name);
}

// table_dims row equal comparator by primary key: model id, dimension name
bool TableDimsRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDimsRow * left = dynamic_cast<TableDimsRow *>(i_left.get());
    const TableDimsRow * right = dynamic_cast<TableDimsRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId && left->name == right->name;
}

// table_dims row equal comparator by model id, table id
bool TableDimsRow::modelIdTableIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDimsRow * left = dynamic_cast<TableDimsRow *>(i_left.get());
    const TableDimsRow * right = dynamic_cast<TableDimsRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId;
}

// table_dims_txt row less comparator by primary key: model id, table id, dimension name, language id
bool TableDimsTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDimsTxtRow * left = dynamic_cast<TableDimsTxtRow *>(i_left.get());
    const TableDimsTxtRow * right = dynamic_cast<TableDimsTxtRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->tableId < right->tableId) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->name < right->name) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->name == right->name && left->langId < right->langId);
}

// table_dims_txt row equal comparator by primary key: model id, table id, dimension name, language id
bool TableDimsTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableDimsTxtRow * left = dynamic_cast<TableDimsTxtRow *>(i_left.get());
    const TableDimsTxtRow * right = dynamic_cast<TableDimsTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId && left->name == right->name && left->langId == right->langId;
}

// table_dims_txt row equal comparator by language id
bool TableDimsTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TableDimsTxtRow *>(i_left.get())->langId == dynamic_cast<TableDimsTxtRow *>(i_right.get())->langId;
}

// table_acc row less comparator by primary key: model id, table id, accumulator id
bool TableAccRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableAccRow * left = dynamic_cast<TableAccRow *>(i_left.get());
    const TableAccRow * right = dynamic_cast<TableAccRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->tableId < right->tableId) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->accId < right->accId);
}

// table_acc row equal comparator by primary key: model id,  table id, accumulator id
bool TableAccRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableAccRow * left = dynamic_cast<TableAccRow *>(i_left.get());
    const TableAccRow * right = dynamic_cast<TableAccRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId && left->accId == right->accId;
}

// table_acc row equal comparator by model id
bool TableAccRow::modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TableAccRow *>(i_left.get())->modelId == dynamic_cast<TableAccRow *>(i_right.get())->modelId;
}

// table_acc row equal comparator by model id, table id
bool TableAccRow::modelIdTableIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableAccRow * left = dynamic_cast<TableAccRow *>(i_left.get());
    const TableAccRow * right = dynamic_cast<TableAccRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId;
}

// table_acc_txt row less comparator by primary key: model id, table id, accumulator id, language id
bool TableAccTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableAccTxtRow * left = dynamic_cast<TableAccTxtRow *>(i_left.get());
    const TableAccTxtRow * right = dynamic_cast<TableAccTxtRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->tableId < right->tableId) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->accId < right->accId) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->accId == right->accId && left->langId < right->langId);
}

// table_acc_txt row equal comparator by primary key: model id, table id, accumulator id, language id
bool TableAccTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableAccTxtRow * left = dynamic_cast<TableAccTxtRow *>(i_left.get());
    const TableAccTxtRow * right = dynamic_cast<TableAccTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId && left->accId == right->accId && left->langId == right->langId;
}

// table_acc_txt row equal comparator by language id
bool TableAccTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TableAccTxtRow *>(i_left.get())->langId == dynamic_cast<TableAccTxtRow *>(i_right.get())->langId;
}

// table_unit row less comparator by primary key: model id, table id, unit id
bool TableUnitRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableUnitRow * left = dynamic_cast<TableUnitRow *>(i_left.get());
    const TableUnitRow * right = dynamic_cast<TableUnitRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->tableId < right->tableId) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->unitId < right->unitId);
}

// table_unit row equal comparator by primary key: model id,  table id, unit id
bool TableUnitRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableUnitRow * left = dynamic_cast<TableUnitRow *>(i_left.get());
    const TableUnitRow * right = dynamic_cast<TableUnitRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId && left->unitId == right->unitId;
}

// table_unit row equal comparator by model id
bool TableUnitRow::modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TableUnitRow *>(i_left.get())->modelId == dynamic_cast<TableUnitRow *>(i_right.get())->modelId;
}

// table_unit row equal comparator by model id, table id
bool TableUnitRow::modelIdTableIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableUnitRow * left = dynamic_cast<TableUnitRow *>(i_left.get());
    const TableUnitRow * right = dynamic_cast<TableUnitRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId;
}

// table_unit_txt row less comparator by primary key: model id, table id, unit id, language id
bool TableUnitTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableUnitTxtRow * left = dynamic_cast<TableUnitTxtRow *>(i_left.get());
    const TableUnitTxtRow * right = dynamic_cast<TableUnitTxtRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->tableId < right->tableId) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->unitId < right->unitId) ||
        (left->modelId == right->modelId && left->tableId == right->tableId && left->unitId == right->unitId && left->langId < right->langId);
}

// table_unit_txt row equal comparator by primary key: model id, table id, unit id, language id
bool TableUnitTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const TableUnitTxtRow * left = dynamic_cast<TableUnitTxtRow *>(i_left.get());
    const TableUnitTxtRow * right = dynamic_cast<TableUnitTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->tableId == right->tableId && left->unitId == right->unitId && left->langId == right->langId;
}

// table_unit_txt row equal comparator by language id
bool TableUnitTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<TableUnitTxtRow *>(i_left.get())->langId == dynamic_cast<TableUnitTxtRow *>(i_right.get())->langId;
}

// group_lst row less comparator by primary key: model id, group id
bool GroupLstRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const GroupLstRow * left = dynamic_cast<GroupLstRow *>(i_left.get());
    const GroupLstRow * right = dynamic_cast<GroupLstRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || (left->modelId == right->modelId && left->groupId < right->groupId);
}

// group_lst row equal comparator by primary key: model id, group id
bool GroupLstRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const GroupLstRow * left = dynamic_cast<GroupLstRow *>(i_left.get());
    const GroupLstRow * right = dynamic_cast<GroupLstRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->groupId == right->groupId;
}

// group_lst row equal comparator by model id and is parameter group flag
bool GroupLstRow::modelIdIsParamEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const GroupLstRow * left = dynamic_cast<GroupLstRow *>(i_left.get());
    const GroupLstRow * right = dynamic_cast<GroupLstRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->isParam == right->isParam;
}

// group_txt row less comparator by primary key: model id, group id, language id
bool GroupTxtRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const GroupTxtRow * left = dynamic_cast<GroupTxtRow *>(i_left.get());
    const GroupTxtRow * right = dynamic_cast<GroupTxtRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->groupId < right->groupId) ||
        (left->modelId == right->modelId && left->groupId == right->groupId && left->langId < right->langId);
}

// group_txt row equal comparator by primary key: model id, group id, language id
bool GroupTxtRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const GroupTxtRow * left = dynamic_cast<GroupTxtRow *>(i_left.get());
    const GroupTxtRow * right = dynamic_cast<GroupTxtRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->groupId == right->groupId && left->langId == right->langId;
}

// group_txt row equal comparator by language id
bool GroupTxtRow::langEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<GroupTxtRow *>(i_left.get())->langId == dynamic_cast<GroupTxtRow *>(i_right.get())->langId;
}

// group_pc row less comparator by primary key: model id, group id, child position
bool GroupPcRow::keyLess(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const GroupPcRow * left = dynamic_cast<GroupPcRow *>(i_left.get());
    const GroupPcRow * right = dynamic_cast<GroupPcRow *>(i_right.get());
    return 
        (left->modelId < right->modelId) || 
        (left->modelId == right->modelId && left->groupId < right->groupId) ||
        (left->modelId == right->modelId && left->groupId == right->groupId && left->childPos < right->childPos);
}

// group_pc row equal comparator by primary key: model id, group id, child position
bool GroupPcRow::keyEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    const GroupPcRow * left = dynamic_cast<GroupPcRow *>(i_left.get());
    const GroupPcRow * right = dynamic_cast<GroupPcRow *>(i_right.get());
    return 
        left->modelId == right->modelId && left->groupId == right->groupId && left->childPos == right->childPos;
}

// group_pc row equal comparator by model id
bool GroupPcRow::modelIdEqual(const IRowBaseUptr & i_left, const IRowBaseUptr & i_right)
{
    return dynamic_cast<GroupPcRow *>(i_left.get())->modelId == dynamic_cast<GroupPcRow *>(i_right.get())->modelId;
}

