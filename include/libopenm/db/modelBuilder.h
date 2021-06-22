/**
 * @file
 * OpenM++ data library: public interface to produce model creation script
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_MODEL_BUILDER_H
#define OM_MODEL_BUILDER_H

#include <list>
#include "dbMetaRow.h"
#include "metaModelHolder.h"
#include "metaSetHolder.h"

using namespace std;

namespace openm
{
    /** public interface to build model creation script from metadata rows. */
    struct IModelBuilder
    {
    public:
        virtual ~IModelBuilder() noexcept = 0;

        /** new model builder to create sql script specific to each db-provider. */
        static IModelBuilder * create(const string & i_providerNames, const string & i_sqlDir, const string & i_outputDir, const string & i_sqliteDir);

        /** if true then create SQLite database */
        virtual bool isSqliteDb(void) const = 0;

        /** set meta rows values and calculate digests for types, parameters, tables and model */
        virtual void setModelMetaRows(MetaModelHolder & io_metaRows) const = 0;

        /** update metadata and write sql script to create new model from supplied metadata rows */
        virtual void build(MetaModelHolder & io_metaRows) = 0;

        /** start sql script to create new working set */
        virtual void beginWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet) = 0;

        /** append scalar parameter value to sql script for new working set creation */
        virtual void addWorksetParameter(
            const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet, const string & i_name, int i_subId, const string & i_value
        ) = 0;

        /** append parameter values to sql script for new working set creation */
        virtual void addWorksetParameter(
            const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet, const string & i_name, int i_subId, const list<string> & i_valueLst
        ) = 0;

        /** finish sql script to create new working set */
        virtual void endWorkset(const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet) = 0;

        /** write sql script to create backward compatibility views */
        virtual void buildCompatibilityViews(const MetaModelHolder & i_metaRows) const = 0;
    };
}

#endif  // OM_MODEL_BUILDER_H
