##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

# 
# Select output table expression values from model run result
#
# dbCon     - database connection
# defRs     - model definition database rows
# runId     - id of model run results
# tableName - output table name, i.e.: "sexAge"
# exprName  - (optional) output table expression name, i.e.: "Expr0"
#             if missing or NA then return all expressions
#
# return data frame of output table rows:
#   $dim0,... - dimension item enum ids (not returned if rank is zero)
#   $unit_id  - output table expression id
#   $value    - output table value
#
selectRunOutputValue <- function(dbCon, defRs, runId, tableName, exprName = NA)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")
  
  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  if (missing(runId)) stop("invalid (missing) run id")
  if (is.null(runId) || is.na(runId) || !is.integer(runId)) stop("invalid or empty run id")
  if (runId <= 0L) stop("run id must be positive: ", runId)

  if (missing(tableName)) stop("invalid (missing) output table name")
  if (is.null(tableName)) stop("invalid or empty output table name")
  if (is.na(tableName) || !is.character(tableName) || length(tableName) <= 0) {
    stop("invalid or empty output table name")
  }
  
  # get table row by name
  tableRow <- defRs$tableDic[which(defRs$tableDic$table_name == tableName), ]
  if (nrow(tableRow) != 1) {
    stop("output table not found in model output tables list: ", tableName)
  }
  
  nRank <- tableRow$table_rank
  dbTableName <- paste(
    defRs$modelDic$model_prefix, defRs$modelDic$value_prefix, tableRow$db_name_suffix, 
    sep = ""
  )
  
  # find output expression id by name, if specified
  unitId <- NA
  if (!missing(exprName) && !is.na(exprName)) {
    if (!is.character(exprName) || length(exprName) <= 0) stop("invalid or empty output expression name")
    
    uRow <- defRs$tableUnit[which(defRs$tableUnit$table_id == tableRow$table_id & defRs$tableUnit$unit_name == exprName), ]
    
    if (nrow(uRow) != 1) {
      stop("output table ", tableName, " does not contain expression: ", exprName)
    }
    unitId <- uRow$unit_id
  }
  
  # check if run id is belong the model and completed
  rlRow <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT run_id FROM run_lst",
      " WHERE run_id = ", runId,
      " AND model_id = ", defRs$modelDic$model_id,
      " AND sub_completed = sub_count",
      sep=""
    )
  )
  if (nrow(rlRow) != 1L) {
    stop("model run results not found (or not completed), run id: ", runId)
  }
  
  # SELECT dim0, dim1, unit_id, value 
  # FROM modelone_201208171604590148_v0_salarySex
  # WHERE run_id = 2 AND unit_id = 3
  # ORDER BY 1, 2, 3
  #
  sqlSel <-
    paste(
      "SELECT ", 
      ifelse(nRank > 0L,
        paste(
          paste("dim", 0L:(nRank - 1L), sep = "", collapse = ", "), ", ",
          sep = ""
        ),
        ""
      ),
      " unit_id, value",
      " FROM ", dbTableName, 
      " WHERE run_id = ", runId,
      ifelse(!is.na(unitId), 
        paste(" AND unit_id = ", unitId, sep = ""), 
        ""
      ),
      " ORDER BY 1",
      ifelse(nRank > 0L,
        paste(", ",
          paste(2L:(nRank + 1L), sep = "", collapse = ", "),
          sep = ""
        ),
        ""
      ),
      sep = ""
    )

  selRs <- dbGetQuery(dbCon, sqlSel)
  return(selRs)
}

