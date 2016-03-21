##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

#
# Select modeling task run: status and results
#
# dbCon     - database connection
# taskRunId - id of modeling task
#
# return list of database rows for supplied modeling task id:
#   $taskRunLst - task_run_lst row: task run id, status and time
#   $taskRunSet - task_run_set rows: input workset id's and result run id's
#
selectTaskRun <- function(dbCon, taskRunId) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(taskRunId) || is.null(taskRunId) || !is.numeric(taskRunId)) stop("invalid or empty task run id")
  
  # get task_run_lst row: single row expected
  sql <- paste(
      "SELECT task_run_id, task_id, sub_count, create_dt, status, update_dt FROM task_run_lst",
      " WHERE task_run_id = ", taskRunId,
      " ORDER BY 1",
      sep=""
    )
  taskRunRs <- list(taskRunLst = dbGetQuery(dbCon, sql))
  
  if (nrow(taskRunRs$taskRunLst) != 1) stop("task run not found, id: ", taskRunId)
  
  # task_run_set: task input and output
  taskRunRs[["taskRunSet"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT task_run_id, run_id, set_id, task_id FROM task_run_set WHERE task_run_id = ", taskRunId, " ORDER BY 1, 2",
      sep = ""
    )
  )
  # task_run_set may not exist

  return(taskRunRs)
}

#
# Select list of modeling task runs
#
# dbCon  - database connection
# taskId - id of modeling task
#
# return list of database rows for that task id:
#   $taskLst    - task_lst row: task id and name
#   $taskRunLst - task_run_lst rows: task run id, status and time
#
selectTaskRunList <- function(dbCon, taskId) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(taskId) || is.null(taskId) || !is.numeric(taskId)) stop("invalid or empty modeling task id")
  
  # get task_lst row: single row expected
  sql <- paste(
      "SELECT task_id, model_id, task_name FROM task_lst",
      " WHERE task_id = ", taskId,
      " ORDER BY 1",
      sep=""
    )
  taskRunLstRs <- list(taskLst = dbGetQuery(dbCon, sql))
  
  if (nrow(taskRunLstRs$taskLst) != 1) stop("modeling task not found, id: ", taskId)
  
  # task_run_lst rows:
  taskRunLstRs[["taskRunLst"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT task_run_id, task_id, sub_count, create_dt, status, update_dt FROM task_run_lst",
      " WHERE task_id = ", taskId,
      " ORDER BY 1",
      sep = ""
    )
  )
  
  return(taskRunLstRs)
}
