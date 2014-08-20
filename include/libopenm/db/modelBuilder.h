/**
 * @file
 * OpenM++ data library: public interface to produce model creation script
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_BUILDER_H
#define MODEL_BUILDER_H

#include <list>
#include "libopenm/db/dbMetaRow.h"
#include "libopenm/db/metaModelHolder.h"
#include "libopenm/db/metaSetHolder.h"

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
        virtual void build(MetaModelHolder & io_metaRows) = 0;

        /** start sql script to create new working set */
        virtual void beginWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet) = 0;

        /** append scalar parameter value to sql script for new working set  creation */
        virtual void addWorksetParameter(const MetaModelHolder & i_metaRows, const string & i_name, const string & i_value) = 0;

        /** append parameter values to sql script for new working set  creation */
        virtual void addWorksetParameter(
            const MetaModelHolder & i_metaRows, const string & i_name, const list<string *> & i_valueLst
            ) = 0;

        /** finish sql script to create new working set */
        virtual void endWorkset(void) const = 0;

        /** write sql script to create backward compatibility views */
        virtual void buildCompatibilityViews(const MetaModelHolder & i_metaRows) const = 0;
    };
}

#endif  // MODEL_BUILDER_H
