##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

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
  dbTableName <- paramRow$db_run_table
  
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
  
  # SELECT dim0, dim1, param_value 
  # FROM ageSex_p2012_817
  # WHERE run_id = (SELECT base_run_id FROM run_parameter WHERE run_id = 1234 AND parameter_hid = 1)
  # ORDER BY 1, 2, 3
  sqlSel <-
    paste(
      "SELECT ", 
      ifelse(nRank > 0L,
        paste(
          paste(
            defRs$paramDims[which(defRs$paramDims$parameter_hid == paramRow$parameter_hid), "dim_name"],
            sep = "", collapse = ", "
          ), 
          ", ", sep = ""
        ),
        ""
      ),
      " param_value",
      " FROM ", dbTableName, 
      " WHERE run_id = ", 
      " (SELECT base_run_id FROM run_parameter WHERE run_id = ", runId, " AND parameter_hid = ", paramRow$parameter_hid, ")",
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
