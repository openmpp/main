##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

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
#   $acc_id   - output table accumulator id
#   $sub_id   - sub-value number
#   $dim0,... - dimension item enum ids (not returned if rank is zero)
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
  dbTableName <- tableRow$db_acc_table
  
  # find output accumulator id by name, if specified
  accId <- NA
  if (!missing(accName) && !is.na(accName)) {
    if (!is.character(accName) || length(accName) <= 0) {
      stop("invalid or empty output accumulator name")
    }

    accRow <- defRs$tableAcc[which(defRs$tableAcc$table_hid == tableRow$table_hid & defRs$tableAcc$acc_name == accName), ]
    
    if (nrow(accRow) != 1) {
      stop("output table ", tableName, " does not contain accumulator: ", accName)
    }
    accId <- accRow$acc_id
  }
  
  # check if run id is belong the model and completed
  # check if that run has required number of sub-values 
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
  
  # SELECT acc_id, sub_id, dim0, dim1, acc_value 
  # FROM salarySex_a2012_820
  # WHERE run_id = 
  # (
  #   SELECT base_run_id FROM run_table WHERE run_id = 2 AND table_hid = 12345
  # )
  # AND acc_id = 4
  # ORDER BY 1, 2, 3, 4
  #
  sqlSel <-
    paste(
      "SELECT acc_id, sub_id, ", 
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
      " acc_value FROM ", dbTableName, 
      " WHERE run_id = ", 
      " (", 
      " SELECT base_run_id FROM run_table WHERE run_id = ", runId, " AND table_hid = ", tableRow$table_hid, 
      " )",
      ifelse(!is.na(accId), 
        paste(" AND acc_id = ", accId, sep = ""), 
        ""
      ),
      " ORDER BY 1, 2",
      ifelse(nRank > 0L,
        paste(", ",
          paste(3L:(nRank + 2L), sep = "", collapse = ", "),
          sep = ""
        ),
        ""
      ),
      sep = ""
    )
    
  selRs <- dbGetQuery(dbCon, sqlSel)
  return(selRs)
}
