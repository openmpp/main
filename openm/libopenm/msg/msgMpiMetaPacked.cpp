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
        MpiPacked::pack<decltype(val->defaultLangId)>(val->defaultLangId, i_packedSize, io_packedData, io_packPos);
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
        val->defaultLangId = MpiPacked::unpack<decltype(val->defaultLangId)>(i_packedSize, i_packedData, io_packPos);
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
            MpiPacked::packedSize(typeid(val->defaultLangId));
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
        MpiPacked::pack<decltype(val->rank)>(val->rank, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isExtendable)>(val->isExtendable, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isHidden)>(val->isHidden, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->numCumulated)>(val->numCumulated, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbRunTable, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbSetTable, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->importDigest, i_packedSize, io_packedData, io_packPos);
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
        val->rank = MpiPacked::unpack<decltype(val->rank)>(i_packedSize, i_packedData, io_packPos);
        val->typeId = MpiPacked::unpack<decltype(val->typeId)>(i_packedSize, i_packedData, io_packPos);
        val->isExtendable = MpiPacked::unpack<decltype(val->isExtendable)>(i_packedSize, i_packedData, io_packPos);
        val->isHidden = MpiPacked::unpack<decltype(val->isHidden)>(i_packedSize, i_packedData, io_packPos);
        val->numCumulated = MpiPacked::unpack<decltype(val->numCumulated)>(i_packedSize, i_packedData, io_packPos);
        val->dbRunTable = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->dbSetTable = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->importDigest = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
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
            MpiPacked::packedSize(typeid(val->rank)) +
            MpiPacked::packedSize(typeid(val->typeId)) +
            MpiPacked::packedSize(typeid(val->isExtendable)) +
            MpiPacked::packedSize(typeid(val->isHidden)) +
            MpiPacked::packedSize(typeid(val->numCumulated)) +
            MpiPacked::packedSize(val->dbRunTable) +
            MpiPacked::packedSize(val->dbSetTable) +
            MpiPacked::packedSize(val->importDigest);
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
        MpiPacked::pack<decltype(val->isUser)>(val->isUser, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->rank)>(val->rank, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isSparse)>(val->isSparse, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbExprTable, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbAccTable, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->dbAccAll, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->exprPos)>(val->exprPos, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isHidden)>(val->isHidden, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->importDigest, i_packedSize, io_packedData, io_packPos);
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
        val->isUser = MpiPacked::unpack<decltype(val->isUser)>(i_packedSize, i_packedData, io_packPos);
        val->rank = MpiPacked::unpack<decltype(val->rank)>(i_packedSize, i_packedData, io_packPos);
        val->isSparse = MpiPacked::unpack<decltype(val->isSparse)>(i_packedSize, i_packedData, io_packPos);
        val->dbExprTable = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->dbAccTable = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->dbAccAll = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->exprPos = MpiPacked::unpack<decltype(val->exprPos)>(i_packedSize, i_packedData, io_packPos);
        val->isHidden = MpiPacked::unpack<decltype(val->isHidden)>(i_packedSize, i_packedData, io_packPos);
        val->importDigest = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
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
            MpiPacked::packedSize(typeid(val->isUser)) +
            MpiPacked::packedSize(typeid(val->rank)) +
            MpiPacked::packedSize(typeid(val->isSparse)) +
            MpiPacked::packedSize(val->dbExprTable) +
            MpiPacked::packedSize(val->dbAccTable) +
            MpiPacked::packedSize(val->dbAccAll) +
            MpiPacked::packedSize(typeid(val->exprPos)) +
            MpiPacked::packedSize(typeid(val->isHidden)) +
            MpiPacked::packedSize(val->importDigest);
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
        MpiPacked::pack<decltype(val->isDerived)>(val->isDerived, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->accSrc, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->accSql, i_packedSize, io_packedData, io_packPos);
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
        val->isDerived = MpiPacked::unpack<decltype(val->isDerived)>(i_packedSize, i_packedData, io_packPos);
        val->accSrc = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->accSql = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
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
            MpiPacked::packedSize(typeid(val->isDerived)) +
            MpiPacked::packedSize(val->accSrc) +
            MpiPacked::packedSize(val->accSql);
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

    // entity_dic: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<EntityDicRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const EntityDicRow * val = dynamic_cast<const EntityDicRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->entityId)>(val->entityId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->entityName, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->entityHid)>(val->entityHid, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->digest, i_packedSize, io_packedData, io_packPos);
    }

    // entity_dic: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<EntityDicRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        EntityDicRow * val = static_cast<EntityDicRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->entityId = MpiPacked::unpack<decltype(val->entityId)>(i_packedSize, i_packedData, io_packPos);
        val->entityName = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->entityHid = MpiPacked::unpack<decltype(val->entityHid)>(i_packedSize, i_packedData, io_packPos);
        val->digest = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
    }

    // entity_dic: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<EntityDicRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const EntityDicRow * val = dynamic_cast<const EntityDicRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->entityId)) +
            MpiPacked::packedSize(val->entityName) +
            MpiPacked::packedSize(typeid(val->entityHid)) +
            MpiPacked::packedSize(val->digest);
    }

    // entity_attr: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<EntityAttrRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const EntityAttrRow * val = dynamic_cast<const EntityAttrRow *>(i_row.get());

        MpiPacked::pack<decltype(val->modelId)>(val->modelId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->entityId)>(val->entityId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->attrId)>(val->attrId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->name, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->typeId)>(val->typeId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->isInternal)>(val->isInternal, i_packedSize, io_packedData, io_packPos);
    }

    // entity_attr: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<EntityAttrRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        EntityAttrRow * val = static_cast<EntityAttrRow *>(io_row.get());

        val->modelId = MpiPacked::unpack<decltype(val->modelId)>(i_packedSize, i_packedData, io_packPos);
        val->entityId = MpiPacked::unpack<decltype(val->entityId)>(i_packedSize, i_packedData, io_packPos);
        val->attrId = MpiPacked::unpack<decltype(val->attrId)>(i_packedSize, i_packedData, io_packPos);
        val->name = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->typeId = MpiPacked::unpack<decltype(val->typeId)>(i_packedSize, i_packedData, io_packPos);
        val->isInternal = MpiPacked::unpack<decltype(val->isInternal)>(i_packedSize, i_packedData, io_packPos);
    }

    // entity_attr: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<EntityAttrRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const EntityAttrRow * val = dynamic_cast<const EntityAttrRow *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->modelId)) +
            MpiPacked::packedSize(typeid(val->entityId)) +
            MpiPacked::packedSize(typeid(val->attrId)) +
            MpiPacked::packedSize(val->name) +
            MpiPacked::packedSize(typeid(val->typeId)) +
            MpiPacked::packedSize(typeid(val->isInternal));
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

    // generic (code,value) row: pack db row into MPI message
    template<>
    void RowMpiPackedAdapter<CodeValueRow>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const CodeValueRow * val = dynamic_cast<const CodeValueRow *>(i_row.get());

        MpiPacked::pack(val->code, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack(val->value, i_packedSize, io_packedData, io_packPos);
    }

    // generic (code,value) row: unpack MPI message into db row
    template<>
    void RowMpiPackedAdapter<CodeValueRow>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        CodeValueRow * val = static_cast<CodeValueRow *>(io_row.get());

        val->code = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
        val->value = MpiPacked::unpackStr(i_packedSize, i_packedData, io_packPos);
    }

    // generic (code,value) row: return byte size to pack db row into MPI message
    template<>
    int RowMpiPackedAdapter<CodeValueRow>::packedSize(const IRowBaseUptr & i_row)
    {
        const CodeValueRow * val = dynamic_cast<const CodeValueRow *>(i_row.get());
        return
            MpiPacked::packedSize(val->code) +
            MpiPacked::packedSize(val->value);
    }

    // pack model run state into MPI message
    template<>
    void RowMpiPackedAdapter<RunStateItem>::pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos)
    {
        const RunStateItem * val = dynamic_cast<const RunStateItem *>(i_row.get());

        MpiPacked::pack<decltype(val->runId)>(val->runId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->subId)>(val->subId, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<int>((int)val->state.theStatus, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->state.progressCount)>(val->state.progressCount, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<decltype(val->state.progressValue)>(val->state.progressValue, i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<time_t>(chrono::system_clock::to_time_t(val->state.startTime), i_packedSize, io_packedData, io_packPos);
        MpiPacked::pack<time_t>(chrono::system_clock::to_time_t(val->state.updateTime), i_packedSize, io_packedData, io_packPos);
    }

    // unpack MPI message into model run state
    template<>
    void RowMpiPackedAdapter<RunStateItem>::unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos)
    {
        RunStateItem * val = static_cast<RunStateItem *>(io_row.get());

        val->runId = MpiPacked::unpack<decltype(val->runId)>(i_packedSize, i_packedData, io_packPos);
        val->subId = MpiPacked::unpack<decltype(val->subId)>(i_packedSize, i_packedData, io_packPos);
        val->state.theStatus = (ModelStatus)MpiPacked::unpack<int>(i_packedSize, i_packedData, io_packPos);
        val->state.progressCount = MpiPacked::unpack<decltype(val->state.progressCount)>(i_packedSize, i_packedData, io_packPos);
        val->state.progressValue = MpiPacked::unpack<decltype(val->state.progressValue)>(i_packedSize, i_packedData, io_packPos);
        val->state.startTime = chrono::system_clock::from_time_t(MpiPacked::unpack<time_t>(i_packedSize, i_packedData, io_packPos));
        val->state.updateTime = chrono::system_clock::from_time_t(MpiPacked::unpack<time_t>(i_packedSize, i_packedData, io_packPos));
    }

    // return byte size to pack model run state into MPI message
    template<>
    int RowMpiPackedAdapter<RunStateItem>::packedSize(const IRowBaseUptr & i_row)
    {
        const RunStateItem * val = dynamic_cast<const RunStateItem *>(i_row.get());
        return
            MpiPacked::packedSize(typeid(val->runId)) +
            MpiPacked::packedSize(typeid(val->subId)) +
            MpiPacked::packedSize(typeid(int)) +
            MpiPacked::packedSize(typeid(val->state.progressCount)) +
            MpiPacked::packedSize(typeid(val->state.progressValue)) +
            MpiPacked::packedSize(typeid(time_t)) +
            MpiPacked::packedSize(typeid(time_t));
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
    case MsgTag::entityDic:
        return new MetaMpiPackedAdapter<MsgTag::entityDic, EntityDicRow>();
    case MsgTag::entityAttr:
        return new MetaMpiPackedAdapter<MsgTag::entityAttr, EntityAttrRow>();
    case MsgTag::codeValue:
        return new MetaMpiPackedAdapter<MsgTag::codeValue, CodeValueRow>();
    case MsgTag::statusUpdate:
        return new MetaMpiPackedAdapter<MsgTag::statusUpdate, RunStateItem>();
    default:
        throw MsgException("Fail to create message adapter: invalid message tag");
    }
}
#endif  // OM_MSG_MPI
