##
## Copyright (c) 2013 OpenM++
## This is a free software licensed under MIT license
##

# 
# Create new working set of model parameters
# workset must include ALL model parameters 
#   model defined by name and (optional) timestamp
#   if timestamp is NA or NULL then use min(timestamp)
# Return set id of new workset or <= 0 on error
#
# dbCon - database connection
# modelName - model name, ie: "modelOne"
# modelTimestamp - model timestamp, ie: "_201208171604590148_"
#   if timestamp is NA or NULL then use min(timestamp)
# worksetTxt - (optional) workset text data frame:
#   $lang - language code
#   $descr - working set description
#   $note - (optional) working set notes
# ... - list of parameters value and (optional) value notes
#   each element is also a list of $name, $value and $txt
#   $name - parameter name (character)
#   $value - parameter value
#     it can be scalar value, vector or data frame
#     size of $value must be equal to production of dimension sizes
#     if data frame then 
#       it must have $dim0, $dim1,..., $value columns
#       and each column length is equal to production of dimension sizes
#   $txt - (optional) workset parameter text:
#     data frame with $lang = language code and $note = value notes
#
createWorkset <- function(dbCon, modelName, modelTimestamp, worksetTxt, ...)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(modelName)) stop("invalid (missing) model name")
  if (is.null(modelName)) stop("invalid or empty  model name")
  if (is.na(modelName) || !is.character(modelName) || length(modelName) <= 0) {
    stop("invalid or empty  model name")
  }

  # create new workset
  setId <- createWorksetParameter(dbCon, modelName, modelTimestamp, FALSE, NA, worksetTxt, ...)
  return(setId)
}

# 
# Create new working set of model parameters
# based on parameters from existing model run 
# created workset will be combination of:
#   new parameters passed through ... argument(s)
#   and existing parameters from previous model run
# Return set id of new workset or <= 0 on error
#
# dbCon - database connection
# baseRunId - id of model run results
# worksetTxt - (optional) workset text data frame:
#   $lang - language code
#   $descr - working set description
#   $note - (optional) working set notes
# ... - list of parameters value and (optional) value notes
#   each element is also a list of $name, $value and $txt
#   $name - parameter name (character)
#   $value - parameter value
#     it can be scalar value, vector or data frame
#     size of $value must be equal to production of dimension sizes
#     if data frame then 
#       it must have $dim0, $dim1,..., $value columns
#       and each column length is equal to production of dimension sizes
#   $txt - (optional) workset parameter text:
#     data frame with $lang = language code and $note = value notes
#
createWorksetBasedOnRun <- function(dbCon, baseRunId, worksetTxt, ...)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(baseRunId)) stop("invalid (missing) base run id")
  if (is.null(baseRunId)) stop("invalid (missing) base run id")
  if (is.na(baseRunId) || !is.integer(baseRunId) || baseRunId <= 0L) {
    stop("invalid (missing) base run id: ", baseRunId)
  }

  # find the model by run id
  runRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT MD.model_id, MD.model_name, MD.model_ts, RL.run_id",
      " FROM model_dic MD",
      " INNER JOIN run_lst RL ON (RL.model_id = MD.model_id)",
      " WHERE RL.run_id = ", baseRunId,
      sep = ""
    )
  )
  if (nrow(runRs) <= 0) stop("base run id not found: ", baseRunId)

  # create new workset
  setId <- createWorksetParameter(dbCon, runRs$model_name, runRs$model_ts, TRUE, baseRunId, worksetTxt, ...)
  return(setId)
}

