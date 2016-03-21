##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

#
# Return language rows from lang_lst table
# dbCon - database connection
#
getLanguages <- function(dbCon) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  # get lang_lst rows: at least one row expected
  langRs <- dbGetQuery(
    dbCon, 
    "SELECT lang_id, lang_code, lang_name FROM lang_lst ORDER BY 1"
  )
  if (nrow(langRs) <= 0) stop("invalid database: no language(s) found")

  return(langRs)
}

#
# Return working set id by name
#   if model have no working set with that name then return is negative
#   if model has multiple working sets with that name then return min(set id)
#
# dbCon       - database connection
# defRs       - model definition database rows
# worksetName - name of parameters working set
#
# return: id of parameters working set
#         positive integer on success, negative on error
#
getWorksetIdByName <- function(dbCon, defRs, worksetName) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  if (missing(worksetName)) stop("invalid (missing) workset name")
  if (is.null(worksetName) || is.na(worksetName) || !is.character(worksetName)) stop("invalid or empty workset name")
  
  # get first set id with specified name for that model id
  setRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT MIN(set_id) FROM workset_lst",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " AND set_name = ", toQuoted(worksetName),
      sep=""
    )
  )
  if (nrow(setRs) <= 0) return(-1L)

  return(ifelse(!is.na(setRs[1,1]), as.integer(setRs[1,1]), -1L))
}

#
# Return default working set id for the model
#   default workset is a first workset for the model
#   each model must have default workset
#
# dbCon - database connection
# defRs - model definition database rows
#
# return: id of parameters working set
#         positive integer on success, stop if not found
#
getDefaultWorksetId <- function(dbCon, defRs) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  # get first set id for that model id
  setRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT MIN(set_id) FROM workset_lst WHERE model_id = ", defRs$modelDic$model_id,
      sep=""
    )
  )
  # one row expected else model id is invalid
  if (is.null(setRs) || nrow(setRs) != 1) {
    stop("no worksets not found for model: ", i_defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)
  }
  
  return(ifelse(!is.na(setRs[1,1]), as.integer(setRs[1,1]), -1L))
}

#
# Return ids of model run results (run_id) 
# where input parameters are from specified working set
#
# dbCon     - database connection
# worksetId - id of parameters working set
#
# return: data frame with integer $run_id column
#
getWorksetRunIds <- function(dbCon, worksetId) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(worksetId)) stop("invalid (missing) workset id")
  if (is.null(worksetId) || is.na(worksetId) || !is.integer(worksetId)) stop("invalid or empty workset id")
  if (worksetId <= 0L) stop("workset id must be positive: ", worksetId)
  
  # get run ids by predefined option key: OpenM.SetId
  runRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT RL.run_id",
      " FROM run_lst RL",
      " INNER JOIN run_option RO ON (RO.run_id = RL.run_id)",
      " WHERE RL.sub_completed = RL.sub_count",
      " AND RO.option_key = 'OpenM.SetId'",
      " AND RO.option_value = ", toQuoted(worksetId),
      " ORDER BY 1",
      sep=""
    )
  )
  # it can be empty result with nrow() = 0

  return(runRs)
}

#
# Return id of first run results for the model
#
# dbCon - database connection
# defRs - model definition database rows
#
# return: id of first model run results
#         positive integer on success, negative if no run results found
#
getFirstRunId <- function(dbCon, defRs) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  # get first run id for that model id
  runRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT MIN(run_id) FROM run_lst WHERE model_id = ", defRs$modelDic$model_id,
      sep=""
    )
  )
  # one row expected else model id is invalid
  if (is.null(runRs) || nrow(runRs) != 1) return -1L
  
  return(ifelse(!is.na(runRs[1,1]), as.integer(runRs[1,1]), -1L))
}

#
# Return id of last run results for the model
#
# dbCon - database connection
# defRs - model definition database rows
#
# return: id of last model run results
#         positive integer on success, negative if no run results found
#
getLastRunId <- function(dbCon, defRs) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  # get first run id for that model id
  runRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT MAX(run_id) FROM run_lst WHERE model_id = ", defRs$modelDic$model_id,
      sep=""
    )
  )
  # one row expected else model id is invalid
  if (is.null(runRs) || nrow(runRs) != 1) return -1L
  
  return(ifelse(!is.na(runRs[1,1]), as.integer(runRs[1,1]), -1L))
}

#
# Return modeling task id by name
#   if model have no task with that name then return is negative
#   if model has multiple tasks with that name then return min(task id)
#
# dbCon    - database connection
# defRs    - (optional) model definition database rows
# taskName - name of modeling task
#
# return: id of modeling task
#         positive integer on success, negative on error
#
getTaskIdByName <- function(dbCon, defRs = NA, taskName) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (!missing(defRs) && !is.null(defRs) && !is.na(defRs)) {
    if (!is.list(defRs)) stop("invalid or empty model definition")
  }

  if (missing(taskName)) stop("invalid (missing) task name")
  if (is.null(taskName) || is.na(taskName) || !is.character(taskName)) stop("invalid or empty task name")
  
  # get first task id with specified name for that model id
  sql <- ifelse(
    !missing(defRs) && !is.null(defRs) && !is.na(defRs),
    paste(
      "SELECT MIN(task_id) FROM task_lst",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " AND task_name = ", toQuoted(taskName),
      sep=""
    ),
    paste(
      "SELECT MIN(task_id) FROM task_lst WHERE task_name = ", toQuoted(taskName),
      sep=""
    )
  )
  taskRs <- dbGetQuery(dbCon, sql)
  
  if (nrow(taskRs) <= 0) return(-1L)

  return(ifelse(!is.na(taskRs[1,1]), as.integer(taskRs[1,1]), -1L))
}

#
# Return id of first task run
#
# dbCon  - database connection
# taskId - modeling task id
#
# return: id of first task run
#         positive integer on success, negative if no run results found
#
getTaskFirstRunId <- function(dbCon, taskId) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(taskId)) stop("invalid (missing) modeling task id")
  if (is.null(taskId) || is.na(taskId) || !is.numeric(taskId)) stop("invalid or empty modeling task id")
  
  # get first task run id for that modeling task
  runRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT MIN(task_run_id ) FROM task_run_lst WHERE task_id = ", taskId,
      sep=""
    )
  )
  # one row expected else model id is invalid
  if (is.null(runRs) || nrow(runRs) != 1) return -1L
  
  return(ifelse(!is.na(runRs[1,1]), as.integer(runRs[1,1]), -1L))
}

#
# Return id of last task run
#
# dbCon  - database connection
# taskId - modeling task id
#
# return: id of last task run
#         positive integer on success, negative if no run results found
#
getTaskLastRunId <- function(dbCon, taskId) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(taskId)) stop("invalid (missing) modeling task id")
  if (is.null(taskId) || is.na(taskId) || !is.numeric(taskId)) stop("invalid or empty modeling task id")
  
  # get last task run id for that modeling task
  runRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT MAX(task_run_id ) FROM task_run_lst WHERE task_id = ", taskId,
      sep=""
    )
  )
  # one row expected else model id is invalid
  if (is.null(runRs) || nrow(runRs) != 1) return -1L
  
  return(ifelse(!is.na(runRs[1,1]), as.integer(runRs[1,1]), -1L))
}
