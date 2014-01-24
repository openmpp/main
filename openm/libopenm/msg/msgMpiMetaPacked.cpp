// OpenM++ message passing library: pack and unpack adapters for metadata db rows
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifdef OM_MSG_MPI

using namespace std;

#include "libopenm/db/dbMetaRow.h"
#include "msgCommon.h"
using namespace openm;

namespace openm
{
    // model_dic: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<ModelDicRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const ModelDicRow * val = dynamic_cast<const ModelDicRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<int>((int)val->type, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->version, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->timestamp, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->modelPrefix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->paramPrefix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->setPrefix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->subPrefix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->valuePrefix, i_packedSize, io_packedData, io_packPos);
    }
    
    // model_dic: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<ModelDicRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        ModelDicRow * val = static_cast<ModelDicRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->type = (ModelType)MpiPacked::unpack<int>(i_packedSize, i_packedData, io_packPos);
        val->version = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->timestamp = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->modelPrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->paramPrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->setPrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->subPrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->valuePrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
    }

    // model_dic: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<ModelDicRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const ModelDicRow * val = dynamic_cast<const ModelDicRow *>(i_row.get());
        return 
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(int)) +
            MpiPacked::packedSize(val->version) +
            MpiPacked::packedSize(val->timestamp) +
            MpiPacked::packedSize(val->modelPrefix) +
            MpiPacked::packedSize(val->paramPrefix) +
            MpiPacked::packedSize(val->setPrefix) +
            MpiPacked::packedSize(val->subPrefix) +
            MpiPacked::packedSize(val->valuePrefix);
    }

    // run_option: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<RunOptionRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const RunOptionRow * val = dynamic_cast<const RunOptionRow *>(i_row.get());

        MpiPacked::pack<decltype(val->runId)>(val->runId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->key, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->value, i_packedSize, io_packedData, io_packPos);
    }

    // run_option: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<RunOptionRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        RunOptionRow * val = static_cast<RunOptionRow *>(io_row.get());

        val->runId = MpiPacked::unpack<decltype(val->runId)>(i_packedSize, i_packedData, io_packPos);
        val->key = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->value = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
    }

    // run_option: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<RunOptionRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const RunOptionRow * val = dynamic_cast<const RunOptionRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->runId)) +
            MpiPacked::packedSize(val->key) +
            MpiPacked::packedSize(val->value);
    }

    // type_dic: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TypeDicRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TypeDicRow * val = dynamic_cast<const TypeDicRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->dicId)>(val->dicId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->totalEnumId)>(val->totalEnumId, i_packedSize, io_packedData, io_packPos);
    }

    // type_dic: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<TypeDicRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        TypeDicRow * val = static_cast<TypeDicRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->typeId = MpiPacked::unpack<decltype(val->typeId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->dicId = MpiPacked::unpack<decltype(val->dicId)>(i_packedSize, i_packedData, io_packPos);
        val->totalEnumId = MpiPacked::unpack<decltype(val->totalEnumId)>(i_packedSize, i_packedData, io_packPos);
    }

    // type_dic: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<TypeDicRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const TypeDicRow * val = dynamic_cast<const TypeDicRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->typeId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(val->dicId)) +
            MpiPacked::packedSize(typeid(val->totalEnumId));
    }

    // type_enum_lst: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TypeEnumLstRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TypeEnumLstRow * val = dynamic_cast<const TypeEnumLstRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->enumId)>(val->enumId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
    }

    // type_enum_lst: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<TypeEnumLstRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        TypeEnumLstRow * val = static_cast<TypeEnumLstRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->typeId = MpiPacked::unpack<decltype(val->typeId)>(i_packedSize, i_packedData, io_packPos);
        val->enumId = MpiPacked::unpack<decltype(val->enumId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
    }

    // type_enum_lst: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<TypeEnumLstRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const TypeEnumLstRow * val = dynamic_cast<const TypeEnumLstRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->typeId)) +
            MpiPacked::packedSize(typeid(val->enumId)) +
            MpiPacked::packedSize(val->name);
    }

    // parameter_dic: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<ParamDicRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const ParamDicRow * val = dynamic_cast<const ParamDicRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->paramId)>(val->paramId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbNameSuffix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->paramName, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->rank)>(val->rank, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->isHidden, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->isGenerated, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->numCumulated)>(val->numCumulated, i_packedSize, io_packedData, io_packPos);
    }

    // parameter_dic: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<ParamDicRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        ParamDicRow * val = static_cast<ParamDicRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->paramId = MpiPacked::unpack<decltype(val->paramId)>(i_packedSize, i_packedData, io_packPos);
        val->dbNameSuffix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->paramName = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->rank = MpiPacked::unpack<decltype(val->rank)>(i_packedSize, i_packedData, io_packPos);
        val->typeId = MpiPacked::unpack<decltype(val->typeId)>(i_packedSize, i_packedData, io_packPos);
        val->isHidden = MpiPacked::unpackBool(i_packedSize, i_packedData, io_packPos);
        val->isGenerated = MpiPacked::unpackBool(i_packedSize, i_packedData, io_packPos);
        val->numCumulated = MpiPacked::unpack<decltype(val->numCumulated)>(i_packedSize, i_packedData, io_packPos);
    }

    // parameter_dic: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<ParamDicRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const ParamDicRow * val = dynamic_cast<const ParamDicRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->paramId)) +
            MpiPacked::packedSize(val->dbNameSuffix) +
            MpiPacked::packedSize(val->paramName) +
            MpiPacked::packedSize(typeid(val->rank)) +
            MpiPacked::packedSize(typeid(val->typeId)) +
            MpiPacked::packedSize(val->isHidden) +
            MpiPacked::packedSize(val->isGenerated) +
            MpiPacked::packedSize(typeid(val->numCumulated));
    }

    // parameter_dims: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<ParamDimsRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const ParamDimsRow * val = dynamic_cast<const ParamDimsRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->paramId)>(val->paramId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->pos)>(val->pos, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
    }

    // parameter_dims: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<ParamDimsRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        ParamDimsRow * val = static_cast<ParamDimsRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->paramId = MpiPacked::unpack<decltype(val->paramId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->pos = MpiPacked::unpack<decltype(val->pos)>(i_packedSize, i_packedData, io_packPos);
        val->typeId = MpiPacked::unpack<decltype(val->typeId)>(i_packedSize, i_packedData, io_packPos);
    }

    // parameter_dims: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<ParamDimsRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const ParamDimsRow * val = dynamic_cast<const ParamDimsRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->paramId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(val->pos)) +
            MpiPacked::packedSize(typeid(val->typeId));
    }

    // table_dic: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TableDicRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TableDicRow * val = dynamic_cast<const TableDicRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->tableId)>(val->tableId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbNameSuffix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->tableName, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->isUser, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->rank)>(val->rank, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->isSparse, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->isHidden, i_packedSize, io_packedData, io_packPos);
    }

    // table_dic: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<TableDicRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        TableDicRow * val = static_cast<TableDicRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->tableId = MpiPacked::unpack<decltype(val->tableId)>(i_packedSize, i_packedData, io_packPos);
        val->dbNameSuffix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->tableName = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->isUser = MpiPacked::unpackBool(i_packedSize, i_packedData, io_packPos);
        val->rank = MpiPacked::unpack<decltype(val->rank)>(i_packedSize, i_packedData, io_packPos);
        val->isSparse = MpiPacked::unpackBool(i_packedSize, i_packedData, io_packPos);
        val->isHidden = MpiPacked::unpackBool(i_packedSize, i_packedData, io_packPos);
    }

    // table_dic: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<TableDicRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const TableDicRow * val = dynamic_cast<const TableDicRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->tableId)) +
            MpiPacked::packedSize(val->dbNameSuffix) +
            MpiPacked::packedSize(val->tableName) +
            MpiPacked::packedSize(val->isUser) +
            MpiPacked::packedSize(typeid(val->rank)) +
            MpiPacked::packedSize(val->isSparse) +
            MpiPacked::packedSize(val->isHidden);
    }

    // table_dims: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TableDimsRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TableDimsRow * val = dynamic_cast<const TableDimsRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->tableId)>(val->tableId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->pos)>(val->pos, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->isTotal, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->dimSize)>(val->dimSize, i_packedSize, io_packedData, io_packPos);
    }

    // table_dims: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<TableDimsRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        TableDimsRow * val = static_cast<TableDimsRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->tableId = MpiPacked::unpack<decltype(val->tableId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->pos = MpiPacked::unpack<decltype(val->pos)>(i_packedSize, i_packedData, io_packPos);
        val->typeId = MpiPacked::unpack<decltype(val->typeId)>(i_packedSize, i_packedData, io_packPos);
        val->isTotal = MpiPacked::unpackBool(i_packedSize, i_packedData, io_packPos);
        val->dimSize = MpiPacked::unpack<decltype(val->dimSize)>(i_packedSize, i_packedData, io_packPos);
    }

    // table_dims: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<TableDimsRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const TableDimsRow * val = dynamic_cast<const TableDimsRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->tableId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(val->pos)) +
            MpiPacked::packedSize(typeid(val->typeId)) +
            MpiPacked::packedSize(val->isTotal) +
            MpiPacked::packedSize(typeid(val->dimSize));
    }

    // table_acc: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TableAccRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TableAccRow * val = dynamic_cast<const TableAccRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->tableId)>(val->tableId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->accId)>(val->accId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->expr, i_packedSize, io_packedData, io_packPos);
    }

    // table_acc: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<TableAccRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        TableAccRow * val = static_cast<TableAccRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->tableId = MpiPacked::unpack<decltype(val->tableId)>(i_packedSize, i_packedData, io_packPos);
        val->accId = MpiPacked::unpack<decltype(val->accId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->expr = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
    }

    // table_acc: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<TableAccRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const TableAccRow * val = dynamic_cast<const TableAccRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->tableId)) +
            MpiPacked::packedSize(typeid(val->accId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(val->expr);
    }

    // table_unit: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TableUnitRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TableUnitRow * val = dynamic_cast<const TableUnitRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->tableId)>(val->tableId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->unitId)>(val->unitId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->decimals)>(val->decimals, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->src, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->expr, i_packedSize, io_packedData, io_packPos);
    }

    // table_unit: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<TableUnitRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        TableUnitRow * val = static_cast<TableUnitRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->tableId = MpiPacked::unpack<decltype(val->tableId)>(i_packedSize, i_packedData, io_packPos);
        val->unitId = MpiPacked::unpack<decltype(val->unitId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->decimals = MpiPacked::unpack<decltype(val->decimals)>(i_packedSize, i_packedData, io_packPos);
        val->src = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->expr = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
    }

    // table_unit: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<TableUnitRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const TableUnitRow * val = dynamic_cast<const TableUnitRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->tableId)) +
            MpiPacked::packedSize(typeid(val->unitId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(val->decimals)) +
            MpiPacked::packedSize(val->src) +
            MpiPacked::packedSize(val->expr);
    }

    // group_lst: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<GroupLstRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const GroupLstRow * val = dynamic_cast<const GroupLstRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->groupId)>(val->groupId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->isParam, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->isHidden, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->isGenerated, i_packedSize, io_packedData, io_packPos);
    }

    // group_lst: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<GroupLstRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        GroupLstRow * val = static_cast<GroupLstRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->groupId = MpiPacked::unpack<decltype(val->groupId)>(i_packedSize, i_packedData, io_packPos);
        val->isParam = MpiPacked::unpackBool(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->isHidden = MpiPacked::unpackBool(i_packedSize, i_packedData, io_packPos);
        val->isGenerated = MpiPacked::unpackBool(i_packedSize, i_packedData, io_packPos);
    }

    // group_lst: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<GroupLstRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const GroupLstRow * val = dynamic_cast<const GroupLstRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->groupId)) +
            MpiPacked::packedSize(val->isParam) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(val->isHidden) +
            MpiPacked::packedSize(val->isGenerated);
    }

    // group_pc: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<GroupPcRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const GroupPcRow * val = dynamic_cast<const GroupPcRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->groupId)>(val->groupId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->childPos)>(val->childPos, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->childGroupId)>(val->childGroupId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->leafId)>(val->leafId, i_packedSize, io_packedData, io_packPos);
    }

    // group_pc: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<GroupPcRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        GroupPcRow * val = static_cast<GroupPcRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->groupId = MpiPacked::unpack<decltype(val->groupId)>(i_packedSize, i_packedData, io_packPos);
        val->childPos = MpiPacked::unpack<decltype(val->childPos)>(i_packedSize, i_packedData, io_packPos);
        val->childGroupId = MpiPacked::unpack<decltype(val->childGroupId)>(i_packedSize, i_packedData, io_packPos);
        val->leafId = MpiPacked::unpack<decltype(val->leafId)>(i_packedSize, i_packedData, io_packPos);
    }

    // group_pc: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<GroupPcRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const GroupPcRow * val = dynamic_cast<const GroupPcRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->groupId)) +
            MpiPacked::packedSize(typeid(val->childPos)) +
            MpiPacked::packedSize(typeid(val->childGroupId)) +
            MpiPacked::packedSize(typeid(val->leafId));
    }
}