# 
# For internal use only, it MUST be in transaction scope
#
# Create new working set of model parameters
# return set id of new workset or <= 0 on error
#
# dbCon - database connection
# modelName - model name, ie: "modelOne"
# modelTimestamp - model timestamp, ie: "_201208171604590148_"
#   if timestamp is NA or NULL then use min(timestamp)
# i_isRunBased - if true then use base run id 
#   else all parameters must be supplied by ... argument(s)
# i_baseRunId - id of model run results
# worksetTxt - (optional) workset text data frame:
#   $lang - language code
#   $descr - working set description
#   $note - (optional) working set notes
# ... - list of parameters value and (optional) value notes
#   each element is also a list of $name, $value and $txt
#   $name - parameter name (character)
#   $value - parameter value
#     it can be scalar value, vector or data frame
#     size of $value must be equal to production of dimension sizes
#     if data frame then 
#       it must have $dim0, $dim1,..., $value columns
#       and each column length is equal to production of dimension sizes
#   $txt - (optional) workset parameter text:
#     data frame with $lang = language code and $note = value notes
#
createWorksetParameter <- function(dbCon, i_modelName, i_modelTs, i_isRunBased, i_baseRunId, worksetTxt, ...)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(i_modelName)) stop("invalid (missing) model name")
  if (is.null(i_modelName)) stop("invalid or empty  model name")
  if (is.na(i_modelName) || !is.character(i_modelName) || length(i_modelName) <= 0) {
    stop("invalid or empty  model name")
  }

  # get list of languages and validate parameters data
  wsParamLst <- list(...)
  if (length(wsParamLst) <= 0) stop("invalid (missing) workset parameters list")
  
  langRs <- getLanguageRs(dbCon)

  if (!validateParameterValueLst(langRs, wsParamLst)) return(0L)

  # validate workset text
  isAnyWsTxt <- validateWorksetTxt(langRs, worksetTxt)
  
  # get model parameters definition
  defRs <- getParameterDefs(dbCon, i_modelName, i_modelTs)

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
    dbBeginTransaction(dbCon)

    # get next set id
    dbGetQuery(dbCon, "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_set'")
    idRs <- dbGetQuery(dbCon, "SELECT id_value FROM id_lst WHERE id_key = 'run_set'")
    if (nrow(idRs) <= 0L || idRs$id_value <= 0L) stop("can not get new set id from id_lst table")
    
    setId <- idRs$id_value
    
    # create workset
    dbGetQuery(
      dbCon, 
      paste(
        "INSERT INTO workset_lst (set_id, run_id, model_id, is_readonly, update_dt)",
        " VALUES (",
        setId, ", ",
        ifelse(i_isRunBased, i_baseRunId, "NULL"), ", ",
        defRs$modelDic$model_id, ", ",
        " 0, ",
        toQuoted(format(Sys.time(), "%Y-%m-%d %H:%M:%S")),
        " )",
        sep = ""
      )
    )
    
    # insert workset text rows where laguage and description non-mpty
    if (isAnyWsTxt) {
      sqlInsTxt <-
        paste(
          "INSERT INTO workset_txt (set_id, model_id, lang_id, descr, note)",
          " VALUES (", 
          setId, ", ",
          defRs$modelDic$model_id, ", ",
          " :lang, :descr, :note",
          " )",
          sep = ""
        )
      dbGetPreparedQuery(
        dbCon, 
        sqlInsTxt,
        bind.data = worksetTxt[which(!is.na(worksetTxt$lang) & !is.na(worksetTxt$descr)), ]
      )
    }
    
    #
    # update parameters value and value notes
    #
    for (wsParam in wsParamLst) {
      
      # get parameter row
      paramRow <- defRs$paramDic[which(defRs$paramDic$parameter_name == wsParam$name), ]
      
      # add parameter into workset
      dbGetQuery(
        dbCon, 
        paste(
          "INSERT INTO workset_parameter (set_id, model_id, parameter_id)",
          " VALUES (",
          setId, ", ",
          defRs$modelDic$model_id, ", ",
          paramRow$parameter_id,
          " )",
          sep = ""
        )
      )
      
      # combine parameter metadata to insert value and notes
      paramDef <- data.frame(
        setId = setId, 
        paramId = paramRow$parameter_id, 
        paramTableName = paste(
            defRs$modelDic$model_prefix, defRs$modelDic$workset_prefix, paramRow$db_name_suffix, 
            sep = ""
          )
      )
      
      # get size of dimensions vector if any dimensions exists for that parameter
      dimLen <- c(0L)
      if (paramRow$parameter_rank > 0) {
     
        dimLen <- as.integer( 
          table(defRs$typeEnum$mod_type_id)[as.character(
            defRs$paramDims[which(defRs$paramDims$parameter_id == paramRow$parameter_id), "mod_type_id"]
          )] 
        )

        if (length(dimLen) != paramRow$parameter_rank) {
          stop("invalid length of dimension size vector for parameter: ", paramRow$parameter_name)
        }
      }
      
      # update parameter value
      updateWorksetParameterValue(dbCon, paramDef, dimLen, wsParam$value)

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
