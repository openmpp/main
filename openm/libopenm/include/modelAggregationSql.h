/**
 * @file
 * OpenM++ data library: classes to translate output table aggregation expressions into sql
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_AGGREGATION_SQL_H
#define MODEL_AGGREGATION_SQL_H

#include "libopenm/db/dbMetaRow.h"
using namespace std;

namespace openm
{
    /** aggregation function names */
    struct AggregationFnc
    {
        /** average value */
        static const char * avg;

        /** sum of values */
        static const char * sum;

        /** count of values */
        static const char * count;

        /** minimal value */
        static const char * min;

        /** maximum value */
        static const char * max;

        /** variance */
        static const char * var;

        /** standard deviation */
        static const char * sd;

        /** standard error */
        static const char * se;

        /** coefficient of variation */
        static const char * cv;
    };

    /** aggregation function code */
    enum class FncCode
    {
        /** undefined aggregation function */
        undefined = 0,

        /** average value */
        avg,

        /** sum of values */
        sum,

        /** count of values */
        count,

        /** minimal value */
        min,

        /** maximum value */
        max,

        /** variance */
        var,

        /** standard deviation */
        sd,

        /** standard error */
        se,

        /** coefficient of variation */
        cv
    };

    /** class to produce aggregation sql for otput table */
    class ModelAggregationSql
    {
    public:
        /**
         * create aggregation sql builder for output table.
         *
         * @param[in] i_tableName subsamples table name in database
         * @param[in] i_alias     subsample table alias, ie: S
         * @param[in] i_accVec    name vector of table accumulators
         * @param[in] i_dimVec    name vector of table dimensions
         */
        ModelAggregationSql(
            const string & i_tableName, const string & i_alias, const vector<string> & i_accVec, const vector<string> & i_dimVec
            ) :
            joinCount(0),
            tableName(i_tableName),
            mainAlias(i_alias),
            accNameVec(i_accVec),
            dimNameVec(i_dimVec)
        { }

        /** release sql builder resources. */
        ~ModelAggregationSql() throw() { }

        /** translate output aggregation expression into sql */
        const string translateAggregationExpr(const string & i_expr);

    private:
        /** join count: correlated subquery count */
        int joinCount;

        /** subsamples table name */
        const string tableName;

        /** subsample table alias for top most query */
        const string mainAlias;

        /** name vector of table accumulators */
        const vector<string> accNameVec;

        /** name vector of table dimensions */
        const vector<string> dimNameVec;

        /** translate aggregation function into sql */
        const string translateFnc(FncCode i_code, int i_level, const string & i_arg);

        /** translate function argument: decorate accumulator name with alias */
        const string translateArg(const string & i_alias, const string & i_arg);

        /** insert table alias in front of accumulator names */
        const string insertAliasAcc(const string & i_alias, const string & i_expr);

        /** return join conditions for corelated subquery */
        const string makeJoinCondition(const string & i_subAlias);

        /** build basic sql aggregation subquery, ie: AVG */
        const string sqlSubquery(const string & i_sqlFncName, const string & i_arg);

        /** return basic sql aggregation expression, ie: AVG(i_arg) */
        const string sqlAggregation(const string & i_sqlFncName, const string & i_alias, const string & i_arg);

        /** build sql variance aggregation as subquery */
        const string sqlVarSubquery(const string & i_arg);

        /** return sql variance aggregation as expression with argument */
        const string sqlVarAggregation(const string & i_alias, const string & i_arg);

        /** build sql SD aggregation as subquery */
        const string sqlSdSubquery(const string & i_arg);

        /** return sql SD aggregation as expression with argument */
        const string sqlSdAggregation(const string & i_alias, const string & i_arg);

        /** build sql SE aggregation as subquery */
        const string sqlSeSubquery(const string & i_arg);

        /** return sql SE aggregation as expression with argument */
        const string sqlSeAggregation(const string & i_alias, const string & i_arg);

        /** build sql CV aggregation as subquery */
        const string sqlCvSubquery(const string & i_arg);

        /** return sql CV aggregation as expression with argument */
        const string sqlCvAggregation(const string & i_alias, const string & i_arg);

    private:
        ModelAggregationSql(const ModelAggregationSql & i_other);               // = delete;
        ModelAggregationSql & operator=(const ModelAggregationSql & i_other);   // = delete;
    };
}

#endif  // MODEL_AGGREGATION_SQL_H
