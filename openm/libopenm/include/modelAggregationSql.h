/**
 * @file
 * OpenM++ data library: classes to translate output table aggregation expressions into sql
 */
// Copyright (c) 2013-2014 OpenM++
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

    /** column aggregation expression */
    struct AggregationColumnExpr
    {
        /** column name, ie: ex2 */
        string colName;

        /** source expression, ie: OM_AVG(acc0) */
        string srcExpr;

        /** sql expresiion, ie: AVG(M2.acc0) */
        string sqlExpr;

        /** number of aggregation epxpressions */
        static int nextExprNumber;

        AggregationColumnExpr(const string & i_colName, const string & i_srcExpr) :
            colName(i_colName), srcExpr(i_srcExpr)
        { }
        AggregationColumnExpr(const string & i_srcExpr) :
            AggregationColumnExpr("ex" + to_string(nextExprNumber++), i_srcExpr)
        { }
    };

    /** class to produce aggregation sql for otput table */
    class ModelAggregationSql
    {
    public:
        /**
         * create aggregation sql builder for output table.
         *
         * @param[in] i_accFlatName accumulator flat view name in database
         * @param[in] i_accVec      name vector of table accumulators
         * @param[in] i_dimVec      name vector of table dimensions
         */
        ModelAggregationSql(
            const string & i_accFlatName, const vector<string> & i_accVec, const vector<string> & i_dimVec
            ) :
            accFlatName(i_accFlatName),
            accNameVec(i_accVec),
            dimNameVec(i_dimVec)
        { }

        /** release sql builder resources. */
        ~ModelAggregationSql() throw() { }

        /**
         * translate output aggregation expression into sql.
         *
         * @param   i_name  expression name, ie: expr2.
         * @param   i_expr  source expression, ie: OM_AVG(acc2).
         *
         * @return  aggergation sql select query
         */
        const string translateAggregationExpr(const string & i_name, const string & i_expr);

    private:
        /** accunulator flat view name */
        const string accFlatName;

        /** name vector of table accumulators */
        const vector<string> accNameVec;

        /** name vector of table dimensions */
        const vector<string> dimNameVec;

        /** aggregation expressions for the next level */
        vector<AggregationColumnExpr> nextExprArr;

        /** translate aggregation function into sql */
        const string translateFnc(
            FncCode i_code, const string & i_fromAlias, const string & i_innerAlias, const string & i_arg
            );

        /** translate function argument into sql argument */
        const string translateArg(
            const string & i_fromAlias, const string & i_innerAlias, const string & i_arg
            );

        /** insert table alias in front of accumulator names */
        const string insertAliasAcc(const string & i_alias, const string & i_expr);

        /** push OM_AVG functions to next aggregation level and return column name */
        const string pushAvgToNextLevel(const string & i_arg) { return pushToNextLevel("OM_AVG(" + i_arg + ")"); }

        /** push OM_ function to next aggregation level and return column name */
        const string pushToNextLevel(const string & i_fncExpr);

    private:
        ModelAggregationSql(const ModelAggregationSql & i_other);               // = delete;
        ModelAggregationSql & operator=(const ModelAggregationSql & i_other);   // = delete;
    };
}

#endif  // MODEL_AGGREGATION_SQL_H
