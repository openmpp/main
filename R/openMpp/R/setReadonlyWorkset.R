##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

# 
# Set or clear read-only status of parameters working set by workset id
#   return set id of updated workset or <= 0 on error
#
# dbCon      - database connection
# defRs      - model definition database rows
# isReadonly - if TRUE then set workset as read-only
# worksetId  - id of parameters working set
#
setReadonlyWorkset <- function(dbCon, defRs, isReadonly, worksetId)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  if (missing(worksetId)) stop("invalid (missing) workset id")
  if (is.null(worksetId) || is.na(worksetId) || !is.integer(worksetId)) stop("invalid or empty workset id")
  if (worksetId <= 0L) stop("workset id must be positive: ", worksetId)

  if (missing(isReadonly)) stop("invalid (missing) read-only flag")
  if (is.null(isReadonly) || is.na(isReadonly) || !is.logical(isReadonly)) stop("invalid or empty read-only flag")

  # execute in transaction scope
  isTrxCompleted <- FALSE;
  tryCatch({
    dbBegin(dbCon)
    
    # set readonly flag by workset id
    rdOnlyVal <- ifelse(isReadonly, 1L, 0L)
    setRs <- lockWorksetUsingReadonly(dbCon, defRs, TRUE, worksetId, FALSE, rdOnlyVal)
    if (is.null(setRs) || setRs$is_readonly != rdOnlyVal) {
      stop("workset not found or has invalid read-only status, set id: ", worksetId)
    }
    
    isTrxCompleted <- TRUE; # completed OK
  },
  finally = {
    ifelse(isTrxCompleted, dbCommit(dbCon), dbRollback(dbCon))
  })
  return(ifelse(isTrxCompleted, worksetId, 0L))
}

# 
# Set or clear read-only status for default working set of model parameters
#   return set id of updated workset or <= 0 on error
#   default workset is a first workset for the model
#
# dbCon      - database connection
# defRs      - model definition database rows
# isReadonly - if TRUE then set workset as read-only
#
setReadonlyDefaultWorkset <- function(dbCon, defRs, isReadonly)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  if (missing(isReadonly)) stop("invalid (missing) read-only flag")
  if (is.null(isReadonly) || is.na(isReadonly) || !is.logical(isReadonly)) stop("invalid or empty read-only flag")
  
  # execute in transaction scope
  setId <- 0L
  isTrxCompleted <- FALSE;
  tryCatch({
    dbBegin(dbCon)
    
    # set readonly flag by model name and digest
    rdOnlyVal <- ifelse(isReadonly, 1L, 0L)
    setRs <- lockWorksetUsingReadonly(dbCon, defRs, FALSE, 0, FALSE, rdOnlyVal)
    if (is.null(setRs) || setRs$is_readonly != rdOnlyVal) {
      stop("workset not found or has invalid read-only status, model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)
    }
    
    setId <- setRs$set_id
    isTrxCompleted <- TRUE; # completed OK
  },
  finally = {
    ifelse(isTrxCompleted, dbCommit(dbCon), dbRollback(dbCon))
  })
  return(ifelse(isTrxCompleted, setId, 0L))
}

# 
# For internal use only, it MUST be in transaction scope
#
# Set read-only flag for workset by id or model id
# Return data frame of workset_lst rows: $set_id, $model_id, $is_readonly
#
# dbCon           - database connection
# i_defRs         - model definition database rows
# i_isSetId       - if true then use set id else model id
# i_setId         - id of parameters working set
# i_isAddValue    - if TRUE then add value to is_readonly else set
# i_readonlyValue - integer to set is_readonly
#
#   if i_isAddValue is true then is_readonly = i_isAddValue
#   else is_readonly = is_readonly + i_isAddValue
#
lockWorksetUsingReadonly <- function(dbCon, i_defRs, i_isSetId, i_setId, i_isAddValue, i_readonlyValue)
{
  # set output value
  setRs <- NULL
  
  # expression to update is_readonly
  rdOnlyExpr <- ifelse(i_isAddValue, paste("is_readonly + ", i_readonlyValue, sep = ""), as.character(i_readonlyValue))
  
  if (i_isSetId) {    # use workset id
  
    dbGetQuery(
      dbCon, 
      paste(
        "UPDATE workset_lst SET is_readonly = ", rdOnlyExpr,
        " WHERE set_id = ", i_setId, 
        " AND model_id = ", i_defRs$modelDic$model_id, 
        sep=""
      )
    )
    setRs <- dbGetQuery(
      dbCon, 
      paste(
        "SELECT set_id, model_id, is_readonly FROM workset_lst WHERE set_id = ", i_setId, 
        sep=""
      )
    )
    # one row expected else set id is invalid
    if (is.null(setRs) || nrow(setRs) != 1 || setRs$model_id != i_defRs$modelDic$model_id) {
      stop("workset not found: ", i_setId, " or not belong to model: ", i_defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)
    }
  }
  else {    # use model id
  
    dbGetQuery(
      dbCon, 
      paste(
        "UPDATE workset_lst SET is_readonly = ", rdOnlyExpr,
        " WHERE set_id = ",
        " (",
        " SELECT MIN(M.set_id) FROM workset_lst M WHERE M.model_id = ", i_defRs$modelDic$model_id,
        " )",
        sep=""
      )
    )
    setRs <- dbGetQuery(
      dbCon, 
      paste(
        "SELECT WS.set_id, WS.model_id, WS.is_readonly FROM workset_lst WS WHERE WS.set_id = ",
        " (",
        " SELECT MIN(M.set_id) FROM workset_lst M WHERE M.model_id = ", i_defRs$modelDic$model_id,
        " )",
        sep=""
      )
    )
    # one row expected else model id is invalid
    if (is.null(setRs) || nrow(setRs) != 1) {
      stop("no worksets not found for model: ", i_defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)
    }
  }
  
  return(setRs)
}
