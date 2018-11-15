// OpenM++ data library: classes to translate output table aggregation expressions into sql
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "modelExpressionSql.h"
using namespace openm;

/** average value */
const char * AggregationFnc::avg = "OM_AVG";

/** sum of values */
const char * AggregationFnc::sum = "OM_SUM";

/** count of values */
const char * AggregationFnc::count = "OM_COUNT";

/** minimal value */
const char * AggregationFnc::min = "OM_MIN";

/** maximum value */
const char * AggregationFnc::max = "OM_MAX";

/** variance */
const char * AggregationFnc::var = "OM_VAR";

/** standard deviation */
const char * AggregationFnc::sd = "OM_SD";

/** standard error */
const char * AggregationFnc::se = "OM_SE";

/** coefficient of variation */
const char * AggregationFnc::cv = "OM_CV";

// aggregation function code and name
struct FncTag
{
    FncCode code;       // aggregation function code
    const char * name;  // aggregation function name
    size_t nameLen;     // char size of function name
};

static FncTag fncTagArr[] = {
    {FncCode::avg, AggregationFnc::avg, strlen(AggregationFnc::avg)},
    {FncCode::sum, AggregationFnc::sum, strlen(AggregationFnc::sum)},
    {FncCode::count, AggregationFnc::count, strlen(AggregationFnc::count)},
    {FncCode::min, AggregationFnc::min, strlen(AggregationFnc::min)},
    {FncCode::max, AggregationFnc::max, strlen(AggregationFnc::max)},
    {FncCode::var, AggregationFnc::var, strlen(AggregationFnc::var)},
    {FncCode::sd, AggregationFnc::sd, strlen(AggregationFnc::sd)},
    {FncCode::se, AggregationFnc::se, strlen(AggregationFnc::se)},
    {FncCode::cv, AggregationFnc::cv, strlen(AggregationFnc::cv)}
};
static const size_t fncTagArrSize = sizeof(fncTagArr) / sizeof(FncTag);

/** left side accumulator name delimiters */
static const char leftDelimArr[] = {
    '(', '+', '-', '*', '/', '%', '^', '|', '&', '~', '!', '=', '<', '>'
};
static const size_t leftDelimSize = sizeof(leftDelimArr) / sizeof(char);

/** right side accumulator name delimiters */
static const char rightDelimArr[] = {
    ')', '+', '-', '*', '/', '%', '^', '|', '&', '~', '!', '=', '<', '>'
};
static const size_t rightDelimSize = sizeof(rightDelimArr) / sizeof(char);

/** aggregation function name and positions in input expression string */
struct FncToken
{
    /** aggregation function code */
    FncCode code;

    /** function name position */
    size_t namePos;

    /** open bracket position */
    size_t openPos;

    /** close bracket position */
    size_t closePos;

    /** create empty function token */
    FncToken(void) : 
        code(FncCode::undefined) , namePos(0), openPos(0), closePos(0)
    { }

    /** get next function from expresiion string */
    static const FncToken next(const string & i_expr);
};

static size_t skipIfQuoted(size_t i_pos, const string & i_str);

// Parsed aggregation expressions for each nesting level
struct LevelDef
{
    int level = 0;                          // nesting level
    string fromAlias;                       // from table alias
    string innerAlias;                      // inner join table alias
    vector<AggregationColumnExpr> exprArr;  // column names and expressions
    vector<bool> accUsageArr;               // contains true if accumulator used at current level

    LevelDef(int i_level) : level(i_level)
    {
        fromAlias = "M" + to_string(level);
        innerAlias = "T" + to_string(level + 1);
    }
};

int AggregationColumnExpr::nextExprNumber = 1;   // next aggregation expression number