// create new pack and unpack adapter for metadata table db rows
IPackedAdapter * IPackedAdapter::create(MsgTag i_msgTag)
{
    switch (i_msgTag)
    {
    case MsgTag::modelDic:
        return new MetaMpiPackedAdapter<MsgTag::modelDic, ModelDicRow>();
    case MsgTag::runOption:
        return new MetaMpiPackedAdapter<MsgTag::runOption, RunOptionRow>();
    case MsgTag::typeDic:
        return new MetaMpiPackedAdapter<MsgTag::typeDic, TypeDicRow>();
    case MsgTag::typeEnumLst:
        return new MetaMpiPackedAdapter<MsgTag::typeEnumLst, TypeEnumLstRow>();
    case MsgTag::parameterDic:
        return new MetaMpiPackedAdapter<MsgTag::parameterDic, ParamDicRow>();
    case MsgTag::parameterDims:
        return new MetaMpiPackedAdapter<MsgTag::parameterDims, ParamDimsRow>();
    case MsgTag::tableDic:
        return new MetaMpiPackedAdapter<MsgTag::tableDic, TableDicRow>();
    case MsgTag::tableDims:
        return new MetaMpiPackedAdapter<MsgTag::tableDims, TableDimsRow>();
    case MsgTag::tableAcc:
        return new MetaMpiPackedAdapter<MsgTag::tableAcc, TableAccRow>();
    case MsgTag::tableUnit:
        return new MetaMpiPackedAdapter<MsgTag::tableUnit, TableUnitRow>();
    case MsgTag::groupLst:
        return new MetaMpiPackedAdapter<MsgTag::groupLst, GroupLstRow>();
    case MsgTag::groupPc:
        return new MetaMpiPackedAdapter<MsgTag::groupPc, GroupPcRow>();
    default:
        throw MsgException("Fail to create message adapter: invalid message tag");
    }
}
#endif  // OM_MSG_MPI
