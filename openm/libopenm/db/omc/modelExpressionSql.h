/**
 * @file
 * OpenM++ data library: classes to translate output table accumulators and expressions into sql
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MODEL_EXPRESSION_SQL_H
#define MODEL_EXPRESSION_SQL_H

#include "libopenm/db/dbMetaRow.h"
using namespace std;

namespace openm
{
    /** aggregation and non-aggregation function code */
    enum class FncCode
    {
        /** undefined function */
        undefined = 0,

        /** if condition then value else other value */
        if_case,

        /** denominator: wrap value as divisor */
        denom,

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

    /** base class to produce sql expressions and subqueries for otput table */
    class ModelBaseExpressionSql
    {
    public:
        /**
         * initialization: store output table definition parts.
         *
         * @param[in]   i_accTableName  accumulator table name in database
         * @param[in]   i_accIdVec      ids of table accumulators
         * @param[in]   i_accNameVec    names of table accumulators
         * @param[in]   i_dimNameVec    names of table dimensions
         */
        ModelBaseExpressionSql(
            const string & i_accTableName, const vector<string> & i_dimNameVec, const vector<int> & i_accIdVec, const vector<string> & i_accNameVec
            ) :
            accTableName(i_accTableName),
            dimNameVec(i_dimNameVec),
            accIdVec(i_accIdVec),
            accNameVec(i_accNameVec)
        { }

        /** release sql builder resources. */
        ~ModelBaseExpressionSql() noexcept { }

    protected:
        /** accumulator database table name */
        const string accTableName;

        /** names of table dimensions */
        const vector<string> dimNameVec;

        /** ids of table accumulators */
        const vector<int> accIdVec;

        /** names of table accumulators */
        const vector<string> accNameVec;

        /** translate (substitute) all non-aggregation functions
        *
        * @param i_srcMsg     source table name and expression name to be used in error message
        * @param i_isSkipAggr if true then skip aggregation functions else throw an exception
        * @param i_expr       expression to translate
        */
        const string ModelBaseExpressionSql::translateAllSimpleFnc(const string & i_srcMsg, bool i_isSkipAggr, const string & i_expr);

        /** translate (substitute) non-aggregation function */
        static const string translateSimpleFnc(const string & i_srcMsg, FncCode i_code, const string & i_arg);

    private:
        ModelBaseExpressionSql(const ModelBaseExpressionSql & i_other) = delete;
        ModelBaseExpressionSql & operator=(const ModelBaseExpressionSql & i_other) = delete;
    };

    /** class to produce accumulators sql subqueries for otput table */
    class ModelAccumulatorSql : public ModelBaseExpressionSql
    {
    public:
        /**
         * initialization: store output table definition parts.
         *
         * @param[in]   i_accTableName  accumulator table name in database
         * @param[in]   i_accIdVec      ids of table accumulators
         * @param[in]   i_accNameVec    names of table accumulators
         * @param[in]   i_dimNameVec    names of table dimensions
         */
        ModelAccumulatorSql(
            const string & i_accTableName,
            const vector<string> & i_dimNameVec, 
            const vector<int> & i_accIdVec, 
            const vector<string> & i_accNameVec, 
            const vector<TableAccRow> & i_tableAcc
            ) :
            ModelBaseExpressionSql(i_accTableName, i_dimNameVec, i_accIdVec, i_accNameVec),
            tableAccVec(i_tableAcc)
        { }

        /** release sql builder resources. */
        ~ModelAccumulatorSql() noexcept { }

        /**
         * translate output table "native" (non-derived) accumulator into sql CTE subquery.
         *
         * @param i_outTableName    output table name
         * @param i_accName         accumulator name
         * @param i_accId           accumulator id
         *
         * @return  sql select subquery
         */
        const string translateNativeAccExpr(const string & i_outTableName, const string & i_accName, int i_accId);

        /**
         * translate output table derived accumulator into sql CTE subquery.
         *
         * @param i_outTableName    output table name
         * @param i_accName         accumulator name
         * @param i_expr            source expression, ie: acc0 + acc1.
         * @param i_nativeMap       native accumlators map of (name, accumulator index)
         *
         * @return  sql select subquery
         */
        const string translateDerivedAccExpr(const string & i_outTableName, const string & i_accName, const string & i_expr, const map<string, size_t> & i_nativeMap);

    private:
        /** current source name: output table name and accumulator name */
        string srcMsg;

        /** table_acc table rows */
        const vector<TableAccRow> & tableAccVec;

        /** contains true if accumulator used at current level */
        vector<bool> isAccUsedArr;

    private:
        ModelAccumulatorSql(const ModelAccumulatorSql & i_other) = delete;
        ModelAccumulatorSql & operator=(const ModelAccumulatorSql & i_other) = delete;
    };

    /** class to produce aggregation sql for otput table */
    class ModelAggregationSql : public ModelBaseExpressionSql
    {
    public:
        /**
         * create aggregation sql builder for output table.
         *
         * @param[in]   i_accTableName  accumulator table name in database
         * @param[in]   i_accIdVec      ids of table accumulators
         * @param[in]   i_accNameVec    names of table accumulators
         * @param[in]   i_dimNameVec    names of table dimensions
         */
        ModelAggregationSql(
            const string & i_accTableName, 
            const vector<string> & i_dimNameVec, 
            const vector<int> & i_accIdVec, 
            const vector<string> & i_accNameVec
            ) :
            ModelBaseExpressionSql(i_accTableName, i_dimNameVec, i_accIdVec, i_accNameVec)
        { }

        /** release sql builder resources. */
        ~ModelAggregationSql() noexcept { }

        /**
         * translate output aggregation expression into sql.
         *
         * @param i_outTableName    output table name
         * @param i_name            expression name, ie: expr2.
         * @param i_expr            source expression, ie: OM_AVG(acc2).
         *
         * @return  aggergation sql select query
         */
        const string translateAggregationExpr(const string & i_outTableName, const string & i_name, const string & i_expr);

    private:
        /** current source name: output table name and expression name */
        string srcMsg;

        /** contains true if accumulator used at current level */
        vector<bool> isAccUsedArr;

        /** aggregation expressions for the next level */
        vector<AggregationColumnExpr> nextExprArr;

        /** translate aggregation and non-aggregation function into sql */
        const string translateAggregationFnc(FncCode i_code, const string & i_nextInnerAlias, const string & i_arg);

        /** translate function argument into sql argument */
        const string translateArg(const string & i_nextInnerAlias, const string & i_arg);

        /** collect accumulator name usage in expression */
        const string processAccumulators(const string & i_expr);

        /** insert table alias in front of accumulator names */
        const string processAccumulators(bool i_isTranslate, int i_level, const string & i_fromAlias, const string & i_expr);

        /** push OM_AVG functions to next aggregation level and return column name */
        const string pushAvgToNextLevel(const string & i_arg) { return pushToNextLevel("OM_AVG(" + i_arg + ")"); }

        /** push OM_ function to next aggregation level and return column name */
        const string pushToNextLevel(const string & i_fncExpr);

        /** return true if this is first used accumulator */
        bool isFirstUsedAcc(int i_accPos, const vector<bool> & i_accUsage) const;

        /** make accumulator table alias or return fromAlias for the first used accumulator */
        const string makeAccTableAlias(int i_accPos, const vector<bool> & i_accUsage, int i_level, const string i_firstAlias) const;

    private:
        ModelAggregationSql(const ModelAggregationSql & i_other) = delete;
        ModelAggregationSql & operator=(const ModelAggregationSql & i_other) = delete;
    };
}

#endif  // MODEL_EXPRESSION_SQL_H
