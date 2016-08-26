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
#   $expr_id  - output table expression id
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
  dbTableName <- tableRow$db_expr_table
  
  # find output expression id by name, if specified
  exprId <- NA
  if (!missing(exprName) && !is.na(exprName)) {
    if (!is.character(exprName) || length(exprName) <= 0) stop("invalid or empty output expression name")
    
    uRow <- defRs$tableExpr[which(defRs$tableExpr$table_hid == tableRow$table_hid & defRs$tableExpr$expr_name == exprName), ]
    
    if (nrow(uRow) != 1) {
      stop("output table ", tableName, " does not contain expression: ", exprName)
    }
    exprId <- uRow$expr_id
  }
  
  # check if run id is belong the model and completed
  rlRow <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT run_id FROM run_lst",
      " WHERE run_id = ", runId,
      " AND model_id = ", defRs$modelDic$model_id,
      " AND status = 's'",
      sep=""
    )
  )
  if (nrow(rlRow) != 1L) {
    stop("model run results not found (or not completed), run id: ", runId)
  }
  
  # SELECT dim0, dim1, expr_id, expr_value 
  # FROM salarySex_v2012_820
  # WHERE run_id = 
  # (
  #   SELECT base_run_id FROM run_table WHERE run_id = 2 AND table_hid = 12345
  # )
  # AND expr_id = 3
  # ORDER BY 1, 2, 3
  #
  sqlSel <-
    paste(
      "SELECT ", 
      ifelse(nRank > 0L,
        paste(
          paste(
            defRs$tableDims[which(defRs$tableDims$table_hid == tableRow$table_hid), "dim_name"],
            sep = "", collapse = ", "
          ),
          ", ", sep = ""
        ),
        ""
      ),
      " expr_id, expr_value",
      " FROM ", dbTableName, 
      " WHERE run_id = ", 
      " (", 
      " SELECT base_run_id FROM run_table WHERE run_id = ", runId, " AND table_hid = ", tableRow$table_hid, 
      " )",
      ifelse(!is.na(exprId), 
        paste(" AND expr_id = ", exprId, sep = ""), 
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
