##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

#
# Select modeling task text (name, description, notes) and input workset id's
#
# dbCon  - database connection
# taskId - id of modeling task
#
# return list of database rows for supplied modeling task id:
#   $taskLst - task_lst row: task id and name
#   $taskTxt - task_txt rows: language, description, notes
#   $taskSet - task_set rows: task input workset id's
#
selectTask <- function(dbCon, taskId) 
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
  taskRs <- list(taskLst = dbGetQuery(dbCon, sql))
  
  if (nrow(taskRs$taskLst) != 1) stop("modeling task not found, id: ", taskId)
  
  # task text: language, description, notes
  taskRs[["taskTxt"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT T.task_id, T.lang_id, L.lang_code, T.descr, T.note",
      " FROM task_txt T INNER JOIN lang_lst L ON (L.lang_id = T.lang_id)",
      " WHERE T.task_id = ", taskId,
      " ORDER BY 1, 2",
      sep = ""
    )
  )
  # task_txt may not exist
  
  # task sets: workset id's
  taskRs[["taskSet"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT task_id, set_id FROM task_set WHERE task_id = ", taskId, " ORDER BY 1, 2",
      sep = ""
    )
  )
  # task_set may not exist

  return(taskRs)
}

#
# Select list of modeling tasks for the model
#
# dbCon - database connection
# defRs - model definition database rows
#
# return list of database rows for that model:
#   $taskLst - task_lst rows: task id and name
#   $taskTxt - task_txt rows: language, description, notes
#
selectTaskList <- function(dbCon, defRs) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  # get task_lst rows
  sql <- paste(
      "SELECT task_id, model_id, task_name FROM task_lst",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1",
      sep=""
    )
  taskLstRs <- list(taskLst = dbGetQuery(dbCon, sql))
  
  # task text: language, description, notes
  taskLstRs[["taskTxt"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT T.task_id, T.lang_id, L.lang_code, T.descr, T.note",
      " FROM task_txt T",
      " INNER JOIN task_lst M ON (M.task_id = T.task_id)",
      " INNER JOIN lang_lst L ON (L.lang_id = T.lang_id)",
      " WHERE M.model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1, 2",
      sep = ""
    )
  )
  
  return(taskLstRs)
}
