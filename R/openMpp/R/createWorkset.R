##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

# 
# Create new workset FULL set of model parameters
# workset must include ALL model parameters 
#
# Return set id of new workset or <= 0 on error
#
# dbCon  - database connection
# defRs  - model definition database rows
# setDef - (optional) workset definition data frame:
#   $name  - working set name
#   $lang  - language code
#   $descr - working set description
#   $note  - (optional) working set notes
# ... - list of parameters value and (optional) value notes
#   each element is also a list of $name, $subCount, $subId, $value, $txt
#   $name     - parameter name (character)
#   $subCount - (optional) number of parameter sub-values, default: 1
#   $subId    - (optional) sub-value index, default: 0
#   $value    - parameter value
#     it can be scalar value, vector or data frame
#     size of $value must be equal to production of dimension sizes
#     if data frame then 
#       it must have $dim0, $dim1,..., $value columns
#       and each column length is equal to production of dimension sizes
#   $txt - (optional) workset parameter text:
#     data frame with $lang = language code and $note = value notes
#
createWorkset <- function(dbCon, defRs, setDef, ...)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  # create new workset
  setId <- createNewWorkset(dbCon, defRs, FALSE, NA, setDef, ...)
  return(setId)
}

# 
# Create new workset as SUBSET of model parameters
# based on parameters from existing model run 
# created workset will be combination of:
#   new parameters passed through ... argument(s)
#   and existing parameters from previous model run
#
# Return set id of new workset or <= 0 on error
#
# dbCon      - database connection
# defRs      - model definition database rows
# baseRunId  - id of model run results
# setDef - (optional) workset definition data frame:
#   $name  - working set name
#   $lang  - language code
#   $descr - working set description
#   $note  - (optional) working set notes
# ... - list of parameters value and (optional) value notes
#   each element is also a list of $name, $subCount, $subId, $value, $txt
#   $name     - parameter name (character)
#   $subCount - (optional) number of parameter sub-values, default: 1
#   $subId    - (optional) sub-value index, default: 0
#   $value    - parameter value
#     it can be scalar value, vector or data frame
#     size of $value must be equal to production of dimension sizes
#     if data frame then 
#       it must have $dim0, $dim1,..., $value columns
#       and each column length is equal to production of dimension sizes
#   $txt - (optional) workset parameter text:
#     data frame with $lang = language code and $note = value notes
#
createWorksetBasedOnRun <- function(dbCon, defRs, baseRunId, setDef, ...)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  if (missing(baseRunId)) stop("invalid (missing) base run id")
  if (is.null(baseRunId)) stop("invalid (missing) base run id")
  if (is.na(baseRunId) || !is.integer(baseRunId) || baseRunId <= 0L) {
    stop("invalid (missing) base run id: ", baseRunId)
  }

  # check if base run id is belong to the model
  runRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT run_id, model_id FROM run_lst WHERE run_id = ", baseRunId,
      sep = ""
    )
  )
  if (nrow(runRs) != 1L || runRs$model_id != defRs$modelDic$model_id) {
    stop("base run id not found: ", baseRunId, " or not belong to model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)
  }

  # create new workset
  setId <- createNewWorkset(dbCon, defRs, TRUE, baseRunId, setDef, ...)
  return(setId)
}