/** translate output aggregation expression into sql */
const string ModelAggregationSql::translateAggregationExpr(const string & i_name, const string & i_expr)
{
    vector<LevelDef> levelArr;      // array of aggregation expressions by levels

    // start with source expression and column name
    int level = 1;

    levelArr.emplace_back(level);
    LevelDef & currLevel = levelArr.back();
    currLevel.exprArr.emplace_back(i_name, i_expr);

    // until any expressions to parse repeat translation
    bool isFound = false;
    do {
        isAccUsedArr.assign(accNameVec.size(), false);  // clear accumulator usage flags

        for (AggregationColumnExpr & currExpr : currLevel.exprArr) {

            string expr = currExpr.srcExpr;

            // parse until source expression not completed
            while (!expr.empty()) {

                // find next function in current expression
                FncToken fnc = FncToken::next(expr);
                if (fnc.code == FncCode::undefined) {   // no function found: copy source expression into output
                    currExpr.sqlExpr += expr;
                    expr.clear();
                }
                else {

                    // if anything before the function then copy it into output
                    if (fnc.namePos != 0) currExpr.sqlExpr += expr.substr(0, fnc.namePos);

                    // translate function into sql expression and append to output
                    currExpr.sqlExpr += translateFnc(
                        fnc.code, currLevel.innerAlias, expr.substr(fnc.openPos + 1, fnc.closePos - (fnc.openPos + 1))
                        );

                    // remove parsed function from expression
                    expr = (fnc.closePos + 1 < expr.length()) ? expr.substr(fnc.closePos + 1) : "";
                }
            }
        }

        // second pass: translate accumulators for all sql expressions
        currLevel.accUsageArr = isAccUsedArr;

        for (AggregationColumnExpr & expr : currLevel.exprArr) {
            expr.sqlExpr = processAccumulators(true, level, currLevel.fromAlias, expr.sqlExpr);
        }

        // if any expressions pushed to the next level then continue parsing
        isFound = nextExprArr.size() > 0;

        if (isFound) {
            levelArr.emplace_back(++level);
            currLevel = levelArr.back();
            currLevel.exprArr.swap(nextExprArr);
        }
    }
    while (isFound);

    // build output sql for expression:
    // 
    // OM_SUM(acc0 + 0.5 * OM_AVG(acc1 + acc4 + 0.1 * (OM_MAX(acc0) - OM_MIN(acc1)) ))
    // =>
    //   SELECT 
    //     M1.run_id, M1.dim0, M1.dim1, 
    //     SUM(M1.acc_value + 0.5 * T2.ex2) AS ex1
    //   FROM age_acc M1
    //   INNER JOIN 
    //   (
    //     SELECT 
    //       M2.run_id, M2.dim0, M2.dim1, 
    //       AVG(M2.acc_value + A4.acc4 + 0.1 * (T3.ex31 - T3.ex32)) AS ex2
    //     FROM age_acc M2
    //     INNER JOIN
    //     (
    //       SELECT run_id, dim0, dim1, sub_id, acc_value AS acc4 FROM age_acc WHERE acc_id = 4
    //     ) L2A4
    //     ON (L2A4.run_id = M2.run_id AND L2A4.dim0 = M2.dim0 AND L2A4.dim1 = M2.dim1 AND L2A4.sub_id = M2.sub_id)
    //     INNER JOIN 
    //     (
    //       SELECT 
    //         M3.run_id, M3.dim0, M3.dim1, 
    //         MAX(M3.acc_value) AS ex31, 
    //         MIN(A3.acc1) AS ex32
    //       FROM age_acc M3
    //       INNER JOIN
    //       (
    //         SELECT run_id, dim0, dim1, sub_id, acc_value AS acc1 FROM age_acc WHERE acc_id = 1
    //       ) L3A1
    //       ON (L3A1.run_id = M3.run_id AND L3A1.dim0 = M3.dim0 AND L3A1.dim1 = M3.dim1 AND L3A1.sub_id = M3.sub_id)
    //       WHERE M3.acc_id = 0
    //       GROUP BY M3.run_id, M3.dim0, M3.dim1
    //     ) T3
    //     ON (T3.run_id = M2.run_id AND T3.dim0 = M2.dim0 AND T3.dim1 = M2.dim1)
    //     WHERE M2.acc_id = 1
    //     GROUP BY M2.run_id, M2.dim0, M2.dim1
    //   ) T2
    //   ON (T2.run_id = M1.run_id AND T2.dim0 = M1.dim0 AND T2.dim1 = M1.dim1)
    //   WHERE M1.acc_id = 0
    //   GROUP BY M1.run_id, M1.dim0, M1.dim1
    //
    string sql;
    
    for (int nLev = 0; nLev < (int)levelArr.size(); nLev++) {

        sql += "SELECT " + levelArr[nLev].fromAlias + ".run_id";

        for (const string & dimName : dimNameVec) {
            sql += ", " + levelArr[nLev].fromAlias + "." + dimName;
        }

        for (const AggregationColumnExpr & expr : levelArr[nLev].exprArr) {
            sql += ", " + expr.sqlExpr + " AS " + expr.colName;
        }

        sql += " FROM " + accTableName + " " + levelArr[nLev].fromAlias;

        for (int nAcc = 0; nAcc < (int)accNameVec.size(); nAcc++) {

            if (!levelArr[nLev].accUsageArr[nAcc] || isFirstUsedAcc(nAcc, levelArr[nLev].accUsageArr)) continue;

            string accAlias = makeAccTableAlias(
                nAcc, levelArr[nLev].accUsageArr, levelArr[nLev].level, levelArr[nLev].fromAlias
                );

            sql += " INNER JOIN (SELECT run_id, ";

            for (const string & dimName : dimNameVec) {
                sql += dimName + ", ";
            }

            sql += "sub_id, acc_value AS " + accNameVec[nAcc] +
                " FROM " + accTableName +
                " WHERE acc_id = " + to_string(accIdVec[nAcc]) +
                ") " + accAlias;

            sql += " ON (" + accAlias + ".run_id = " + levelArr[nLev].fromAlias + ".run_id";

            for (const string & dimName : dimNameVec) {
                sql += " AND " + accAlias + "." + dimName + " = " + levelArr[nLev].fromAlias + "." + dimName;
            }

            sql += " AND " + accAlias + ".sub_id = " + levelArr[nLev].fromAlias + ".sub_id)";
        }

        if (nLev < (int)levelArr.size() - 1) sql += " INNER JOIN (";
    }

    for (int nLev = (int)levelArr.size() - 1; nLev >= 0; nLev--) {

        int firstAccId = -1;
        for (int nAcc = 0; nAcc < (int)accIdVec.size(); nAcc++) {
            if (levelArr[nLev].accUsageArr[nAcc]) {
                firstAccId = accIdVec[nAcc];
                break;
            }
        }

        sql += " WHERE " + levelArr[nLev].fromAlias + ".acc_id = " + (firstAccId < 0 ? "0" : to_string(firstAccId));

        sql += " GROUP BY " + levelArr[nLev].fromAlias + ".run_id";

        for (const string & dimName : dimNameVec) {
            sql += ", " + levelArr[nLev].fromAlias + "." + dimName;
        }

        if (nLev > 0) {

            sql += ") " + levelArr[nLev].innerAlias +
                " ON (" + levelArr[nLev].innerAlias + ".run_id = " + levelArr[nLev].fromAlias + ".run_id";

            for (const string & dimName : dimNameVec) {
                sql += " AND " + levelArr[nLev].innerAlias + "." + dimName + " = " + levelArr[nLev].fromAlias + "." + dimName;
            }

            sql += ")";
        }
    }

    return sql;
}