# 
# Select output table accumulator values from model run result
#
# dbCon     - database connection
# defRs     - model definition database rows
# runId     - id of model run results
# tableName - output table name, i.e.: "sexAge"
# accName   - (optional) output table accumulator name, i.e.: "acc0"
#             if missing or NA then return all accumulators
#
# return data frame of output table rows:
#   $dim0,... - dimension item enum ids (not returned if rank is zero)
#   $acc_id   - output table accumulator id
#   $sub_id   - subsample number
#   $acc0,... - output table accumulator value(s)
#
selectRunAccumulator <- function(dbCon, defRs, runId, tableName, accName = NA)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")
  
  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  if (missing(runId)) stop("invalid (missing) run id")
  if (is.null(runId) || is.na(runId) || !is.integer(runId)) stop("invalid or empty run id")
  if (runId <= 0L) stop("run id must be positive: ", runId)
  
  if (missing(tableName)) stop("invalid (missing) output table name")
  if (is.null(tableName)) stop("invalid or empty output table name")
  if (is.na(tableName) || !is.character(tableName) || length(tableName) <= 0) {
    stop("invalid or empty output table name")
  }
  
  # get table row by name
  tableRow <- defRs$tableDic[which(defRs$tableDic$table_name == tableName), ]
  if (nrow(tableRow) != 1) {
    stop("output table not found in model output tables list: ", tableName)
  }
  
  nRank <- tableRow$table_rank
  dbTableName <- paste(
    defRs$modelDic$model_prefix, defRs$modelDic$acc_prefix, tableRow$db_name_suffix, 
    sep = ""
  )
  
  # find output accumulator id by name, if specified
  accId <- NA
  if (!missing(accName) && !is.na(accName)) {
    if (!is.character(accName) || length(accName) <= 0) {
      stop("invalid or empty output accumulator name")
    }

    accRow <- defRs$tableAcc[which(defRs$tableAcc$table_id == tableRow$table_id & defRs$tableAcc$acc_name == accName), ]
    
    if (nrow(accRow) != 1) {
      stop("output table ", tableName, " does not contain accumulator: ", accName)
    }
    accId <- accRow$acc_id
  }
  
  # check if run id is belong the model and completed
  # check if that run has required number of subsamples 
  rlRow <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT run_id, sub_count FROM run_lst",
      " WHERE run_id = ", runId,
      " AND model_id = ", defRs$modelDic$model_id,
      " AND sub_completed = sub_count",
      sep=""
    )
  )
  if (nrow(rlRow) != 1L) {
    stop("model run results not found (or not completed), run id: ", runId)
  }
  
  # SELECT dim0, dim1, acc_id, sub_id, acc_value 
  # FROM modelone_201208171604590148_f0_salarySex
  # WHERE run_id = 2
  # AND acc_id = 4
  # ORDER BY 1, 2, 3, 4
  #
  sqlSel <-
    paste(
      "SELECT ", 
      ifelse(nRank > 0L,
        paste(
          paste("dim", 0L:(nRank - 1L), sep = "", collapse = ", "), ", ",
          sep = ""
        ),
        ""
      ),
      " acc_id, sub_id, acc_value FROM ", dbTableName, 
      " WHERE run_id = ", runId,
      ifelse(!is.na(accId), 
        paste(" AND acc_id = ", accId, sep = ""), 
        ""
      ),
      " ORDER BY 1, 2",
      ifelse(nRank > 0L,
        paste(", ",
          paste(2L:(nRank + 1L), sep = "", collapse = ", "),
          sep = ""
        ),
        ""
      ),
      sep = ""
    )

  selRs <- dbGetQuery(dbCon, sqlSel)
  return(selRs)
}

# 
# Select parameter values from model run result
#
# dbCon     - database connection
# defRs     - model definition database rows
# runId     - id of model run results
# paramName - parameter name, i.e.: "sexAge"
#
# return data frame of parameter table rows:
#   $dim0,... - dimension item enum ids (not returned if rank is zero)
#   $value    - parameter value
#
selectRunParameter <- function(dbCon, defRs, runId, paramName)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")
  
  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  if (missing(runId)) stop("invalid (missing) run id")
  if (is.null(runId) || is.na(runId) || !is.integer(runId)) stop("invalid or empty run id")
  if (runId <= 0L) stop("run id must be positive: ", runId)

  if (missing(paramName)) stop("invalid (missing) parameter name")
  if (is.null(paramName)) stop("invalid or empty parameter name")
  if (is.na(paramName) || !is.character(paramName) || length(paramName) <= 0) {
    stop("invalid or empty parameter name")
  }
  
  # get parameter row by name
  paramRow <- defRs$paramDic[which(defRs$paramDic$parameter_name == paramName), ]
  if (nrow(paramRow) != 1) {
    stop("parameter not found in model parameters list: ", paramName)
  }
  
  nRank <- paramRow$parameter_rank
  dbTableName <- paste(
    defRs$modelDic$model_prefix, defRs$modelDic$parameter_prefix, paramRow$db_name_suffix, 
    sep = ""
  )
  
  # check if run id is belong the model and completed
  rlRow <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT run_id FROM run_lst",
      " WHERE run_id = ", runId,
      " AND model_id = ", defRs$modelDic$model_id,
      " AND sub_completed = sub_count",
      sep=""
    )
  )
  if (nrow(rlRow) != 1L) {
    stop("model run results not found (or not completed), run id: ", runId)
  }
  
  # SELECT dim0, dim1, value 
  # FROM modelone_201208171604590148_p0_ageSex
  # WHERE run_id = 2
  # ORDER BY 1, 2, 3
  #
  sqlSel <-
    paste(
      "SELECT ", 
      ifelse(nRank > 0L,
        paste(
          paste("dim", 0L:(nRank - 1L), sep = "", collapse = ", "), ", ",
          sep = ""
        ),
        ""
      ),
      " value",
      " FROM ", dbTableName, 
      " WHERE run_id = ", runId,
      ifelse(nRank > 0L,
        paste(" ORDER BY ",
          paste(1L:nRank, sep = "", collapse = ", "),
          sep = ""
        ),
        ""
      ),
      sep = ""
    )

  selRs <- dbGetQuery(dbCon, sqlSel)
  return(selRs)
}