# 
# For internal use only, it MUST be in transaction scope
#
# Create new working set of model parameters
# return set id of new workset or <= 0 on error
#
# dbCon        - database connection
# defRs        - model definition database rows
# i_isRunBased - if true then use base run id 
#                else all parameters must be supplied by ... argument(s)
# i_baseRunId  - id of model run results
# i_setDef - (optional) workset definition data frame:
#   $name  - working set name
#   $lang  - language code
#   $descr - working set description
#   $note  - (optional) working set notes
# ... - list of parameters value and (optional) value notes
#   each element is also a list of $name, $subCount, $subId, $value, $txt
#   $name     - parameter name (character)
#   $subCount - (optional) number of parameter sub-values, default: 1
#   $subId    - (optional) sub-value index, default: 0
#   $value    - parameter value
#     it can be scalar value, vector or data frame
#     size of $value must be equal to production of dimension sizes
#     if data frame then 
#       it must have $dim0, $dim1,..., $value columns
#       and each column length is equal to production of dimension sizes
#   $txt - (optional) workset parameter text:
#     data frame with $lang = language code and $note = value notes
#
createNewWorkset <- function(dbCon, defRs, i_isRunBased, i_baseRunId, i_setDef, ...)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")

  # get list of languages and validate parameters data
  wsParamLst <- list(...)
  if (!i_isRunBased && length(wsParamLst) <= 0) stop("invalid (missing) workset parameters list")
  
  if (length(wsParamLst) > 0 && !validateParameterValueLst(defRs$langLst, TRUE, wsParamLst)) return(0L)

  # validate workset text
  isAnyWsTxt <- validateTxtFrame("workset text", defRs$langLst, i_setDef)

  # check if supplied parameters are in model: parameter_name in parameter_dic table
  # if all parameters required then check if ALL parameters supplied
  if (!i_isRunBased) nameLst <- defRs$paramDic$parameter_name
  
  for (wsParam in wsParamLst) {
    if (!wsParam$name %in% defRs$paramDic$parameter_name) {
      stop("parameter not found in model parameters list: ", wsParam$name)
    }
    if (!i_isRunBased) nameLst <- nameLst[which(nameLst != wsParam$name)]
  }
  
  if (!i_isRunBased && length(nameLst) > 0) {
    stop(
      "workset must contain ALL model parameters,",
      " not found: ", length(nameLst), " parameter(s),",
      " first 10 names: ", paste(head(nameLst, n = 10), collapse = ", ")
    )
  }
  
  # execute in transaction scope
  setId <- 0L
  
  isTrxCompleted <- FALSE;
  tryCatch({
    dbBegin(dbCon)

    # get next set id
    dbExecute(dbCon, "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_id_set_id'")
    idRs <- dbGetQuery(dbCon, "SELECT id_value FROM id_lst WHERE id_key = 'run_id_set_id'")
    if (nrow(idRs) <= 0L || idRs$id_value <= 0L) stop("can not get new set id from id_lst table")
    
    setId <- idRs$id_value
    
    # workset name, make auto-name if empty
    setName <- ifelse(isAnyWsTxt, i_setDef$name, NA)
    if (is.na(setName)) setName <- toQuoted(paste("set_", setId, sep = ""))
    
    # create workset
    dbExecute(
      dbCon, 
      paste(
        "INSERT INTO workset_lst (set_id, base_run_id, model_id, set_name, is_readonly, update_dt)",
        " VALUES (",
        setId, ", ",
        ifelse(i_isRunBased, i_baseRunId, "NULL"), ", ",
        defRs$modelDic$model_id, ", ",
        toQuoted(setName), ", ",
        " 0, ",
        toQuoted(format(Sys.time(), "%Y-%m-%d %H:%M:%S")),
        " )",
        sep = ""
      )
    )
    
    # insert workset text rows where laguage and description non-empty
    if (isAnyWsTxt) {
      sqlInsTxt <-
        paste(
          "INSERT INTO workset_txt (set_id, lang_id, descr, note)",
          " VALUES (", setId, ", ", " :lang, :descr, :note", " )",
          sep = ""
        )
      dbExecute(
        dbCon, 
        sqlInsTxt,
        params = subset(i_setDef, !is.na(lang) & !is.na(descr), select = c(lang, descr, note))
      )
    }
    
    #
    # update parameters value and value notes
    #
    for (wsParam in wsParamLst) {
      
      # get parameter row
      paramRow <- defRs$paramDic[which(defRs$paramDic$parameter_name == wsParam$name), ]
      
      # validate sub-value count and index
      isCount <- !is.null(wsParam$subCount) && !is.na(wsParam$subCount)
      nCount <- ifelse(isCount, as.integer(wsParam$subCount), 1L)
      if (nCount < 1) stop("invalid (less than 1) sub-value count for parameter ", wsParam$name)
      
      if (i_isRunBased) {
        rpRs <- dbGetQuery(
          dbCon, 
          paste(
            "SELECT sub_count FROM run_parameter ",
            " WHERE run_id = ", 
            " (", 
            " SELECT base_run_id FROM run_parameter WHERE run_id = ", i_baseRunId, " AND parameter_hid = ", paramRow$parameter_hid, 
            " )",
            sep = ""
          )
        )
        if (nrow(rpRs) == 1L) {
          if (!isCount){
            nCount <- rpRs$sub_count
          } 
          else {
            if (nCount > rpRs$sub_count) stop("invalid (less than 1) sub-value count for parameter ", wsParam$name)
          }
        }
      }

      nSubId <- ifelse(!is.null(wsParam$subId) && !is.na(wsParam$subId), as.integer(wsParam$subId), 0L)
      if (nSubId < 0 || nSubId >= nCount) stop("invalid sub-value index for parameter ", wsParam$name)
      
      # add parameter into workset
      dbExecute(
        dbCon, 
        paste(
          "INSERT INTO workset_parameter (set_id, parameter_hid, sub_count) VALUES (", setId, ", ", paramRow$parameter_hid, ", ", nCount, " )",
          sep = ""
        )
      )
      
      # get name and size for each dimension if any dimensions exists for that parameter
      dimNames <- c("")
      dimLen <- c(0L)
      if (paramRow$parameter_rank > 0) {
     
        dimNames <- defRs$paramDims[which(defRs$paramDims$parameter_hid == paramRow$parameter_hid), "dim_name"]
        
        if (length(dimNames) != paramRow$parameter_rank) {
          stop("invalid length of dimension names vector for parameter: ", paramRow$parameter_name)
        }
        
        dimLen <- as.integer( 
          table(defRs$typeEnum$type_hid)[as.character(
            defRs$paramDims[which(defRs$paramDims$parameter_hid == paramRow$parameter_hid), "type_hid"]
          )] 
        )
        
        if (length(dimLen) != paramRow$parameter_rank) {
          stop("invalid length of dimension size vector for parameter: ", paramRow$parameter_name)
        }
      }
      
      # combine parameter definition to insert value and notes
      paramDef <- list(
        setId = setId, 
        subId = nSubId,
        paramHid = paramRow$parameter_hid, 
        dbTableName = paramRow$db_set_table,
        dims = data.frame(name = dimNames, size = dimLen, stringsAsFactors = FALSE)
      )
      
      # update parameter value
      updateWorksetParameterValue(dbCon, paramDef, wsParam$value)

      # if parameter value notes not empty then update value notes
      if (!is.null(wsParam$txt)) {
        updateWorksetParameterTxt(dbCon, paramDef, wsParam$txt)
      }
    }
    
    isTrxCompleted <- TRUE; # completed OK
  },
  finally = {
    ifelse(isTrxCompleted, dbCommit(dbCon), dbRollback(dbCon))
  })
  return(ifelse(isTrxCompleted, setId, 0L))
}
