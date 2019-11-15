/**
 * @file
 * OpenM++ modeling library: holder struct for metadata tables used by model.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "metaHolder.h"
using namespace openm;

/** return input parameter size: total number of values in the parameter as product of all of dimensions size */
size_t MetaHolder::parameterSize(const ParamDicRow & i_paramRow) const
{
    // get dimensions list
    const vector<ParamDimsRow> dims = paramDims->byModelIdParamId(i_paramRow.modelId, i_paramRow.paramId);
    if (i_paramRow.rank != (int)dims.size()) throw DbException("invalid parameter rank or dimensions not found for parameter: %s", i_paramRow.paramName.c_str());

    // get dimensions type and size, calculate total size
    size_t totalSize = 1;
    for (const ParamDimsRow & d : dims) {

        const TypeDicRow * typeRow = typeDic->byKey(i_paramRow.modelId, d.typeId);
        if (typeRow == nullptr) throw DbException("type not found for dimension %s of parameter: %s", d.name.c_str(), i_paramRow.paramName.c_str());

        int dimSize = (int)typeEnumsSize(i_paramRow.modelId, d.typeId);

        if (dimSize > 0) totalSize *= dimSize;  // can be simple type, without enums in enum list
    }
    if (totalSize <= 0) throw DbException("invalid size of the parameter: %s", i_paramRow.paramName.c_str());

    return totalSize;
}

/** return accumulator size: total number of accumulator values as product of dimensions size, including total item */
size_t MetaHolder::accumulatorSize(const TableDicRow & i_tableRow) const
{
    const vector<TableDimsRow> dims = tableDims->byModelIdTableId(i_tableRow.modelId, i_tableRow.tableId);
    if (i_tableRow.rank != (int)dims.size()) throw DbException("invalid table rank or dimensions not found for table: %s", i_tableRow.tableName.c_str());

    size_t totalSize = 1;
    for (const TableDimsRow & d : dims) {
        if (d.dimSize <= 0) throw DbException("invalid size of dimension %s for table: %s", d.name.c_str(), i_tableRow.tableName.c_str());

        totalSize *= (size_t)d.dimSize;
    }
    if (totalSize <= 0) throw DbException("invalid size of the table: %s", i_tableRow.tableName.c_str());

    return totalSize;
}

/** return type enums size: number of enums for that type or zero if no enumms found (most of built-in types) */
size_t MetaHolder::typeEnumsSize(int i_modelId, int i_typeId) const
{
    // compare type enum rows only by model id and type id
    auto cmp = [](const IRowBaseUptr & i_left, const IRowBaseUptr & i_right) -> bool {
        TypeEnumLstRow * leftRow = dynamic_cast<TypeEnumLstRow *>(i_left.get());
        TypeEnumLstRow * rightRow = dynamic_cast<TypeEnumLstRow *>(i_right.get());
        return
            (leftRow->modelId < rightRow->modelId) ||
            (leftRow->modelId == rightRow->modelId && leftRow->typeId < rightRow->typeId);
    };
    const IRowBaseUptr enumRowUptr = make_unique<TypeEnumLstRow>(i_modelId, i_typeId, 0);

    auto lowerIt = lower_bound(typeEnumLst->rowsCRef().cbegin(), typeEnumLst->rowsCRef().cend(), enumRowUptr, cmp);

    if (lowerIt == typeEnumLst->rowsCRef().cend()) return 0;    // type enums not found

    return distance(
        lowerIt, 
        upper_bound(typeEnumLst->rowsCRef().cbegin(), typeEnumLst->rowsCRef().cend(), enumRowUptr, cmp)
    );
}




