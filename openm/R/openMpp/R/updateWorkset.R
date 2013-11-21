##
## Copyright (c) 2013 OpenM++
## This is a free software licensed under MIT license
##

# 
# Set or clear read-only status of parameters working set by workset id
#   return set id of updated workset or <= 0 on error
#
# dbCon - database connection
# isReadonly - if TRUE then set workset as read-only
# worksetId - id of parameters working set
#
setReadonlyWorkset <- function(dbCon, isReadonly, worksetId)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(worksetId)) stop("invalid (missing) workset id")
  if (is.null(worksetId) || is.na(worksetId) || !is.integer(worksetId)) stop("invalid or empty workset id")
  if (worksetId <= 0L) stop("workset id must be positive: ", worksetId)

  if (missing(isReadonly)) stop("invalid (missing) read-only flag")
  if (is.null(isReadonly) || is.na(isReadonly) || !is.logical(isReadonly)) stop("invalid or empty read-only flag")

  # execute in transaction scope
  isTrxCompleted <- FALSE;
  tryCatch({
    dbBeginTransaction(dbCon)
    
    # set readonly flag by workset id
    rdOnlyVal <- ifelse(isReadonly, 1L, 0L)
    setRs <- lockWorksetUsingReadonly(dbCon, TRUE, worksetId, "", FALSE, "", FALSE, rdOnlyVal)
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
#   model defined by name and (optional) timestamp
#   if timestamp missing or NULL then use min(timestamp)
#
# dbCon - database connection
# isReadonly - if TRUE then set workset as read-only
# modelName - model name, ie: "modelOne"
# modelTimestamp - model timestamp, ie: "_201208171604590148_"
#   if timestamp missing then use min(timestamp)
#
setReadonlyDefaultWorkset <- function(dbCon, isReadonly, modelName, modelTimestamp = NULL)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(modelName)) stop("invalid (missing) model name")
  if (is.null(modelName)) stop("invalid or empty  model name")
  if (is.na(modelName) || !is.character(modelName) || length(modelName) <= 0) {
    stop("invalid or empty  model name")
  }

  isMinTs <- FALSE
  if (missing(modelTimestamp)) {
    isMinTs <- TRUE
  }
  else {
    isMinTs <- is.null(modelTimestamp) || is.na(modelTimestamp)
  }
  
  if (missing(isReadonly)) stop("invalid (missing) read-only flag")
  if (is.null(isReadonly) || is.na(isReadonly) || !is.logical(isReadonly)) stop("invalid or empty read-only flag")
  
  # execute in transaction scope
  setId <- 0L
  isTrxCompleted <- FALSE;
  tryCatch({
    dbBeginTransaction(dbCon)
    
    # set readonly flag by model name and timestamp
    rdOnlyVal <- ifelse(isReadonly, 1L, 0L)
    setRs <- lockWorksetUsingReadonly(dbCon, FALSE, 0, modelName, !isMinTs, modelTimestamp, FALSE, rdOnlyVal)
    if (is.null(setRs) || setRs$is_readonly != rdOnlyVal) {
      stop("workset not found or has invalid read-only status, model: ", modelName)
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
# Set read-only flag for workset by id or model name and (optional) timestamp
#   return model_id, model_name, model_ts, set_id, is_readonly
#
# dbCon - database connection
# i_isSetId - if true then use set id else model name and timestamp
# i_setId - id of parameters working set
# modelName - model name, ie: "modelOne"
# i_isTimestamp - if true then use timestamp else use min(timestamp)
# modelTimestamp - model timestamp, ie: "_201208171604590148_"
# i_isAddValue - if TRUE then add value to is_readonly else set
# i_readonlyValue - integer to set is_readonly
#   if i_isAddValue is true then is_readonly = i_isAddValue
#   else is_readonly = is_readonly + i_isAddValue
#
lockWorksetUsingReadonly <- function(
  dbCon, i_isSetId, i_setId, modelName, i_isTimestamp, modelTimestamp, i_isAddValue, i_readonlyValue
  )
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
        sep=""
      )
    )
    setRs <- dbGetQuery(
      dbCon, 
      paste(
        "SELECT MD.model_id, MD.model_name, MD.model_ts, WS.set_id, WS.is_readonly", 
        " FROM model_dic MD",
        " INNER JOIN workset_lst WS ON (WS.model_id = MD.model_id)",
        " WHERE WS.set_id = ", i_setId, 
        sep=""
      )
    )
  }
  else {    # use model name and (optional) timestamp
  
    if (i_isTimestamp) {  # use model name and timestamp
      dbGetQuery(
        dbCon, 
        paste(
          "UPDATE workset_lst SET is_readonly = ", rdOnlyExpr,
          " WHERE set_id = ",
          " (",
          " SELECT MIN(MWS.set_id)",
          " FROM workset_lst MWS INNER JOIN model_dic MMD ON (MMD.model_id = MWS.model_id)",
          " WHERE MMD.model_name = ", toQuoted(modelName),
          " AND MMD.model_ts = ", toQuoted(modelTimestamp),
          " )",
          sep=""
        )
      )
      setRs <- dbGetQuery(
        dbCon, 
        paste(
          "SELECT MD.model_id, MD.model_name, MD.model_ts, WS.set_id, WS.is_readonly", 
          " FROM model_dic MD",
          " INNER JOIN workset_lst WS ON (WS.model_id = MD.model_id)",
          " WHERE WS.set_id = ",
          " (",
          " SELECT MIN(MWS.set_id)",
          " FROM workset_lst MWS INNER JOIN model_dic MMD ON (MMD.model_id = MWS.model_id)",
          " WHERE MMD.model_name = ", toQuoted(modelName),
          " AND MMD.model_ts = ", toQuoted(modelTimestamp),
          " )",
          sep=""
        )
      )
    }
    else {    # use model name and min(timestamp)
    
      dbGetQuery(
        dbCon, 
        paste(
          "UPDATE workset_lst SET is_readonly = ", rdOnlyExpr,
          " WHERE set_id = ",
          " (",
          " SELECT MIN(MWS.set_id)",
          " FROM workset_lst MWS INNER JOIN model_dic MMD ON (MMD.model_id = MWS.model_id)",
          " WHERE MMD.model_name = ", toQuoted(modelName),
          " AND MMD.model_ts = ",
          " (",
          " SELECT MIN(MTS.model_ts) FROM model_dic MTS WHERE MTS.model_name = ", toQuoted(modelName),
          " )",
          " )",
          sep=""
        )
      )
      setRs <- dbGetQuery(
        dbCon, 
        paste(
          "SELECT MD.model_id, MD.model_name, MD.model_ts, WS.set_id, WS.is_readonly", 
          " FROM model_dic MD",
          " INNER JOIN workset_lst WS ON (WS.model_id = MD.model_id)",
          " WHERE WS.set_id = ",
          " (",
          " SELECT MIN(MWS.set_id)",
          " FROM workset_lst MWS INNER JOIN model_dic MMD ON (MMD.model_id = MWS.model_id)",
          " WHERE MMD.model_name = ", toQuoted(modelName),
          " AND MMD.model_ts = ",
          " (",
          " SELECT MIN(MTS.model_ts) FROM model_dic MTS WHERE MTS.model_name = ", toQuoted(modelName),
          " )",
          " )",
          sep=""
        )
      )
    }
  }

  # one row expected else set id or model name and timestamp is invalid
  if (is.null(setRs) || nrow(setRs) != 1) stop("workset not found: ", i_setId)
  
  return(setRs)
}
