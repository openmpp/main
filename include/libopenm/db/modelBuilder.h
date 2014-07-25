/**
 * @file
 * OpenM++ data library: public interface to produce model creation script
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_BUILDER_H
#define MODEL_BUILDER_H

#include "libopenm/db/dbMetaRow.h"
#include "libopenm/db/metaModelHolder.h"

using namespace std;

namespace openm
{
    /** public interface to build model creation script from metadata rows. */
    struct IModelBuilder
    {
    public:
        virtual ~IModelBuilder() throw() = 0;

        /** create new model builder. */
        static IModelBuilder * create(const string & i_outputDir);

        /** return timestamp string, ie: _201208171604590148_  result does not changed during object lifetime. */
        virtual const string timeStamp(void) const = 0;

        /** update metadata and write sql script to create new model from supplied metadata rows */
        virtual void build(MetaModelHolder & i_metaRows) = 0;

        /** write sql script to create backward compatibility views (Modgen compatibility) */
        virtual const void buildCompatibilityViews(const MetaModelHolder & i_metaRows) const = 0;

        /** return sql script to insert parameters if template file exists */
        virtual const string buildInsertParameters(const MetaModelHolder & i_metaRows, const string & i_sqlTemplateFilePath) const = 0;
    };
}

#endif  // MODEL_BUILDER_H