// translate aggregation function into sql:
//  OM_AVG(acc0) => AVG(M1.acc0)
// or:
//  OM_SUM(acc0 - 0.5 * OM_AVG(acc0)) => SUM(M1.acc0 - 0.5 * T2.ex2)
// or:
//  OM_VAR(acc0)
//  =>
//  OM_SUM((acc0 - OM_AVG(acc0)) * (acc0 - OM_AVG(acc0))) / (OM_COUNT(acc0) – 1)
//  =>
//  SUM((M1.acc0 - T2.ex2) * (M1.acc0 - T2.ex2)) / (COUNT(M1.acc0) – 1)
//
const string ModelAggregationSql::translateFnc(
    FncCode i_code, const string & i_innerAlias, const string & i_arg
    )
{
    // check arguments
    if (i_arg.length() < 1) throw DbException(LT("Invalid (empty) function %d argument"), i_code);

    string sqlArg = translateArg(i_innerAlias, i_arg);
    string avgCol;

    // return aggregation sql expression
    switch (i_code) {
    case FncCode::avg:
        return "AVG(" + sqlArg + ")";

    case FncCode::sum:
        return "SUM(" + sqlArg + ")";

    case FncCode::count:
        return "COUNT(" + sqlArg + ")";

    case FncCode::min:
        return "MIN(" + sqlArg + ")";

    case FncCode::max:
        return "MAX(" + sqlArg + ")";

    case FncCode::var:  // SUM((arg - T2.ex2) * (arg - T2.ex2)) / (COUNT(arg) - 1)
        
        avgCol = pushAvgToNextLevel(i_arg);
        return
            "SUM(((" + sqlArg + ") - " + i_innerAlias + "." + avgCol + ") *" \
            " ((" + sqlArg + ") - " + i_innerAlias + "." + avgCol + "))" \
            " / (COUNT(" + sqlArg + ") – 1)";

    case FncCode::sd:   // SQRT(var)
        
        avgCol = pushAvgToNextLevel(i_arg);
        return
            "SQRT(" \
            " SUM(((" + sqlArg + ") - " + i_innerAlias + "." + avgCol + ") *" \
            " ((" + sqlArg + ") - " + i_innerAlias + "." + avgCol + "))" \
            " / (COUNT(" + sqlArg + ") – 1)" \
            " )";

    case FncCode::se:   // SQRT(var / COUNT(arg))

        avgCol = pushAvgToNextLevel(i_arg);
        return
            "SQRT(" \
            " SUM(((" + sqlArg + ") - " + i_innerAlias + "." + avgCol + ") *" \
            " ((" + sqlArg + ") - " + i_innerAlias + "." + avgCol + "))" \
            " / (COUNT(" + sqlArg + ") – 1)" \
            " / COUNT(" + sqlArg + ") )";

    case FncCode::cv:   // 100 * ( SQRT(var) / AVG(arg) ) 
        
        avgCol = pushAvgToNextLevel(i_arg);
        return
            "100 * (SQRT(" \
            " SUM(((" + sqlArg + ") - " + i_innerAlias + "." + avgCol + ") *" \
            " ((" + sqlArg + ") - " + i_innerAlias + "." + avgCol + "))" \
            " / (COUNT(" + sqlArg + ") – 1)" \
            " ) / AVG(" + sqlArg + ") )";

    default:
        throw DbException(LT("unknown aggregation function code: %d with arguments: %s"), i_code, i_arg.c_str());
    }
}

