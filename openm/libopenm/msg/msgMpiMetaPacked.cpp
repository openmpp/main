// OpenM++ message passing library: pack and unpack adapters for metadata db rows
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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
        MpiPacked::pack(val->digest, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->type)>(val->type, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->version, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->createDateTime, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->paramPrefix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->setPrefix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->accPrefix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->valuePrefix, i_packedSize, io_packedData, io_packPos);
    }

    // model_dic: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<ModelDicRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        ModelDicRow * val = static_cast<ModelDicRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->digest = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->type = MpiPacked::unpack<decltype(val->type)>(i_packedSize, i_packedData, io_packPos);
        val->version = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->createDateTime = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->paramPrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->setPrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->accPrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
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
            MpiPacked::packedSize(val->digest) +
            MpiPacked::packedSize(typeid(int)) +
            MpiPacked::packedSize(val->version) +
            MpiPacked::packedSize(val->createDateTime) +
            MpiPacked::packedSize(val->paramPrefix) +
            MpiPacked::packedSize(val->setPrefix) +
            MpiPacked::packedSize(val->accPrefix) +
            MpiPacked::packedSize(val->valuePrefix);
    }

    // type_dic: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TypeDicRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TypeDicRow * val = dynamic_cast<const TypeDicRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->digest, i_packedSize, io_packedData, io_packPos);
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
        val->digest = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
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
            MpiPacked::packedSize(val->digest) +
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
        MpiPacked::pack(val->paramName, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->paramHid)>(val->paramHid, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->digest, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbPrefix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbSuffix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->rank)>(val->rank, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isHidden)>(val->isHidden, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->numCumulated)>(val->numCumulated, i_packedSize, io_packedData, io_packPos);
    }

    // parameter_dic: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<ParamDicRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        ParamDicRow * val = static_cast<ParamDicRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->paramId = MpiPacked::unpack<decltype(val->paramId)>(i_packedSize, i_packedData, io_packPos);
        val->paramName = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->paramHid = MpiPacked::unpack<decltype(val->paramHid)>(i_packedSize, i_packedData, io_packPos);
        val->digest = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->dbPrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->dbSuffix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->rank = MpiPacked::unpack<decltype(val->rank)>(i_packedSize, i_packedData, io_packPos);
        val->typeId = MpiPacked::unpack<decltype(val->typeId)>(i_packedSize, i_packedData, io_packPos);
        val->isHidden = MpiPacked::unpack<decltype(val->isHidden)>(i_packedSize, i_packedData, io_packPos);
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
            MpiPacked::packedSize(val->paramName) +
            MpiPacked::packedSize(typeid(val->paramHid)) +
            MpiPacked::packedSize(val->digest) +
            MpiPacked::packedSize(val->dbPrefix) +
            MpiPacked::packedSize(val->dbSuffix) +
            MpiPacked::packedSize(typeid(val->rank)) +
            MpiPacked::packedSize(typeid(val->typeId)) +
            MpiPacked::packedSize(typeid(val->isHidden)) +
            MpiPacked::packedSize(typeid(val->numCumulated));
    }

    // parameter_dims: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<ParamDimsRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const ParamDimsRow * val = dynamic_cast<const ParamDimsRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->paramId)>(val->paramId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->dimId)>(val->dimId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
    }

    // parameter_dims: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<ParamDimsRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        ParamDimsRow * val = static_cast<ParamDimsRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->paramId = MpiPacked::unpack<decltype(val->paramId)>(i_packedSize, i_packedData, io_packPos);
        val->dimId = MpiPacked::unpack<decltype(val->dimId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
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
            MpiPacked::packedSize(typeid(val->dimId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(val->typeId));
    }

    // table_dic: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TableDicRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TableDicRow * val = dynamic_cast<const TableDicRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->tableId)>(val->tableId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->tableName, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->tableHid)>(val->tableHid, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->digest, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbPrefix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbSuffix, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isUser)>(val->isUser, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->rank)>(val->rank, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isSparse)>(val->isSparse, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->exprPos)>(val->exprPos, i_packedSize, io_packedData, io_packPos);
    }

    // table_dic: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<TableDicRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        TableDicRow * val = static_cast<TableDicRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->tableId = MpiPacked::unpack<decltype(val->tableId)>(i_packedSize, i_packedData, io_packPos);
        val->tableName = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->tableHid = MpiPacked::unpack<decltype(val->tableHid)>(i_packedSize, i_packedData, io_packPos);
        val->digest = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->dbPrefix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->dbSuffix = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->isUser = MpiPacked::unpack<decltype(val->isUser)>(i_packedSize, i_packedData, io_packPos);
        val->rank = MpiPacked::unpack<decltype(val->rank)>(i_packedSize, i_packedData, io_packPos);
        val->isSparse = MpiPacked::unpack<decltype(val->isSparse)>(i_packedSize, i_packedData, io_packPos);
        val->exprPos = MpiPacked::unpack<decltype(val->exprPos)>(i_packedSize, i_packedData, io_packPos);
    }

    // table_dic: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<TableDicRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const TableDicRow * val = dynamic_cast<const TableDicRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->tableId)) +
            MpiPacked::packedSize(val->tableName) +
            MpiPacked::packedSize(typeid(val->tableHid)) +
            MpiPacked::packedSize(val->digest) +
            MpiPacked::packedSize(val->dbPrefix) +
            MpiPacked::packedSize(val->dbSuffix) +
            MpiPacked::packedSize(typeid(val->isUser)) +
            MpiPacked::packedSize(typeid(val->rank)) +
            MpiPacked::packedSize(typeid(val->isSparse)) +
            MpiPacked::packedSize(typeid(val->exprPos));
    }

    // table_dims: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TableDimsRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TableDimsRow * val = dynamic_cast<const TableDimsRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->tableId)>(val->tableId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->dimId)>(val->dimId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isTotal)>(val->isTotal, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->dimSize)>(val->dimSize, i_packedSize, io_packedData, io_packPos);
    }

    // table_dims: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<TableDimsRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        TableDimsRow * val = static_cast<TableDimsRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->tableId = MpiPacked::unpack<decltype(val->tableId)>(i_packedSize, i_packedData, io_packPos);
        val->dimId = MpiPacked::unpack<decltype(val->dimId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->typeId = MpiPacked::unpack<decltype(val->typeId)>(i_packedSize, i_packedData, io_packPos);
        val->isTotal = MpiPacked::unpack<decltype(val->isTotal)>(i_packedSize, i_packedData, io_packPos);
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
            MpiPacked::packedSize(typeid(val->dimId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(val->typeId)) +
            MpiPacked::packedSize(typeid(val->isTotal)) +
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

    // table_expr: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<TableExprRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const TableExprRow * val = dynamic_cast<const TableExprRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->tableId)>(val->tableId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->exprId)>(val->exprId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->decimals)>(val->decimals, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->srcExpr, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->sqlExpr, i_packedSize, io_packedData, io_packPos);
    }

    // table_expr: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<TableExprRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        TableExprRow * val = static_cast<TableExprRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->tableId = MpiPacked::unpack<decltype(val->tableId)>(i_packedSize, i_packedData, io_packPos);
        val->exprId = MpiPacked::unpack<decltype(val->exprId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->decimals = MpiPacked::unpack<decltype(val->decimals)>(i_packedSize, i_packedData, io_packPos);
        val->srcExpr = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->sqlExpr = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
    }

    // table_expr: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<TableExprRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const TableExprRow * val = dynamic_cast<const TableExprRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->tableId)) +
            MpiPacked::packedSize(typeid(val->exprId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(val->decimals)) +
            MpiPacked::packedSize(val->srcExpr) +
            MpiPacked::packedSize(val->sqlExpr);
    }

    // group_lst: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<GroupLstRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const GroupLstRow * val = dynamic_cast<const GroupLstRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->groupId)>(val->groupId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isParam)>(val->isParam, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isHidden)>(val->isHidden, i_packedSize, io_packedData, io_packPos);
    }

    // group_lst: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<GroupLstRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        GroupLstRow * val = static_cast<GroupLstRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->groupId = MpiPacked::unpack<decltype(val->groupId)>(i_packedSize, i_packedData, io_packPos);
        val->isParam = MpiPacked::unpack<decltype(val->isParam)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->isHidden = MpiPacked::unpack<decltype(val->isHidden)>(i_packedSize, i_packedData, io_packPos);
    }

    // group_lst: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<GroupLstRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const GroupLstRow * val = dynamic_cast<const GroupLstRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->groupId)) +
            MpiPacked::packedSize(typeid(val->isParam)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(val->isHidden));
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
}

// create new pack and unpack adapter for metadata table db rows
IPackedAdapter * IPackedAdapter::create(MsgTag i_msgTag)
{
    lock_guard<recursive_mutex> lck(msgMutex);

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
    case MsgTag::tableExpr:
        return new MetaMpiPackedAdapter<MsgTag::tableExpr, TableExprRow>();
    case MsgTag::groupLst:
        return new MetaMpiPackedAdapter<MsgTag::groupLst, GroupLstRow>();
    case MsgTag::groupPc:
        return new MetaMpiPackedAdapter<MsgTag::groupPc, GroupPcRow>();
    default:
        throw MsgException("Fail to create message adapter: invalid message tag");
    }
}
#endif  // OM_MSG_MPI