// translate function argument into sql argument:
// push nested OM_ functions to next aggregation level
// if accumulator outside of nested function then insert table alias in front of accumulator
const string ModelAggregationSql::translateArg(const string & i_innerAlias, const string & i_arg)
{
    // parse until source expression not completed
    string outExpr;
    string expr = i_arg;

    while (!expr.empty()) {

        // find next function in source expression
        FncToken fnc = FncToken::next(expr);
        if (fnc.code == FncCode::undefined) {    // for entire source insert alias in front of accumulators and append to output
            outExpr += processAccumulators(expr);
            expr.clear();
        }
        else {
            // if anything before the function then insert alias in front of accumulators and result to output
            if (fnc.namePos != 0) {
                outExpr += processAccumulators(expr.substr(0, fnc.namePos));
            }

            // push nested function to the next level, insert column name i.e.: T2.ex2 
            // and nested function remove from source expression
            string colName = pushToNextLevel(expr.substr(fnc.namePos, (fnc.closePos + 1) - fnc.namePos));

            outExpr += i_innerAlias + "." + colName;
            expr = (fnc.closePos + 1 < expr.length()) ? expr.substr(fnc.closePos + 1) : "";
        }
    }
    return outExpr;
}

// push OM_ function to next aggregation level and return column name 
const string ModelAggregationSql::pushToNextLevel(const string & i_fncExpr)
{
    nextExprArr.push_back(AggregationColumnExpr(i_fncExpr));
    return
        nextExprArr.back().colName;
}

// first pass: collect accumulator name usage in expression
const string ModelAggregationSql::processAccumulators(const string & i_expr)
{
    return processAccumulators(false, 0, "", i_expr);
}

// first pass: collect accumulator name usage in expression
// second pass: translate accumulator names by inserting table alias: acc1 => S.acc1
const string ModelAggregationSql::processAccumulators(
    bool i_isTranslate, int i_level, const string & i_fromAlias, const string & i_expr
    )
{
    // find accumulator in source expression
    string expr;
    bool isLeftDelim = true;

    for (size_t nPos = 0; nPos < i_expr.length(); nPos++) {

        // skip until end of "quotes" or 'apostrophes'
        size_t nSkip = skipIfQuoted(nPos, i_expr);
        if (nSkip != nPos) {
            if (nSkip < i_expr.length()) {      // append to output
                expr += i_expr.substr(nPos, (nSkip + 1) - nPos);
                nPos = nSkip;
                isLeftDelim = false;    // "quotes" or 'apostrophes' is not left delimiter
                continue;               // move to the next char after "quotes" or 'apostrophes'
            }
            else { 
                expr += i_expr.substr(nPos);
                break;      // skipped until end of string
            }
        }

        // if previous char was name left delimiter then check for accumulator name in current position
        if (isLeftDelim) {

            // check for each accumulator name
            bool isAcc = false;
            size_t nLen = 0;
            int accPos;
            for (accPos = 0; accPos < (int)accNameVec.size(); accPos++) {
                
                nLen = accNameVec[accPos].length();
                isAcc = equalNoCase(accNameVec[accPos].c_str(), i_expr.c_str() + nPos, nLen);

                // check if accumulator name end with right delimiter
                if (isAcc && nPos + nLen < i_expr.length()) {

                    char chEnd = i_expr[nPos + nLen];
                    isAcc =
                        isspace<char>(chEnd, locale::classic()) ||
                        std::any_of(
                        rightDelimArr,
                        rightDelimArr + rightDelimSize,
                        [chEnd](const char i_delim) -> bool { return chEnd == i_delim; }
                    );
                }

                if (isAcc) break;   // accumulator found
            }

            // append alias and accumulator to output
            if (isAcc) {

                if (!i_isTranslate) {
                    expr += accNameVec[accPos];     // push accumulator to output "as is"
                    isAccUsedArr[accPos] = true;    // collect accumulator usage
                }
                else {          // make accumulator db-column name
                    expr += 
                        makeAccTableAlias(accPos, isAccUsedArr, i_level, i_fromAlias) + 
                        "." + 
                        (isFirstUsedAcc(accPos, isAccUsedArr) ? "acc_value" : accNameVec[accPos]);
                }
                nPos += nLen - 1;
                isLeftDelim = false;
                continue;       // done with accumulator
            }
        }

        // append current char to output
        char chNow = i_expr[nPos];
        expr += chNow;

        // check if current char is a name delimiter
        isLeftDelim = 
            isspace<char>(chNow, locale::classic()) || 
            std::any_of(
                leftDelimArr, 
                leftDelimArr + leftDelimSize, 
                [chNow](const char i_delim) -> bool { return chNow == i_delim; }
            );
    }

    return expr;
}

// get next function from expresiion string
const FncToken FncToken::next(const string & i_expr)
{
    // find function name
    FncToken fncToken;
    size_t nOpen = 0;
    bool isLeftDelim = true;

    for (size_t nPos = 0; nPos < i_expr.length() && fncToken.code == FncCode::undefined; nPos++) {

        // skip until end of "quotes" or 'apostrophes'
        size_t nSkip = skipIfQuoted(nPos, i_expr);
        if (nSkip != nPos) {
            if (nSkip >= i_expr.length()) break;    // skipped until end of string

            nPos = nSkip;
            isLeftDelim = false;    // "quotes" or 'apostrophes' is not left delimiter
            continue;               // move to the next char after "quotes" or 'apostrophes'
        }

        // check for each function name
        if (isLeftDelim) {
            for (const FncTag & tag : fncTagArr) {

                if (equalNoCase(tag.name, i_expr.c_str() + nPos, tag.nameLen)) {
                    fncToken.code = tag.code;
                    fncToken.namePos = nPos;
                    nOpen = nPos + tag.nameLen;
                    break;          // function name found
                }
            }
            if (fncToken.code != FncCode::undefined) break;     // function name found
        }

        // check if current char is a name delimiter
        char chNow = i_expr[nPos];
        isLeftDelim = 
            isspace<char>(chNow, locale::classic()) || 
            std::any_of(
                leftDelimArr, 
                leftDelimArr + leftDelimSize, 
                [chNow](const char i_delim) -> bool { return chNow == i_delim; }
            );
    }

    // if no function name in source string then return not found result
    if (fncToken.code == FncCode::undefined) return fncToken;

    // function name must followed by optional whitespaces and ( open bracket
    bool isOpen = false;
    for (size_t nPos = nOpen; nPos < i_expr.length(); nPos++) {

        if (isspace<char>(i_expr[nPos], locale::classic())) continue;   // skip optional whitespaces

        isOpen = i_expr[nPos] == '(';
        fncToken.openPos = nPos;
        break;
    }
    if (!isOpen) throw DbException(LT("missing open bracket after function at: %d in: %s"), fncToken.namePos, i_expr.c_str());

    // find function body ) close bracket, it must be on the same bracket level
    int depth = 0;
    bool isClose =  false;
    for (size_t nPos = fncToken.openPos + 1; !isClose && nPos < i_expr.length(); nPos++) {

        // skip until end of "quotes" or 'apostrophes'
        size_t nSkip = skipIfQuoted(nPos, i_expr);
        if (nSkip != nPos) {
            if (nSkip >= i_expr.length()) break;    // skipped until end of string

            nPos = nSkip;
            continue;           // move to the next char after "quotes" or 'apostrophes'
        }

        char chNow = i_expr[nPos];

        // if this is ( open bracket then increase nesting depth
        if (chNow == '(') {
            depth++;
            continue;
        }

        // if this is ) close bracket then check nesting depth
        if (chNow == ')') {
            isClose = depth-- <= 0;     // close bracket must on the same level
            if (isClose) {
                fncToken.closePos = nPos;   // found function body close bracket
                break;
            }
        }
    }
    if (!isClose) throw DbException(LT("missing close bracket after function at: %d in: %s"), fncToken.namePos, i_expr.c_str());

    return fncToken;
}

// return true if this is first used accumulator
bool ModelAggregationSql::isFirstUsedAcc(int i_accPos, const vector<bool> & i_accUsage) const
{
    for (size_t nPos = 0; nPos < i_accUsage.size(); nPos++) {
        if ((int)nPos == i_accPos) return true;
        if (i_accUsage[nPos]) break;
    }
    return false;
}

// make accumulator table alias or return fromAlias for the first used accumulator
const string ModelAggregationSql::makeAccTableAlias(
    int i_accPos, const vector<bool> & i_accUsage, int i_level, const string i_firstAlias
) const
{
    return isFirstUsedAcc(i_accPos, i_accUsage) ? i_firstAlias : "L" + to_string(i_level) + "A" + to_string(i_accPos);
}

// if quote opens at current position then skip until the end of "quotes" or 'apostrophes'
size_t skipIfQuoted(size_t i_pos, const string & i_str)
{
    // check if current position is open of "quotes" or 'apostrophes'
    if (i_pos >= i_str.length()) return i_pos;

    char chQuote = i_str[i_pos];
    if (chQuote != '"' && chQuote != '\'') return i_pos;

    // skip until end of "quotes" or 'apostrophes'
    for (size_t nPos = i_pos + 1; nPos < i_str.length(); nPos++) {
        if (i_str[nPos] == chQuote) return nPos;
    }
    throw DbException(LT("unbalanced \"quotes\" or 'apostrophes' in: %s"), i_str.c_str());
}

/** translate output table "native" (non-derived) accumulator into sql subquery. */
const string ModelAccumulatorSql::translateNativeAccExpr(int i_accId, bool i_isFirstAcc) const
{
    // if this is first accumulator then return value field: no subquery required
    if (i_isFirstAcc) return "A.acc_value";

    // SELECT A1.acc_value FROM salarySex_a_2012820 A1
    // WHERE A1.run_id = A.run_id 
    // AND A1.sub_id = A.sub_id 
    // AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1
    // AND A1.acc_id = 1
    //
    string alias = "A" + to_string(i_accId);    // table alsias
    string sql = 
        "SELECT " + alias + ".acc_value FROM " + accTableName + " " + alias +
        " WHERE " + alias + ".run_id = A.run_id" +
        " AND " + alias + ".sub_id = A.sub_id";

    for (const string & name : dimNameVec) {
        sql += " AND " + alias + "." + name + " = A." + name;
    }

    sql += " AND " + alias + ".acc_id = " + to_string(i_accId);

    return sql;
}

/** translate output table derived accumulator into sql subquery. */
const string ModelAccumulatorSql::translateDerivedAccExpr(
    const string & i_accName, const string & i_expr, const map<string, string> & i_nativeMap
) const
{
    // find native accumulators in source expression and substitute with sql subquery
    string sql;
    bool isLeftDelim = true;

    for (size_t nPos = 0; nPos < i_expr.length(); nPos++) {

        // skip until end of "quotes" or 'apostrophes'
        size_t nSkip = skipIfQuoted(nPos, i_expr);
        if (nSkip != nPos) {
            if (nSkip < i_expr.length()) {      // append to output
                sql += i_expr.substr(nPos, (nSkip + 1) - nPos);
                nPos = nSkip;
                isLeftDelim = false;    // "quotes" or 'apostrophes' is not left delimiter
                continue;               // move to the next char after "quotes" or 'apostrophes'
            }
            else { 
                sql += i_expr.substr(nPos);
                break;      // skipped until end of string
            }
        }

        // if previous char was name left delimiter then check for accumulator name in current position
        if (isLeftDelim) {

            // check for each accumulator name
            bool isAcc = false;
            size_t nLen = 0;
            int accPos;
            for (accPos = 0; accPos < (int)accNameVec.size(); accPos++) {
                
                nLen = accNameVec[accPos].length();
                isAcc = equalNoCase(accNameVec[accPos].c_str(), i_expr.c_str() + nPos, nLen);

                // check if accumulator name end with right delimiter
                if (isAcc && nPos + nLen < i_expr.length()) {

                    char chEnd = i_expr[nPos + nLen];
                    isAcc =
                        isspace<char>(chEnd, locale::classic()) ||
                        std::any_of(
                        rightDelimArr,
                        rightDelimArr + rightDelimSize,
                        [chEnd](const char i_delim) -> bool { return chEnd == i_delim; }
                    );
                }

                if (isAcc) break;   // accumulator found
            }

            // accumulator found: replace with subquery
            if (isAcc) {

                // validate: it must be native accumulator
                const map<string, string>::const_iterator it = i_nativeMap.find(accNameVec[accPos]);
                if (it == i_nativeMap.cend()) 
                    throw DbException(LT("error in derived accumulator: %s invalid name: %s in: %s"), i_accName.c_str(), accNameVec[accPos].c_str(), i_expr.c_str());

                sql += "(" + it->second +")";   // append subquery

                nPos += nLen - 1;   // skip accumulator in source expression
                isLeftDelim = false;
                continue;           // done with accumulator
            }
        }

        // append current char to output
        char chNow = i_expr[nPos];
        sql += chNow;

        // check if current char is a name delimiter
        isLeftDelim = 
            isspace<char>(chNow, locale::classic()) || 
            std::any_of(
                leftDelimArr, 
                leftDelimArr + leftDelimSize, 
                [chNow](const char i_delim) -> bool { return chNow == i_delim; }
            );
    }

    return sql;
}

