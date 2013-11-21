##
## Copyright (c) 2013 OpenM++
## This is a free software licensed under MIT license
##

# 
# Update parameters working set with new values and value notes
#   return set id of updated workset or <= 0 on error
#
# dbCon - database connection
# worksetId - id of parameters working set
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
updateWorksetParameter <- function(dbCon, worksetId, ...)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(worksetId)) stop("invalid (missing) workset id")
  if (is.null(worksetId) || is.na(worksetId) || !is.integer(worksetId)) stop("invalid or empty workset id")
  if (worksetId <= 0L) stop("workset id must be positive: ", worksetId)

  # get list of languages and validate parameters data
  wsParamLst <- list(...)
  if (length(wsParamLst) <= 0) stop("invalid (missing) workset parameters list")

  langRs <- getLanguageRs(dbCon)

  if (!validateParameterValueLst(langRs, wsParamLst)) return(0L)

  # execute in transaction scope
  isTrxCompleted <- FALSE;
  tryCatch({
    dbBeginTransaction(dbCon)
    
    # find model by workset id, it must not be readonly workset
    setRs <- lockWorksetUsingReadonly(dbCon, TRUE, worksetId, "", FALSE, "", TRUE, -1L)
    if (setRs$is_readonly != -1L) {
      stop("workset is read-only (or invalid): ", worksetId)
    }
    
    # get model parameters definition
    defRs <- getParameterDefs(dbCon, setRs$model_name, setRs$model_ts)

    # check if supplied parameters are in model: parameter_name in parameter_dic table
    # check if supplied parameters are in workset: parameter_id in workset_parameter table
    setParamRs <- dbGetQuery(
      dbCon, 
      paste(
        "SELECT set_id, parameter_id FROM workset_parameter WHERE set_id = ", worksetId, 
        sep=""
      )
    )

    for (wsParam in wsParamLst) {

      if (!wsParam$name %in% defRs$paramDic$parameter_name) {
        stop("parameter not found in model parameters list: ", wsParam$name)
      }
      
      paramRow <- defRs$paramDic[which(defRs$paramDic$parameter_name == wsParam$name), ]
      
      if (!paramRow$parameter_id %in% setParamRs$parameter_id) {
        stop("parameter is not in workset: ", wsParam$name)
      }
    }

    #
    # update parameters value and value notes
    #
    for (wsParam in wsParamLst) {
      
      # get parameter row
      paramRow <- defRs$paramDic[which(defRs$paramDic$parameter_name == wsParam$name), ]
      
      # combine parameter metadata to insert value and notes
      paramDef <- data.frame(
        setId = worksetId, 
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
    
    # change workset time of last update and reset readonly to read-write
    dbGetQuery(
      dbCon, 
      paste(
        "UPDATE workset_lst",
        " SET is_readonly = 0, ", 
        " update_dt = ", toQuoted(format(Sys.time(), "%Y-%m-%d %H:%M:%S")),
        " WHERE set_id = ", worksetId, 
        sep=""
      )
    )
    isTrxCompleted <- TRUE; # completed OK
  },
  finally = {
    ifelse(isTrxCompleted, dbCommit(dbCon), dbRollback(dbCon))
  })
  return(ifelse(isTrxCompleted, worksetId, 0L))
}

#
# For internal use only, it MUST be in transaction scope
#
# insert or update workset parameter notes
# dbCon - database connection
# i_paramDef - data frame with $setId and $paramId
#   $setId - workset id
#   $paramId - parameter id
# i_wsParamTxt - workset parameter text:
#   data frame with $lang=language code and $note=parameter notes
#
updateWorksetParameterTxt <- function(dbCon, i_paramDef, i_wsParamTxt = NULL)
{
  # exit if no data
  if (missing(i_wsParamTxt)) return(NULL)
  if (is.null(i_wsParamTxt)) return(NULL)
  if (nrow(i_wsParamTxt) <= 0L) return(NULL)
  
  # insert rows where notes defined
  sqlIns <-
    paste(
      "INSERT INTO workset_parameter_txt (set_id, model_id, parameter_id, lang_id, note)",
      " SELECT", 
      " set_id, model_id, ", 
      i_paramDef$paramId, ", ",
      " :lang, :note",
      " FROM workset_lst",
      " WHERE set_id = ", i_paramDef$setId,
      sep = ""
    )
  sqlDel <-
    paste(
      "DELETE FROM workset_parameter_txt",
      " WHERE set_id = ", i_paramDef$setId, 
      " AND parameter_id = ", i_paramDef$paramId, 
      sep = ""
    )
    
  # execute delete and insert
  dbGetQuery(dbCon, sqlDel)
  dbGetPreparedQuery(
    dbCon, 
    sqlIns,
    bind.data = i_wsParamTxt[which(!is.null(i_wsParamTxt$"note") && !is.na(i_wsParamTxt$"lang")), ]
  )
}

#
# For internal use only, it MUST be in transaction scope
#
# insert or update workset parameter values
# dbCon - database connection
# i_paramDef - data frame with:
#   $setId - workset id
#   $paramTableName - parameter table name
# i_dimSize - vector of dimension sizes
# i_value - workset parameter values
#   it can be scalar value, vector or data frame
#   if scalar then i_dimSize must be c(NA) or c(0) or c(1)
#   if vector then prod(i_dimSize) must be equal to vector length
#   if data frame then 
#     it must have $dim0, $dim1,..., $value columns
#     and prod(i_dimSize) must be equal to length($value)
#
updateWorksetParameterValue <- function(dbCon, i_paramDef, i_dimSize = NULL, i_value = NULL)
{
  # exit if no data
  if (missing(i_value)) return(NULL)
  if (is.null(i_value)) return(NULL)
  
  # check value: it must be scalar, vector or data frame
  isScalar <- length(i_value) == 1 && !is.data.frame(i_value) && !is.list(i_value)
  isVector <- !isScalar && is.vector(i_value) && !is.list(i_value)
  isFrame <- !isScalar && !isVector && is.data.frame(i_value)
  if (!isScalar && !isVector && !isFrame) {
    stop("value must be scalar, vector or data frame")
  }
  
  # check value column size, it must be same as production of dimension sizes
  # for data frame check presence of $value and $dim0,... columns
  valSize <- prod(i_dimSize, na.rm = TRUE)
  dimCount <- length(i_dimSize)
  
  if (isScalar) {
    if(valSize != 1) stop("value size must be 0, 1, or NA")
  }
  if (isVector) {
    if(valSize != length(i_value)) stop("invalid value size, expected: ", valSize)
  }
  if (isFrame) {
    if (is.null(i_value$"value")) stop("value data frame must have $value column")
    if(valSize != length(i_value$value)) stop("invalid value size, expected: ", valSize)
    if (any(i_dimSize <= 0, na.rm = TRUE)) stop("size of dimensions must be a positive")
    
    if (!all(paste("dim", 0:(dimCount - 1), sep = "") %in% names(i_value))) {
      stop("value data frame must have dimension columns: ", paste("dim", 0:(dimCount - 1), sep = "", collapse=", "))
    }
  }
  
  #
  # make data frame to insert
  #
  dbDf <- NULL
  if (isScalar) dbDf <- data.frame(value = i_value)
  
  if (isVector) {
    dbDf <- data.frame(value = i_value)
    
    # make items for all dimensions
    for (k in 1L:length(i_dimSize)) {
      dbDf[paste("dim", k - 1L, sep="")] <- 
        rep(
          seq.int(from = 0L, length.out = i_dimSize[k]), 
          times = prod(head(i_dimSize, k - 1L)), 
          each = prod(tail(i_dimSize, length(i_dimSize) - k))
        )
    }
  }
  
  if (isFrame) dbDf <- i_value
  
  # use smallint as boolean type
  if (is.logical(dbDf$value)) dbDf$value <- ifelse(dbDf$value, 1, 0)
  
  #
  # DELETE FROM param_tbl WHERE set_id = 1234
  #
  # INSERT INTO param_tbl (set_id, dim0, dim1, value) 
  # VALUES (1234, :dim0, :dim1, :value)
  #
  sqlIns <-
    ifelse(isScalar,
      paste(
        "INSERT INTO ", i_paramDef$paramTableName, " (set_id, value)",
        " VALUES", 
        " (", i_paramDef$setId, ", :value)",
        sep = ""
      ),
      paste(
        "INSERT INTO ", i_paramDef$paramTableName, 
        " (set_id, ",
        paste("dim", 0L:(length(i_dimSize) - 1L), sep = "", collapse = ", "), ", ",
        " value)",
        " VALUES (",
        i_paramDef$setId, ", ",
        paste(":dim", 0L:(length(i_dimSize) - 1L), sep = "", collapse = ", "), ", ",
        " :value)",
        sep = ""
      )
    )
  sqlDel <-
    paste(
      "DELETE FROM ", i_paramDef$paramTableName, 
      " WHERE set_id = ", i_paramDef$setId,
      sep = ""
    )

  # execute delete and insert
  dbGetQuery(dbCon, sqlDel)
  dbGetPreparedQuery(dbCon, sqlIns, bind.data = dbDf)
}

# 
# For internal use only
#
# Validate workset text parameter
#   stop if invalid, return FALSE if empty, return TRUE if any data
#
# i_langRs - lang_lst table rows
# i_wsTxt - (optional) workset text data frame:
#   $lang - language code
#   $descr - working set description
#   $note - (optional) working set notes
#
validateWorksetTxt <- function(i_langRs, i_wsTxt)
{
  # validate data frame itself, exit if empty
  if (missing(i_wsTxt)) return(FALSE)
  if (is.null(i_wsTxt) || is.na(i_wsTxt)) return(FALSE)
  if (!is.data.frame(i_wsTxt)) stop("workset text must be a data frame")
  if (nrow(i_wsTxt) <= 0L) return(FALSE)
  
  # workset text must have $lang, $descr, $note column
  if (is.null(i_wsTxt$"lang") || is.null(i_wsTxt$"descr") || is.null(i_wsTxt$"note")) {
    stop("workset text must have $lang, $descr, $note columns")
  }
  
  # workset text language code must NOT NULL and in the lang_lst table
  if (any(is.na(i_wsTxt$"lang"))) {
    stop("workset text must have $lang NOT NULL")
  }
  if (!all(i_wsTxt$"lang" %in% i_langRs$lang_code)) {
    stop("invalid (or empty) language of workset text")
  }

  # workset description must NOT NULL
  if (any(is.na(i_wsTxt$"descr"))) {
    stop("workset text must have $descr NOT NULL")
  }
  
  return(TRUE)  # valid and not empty
}

# 
# For internal use only
#
# Validate list of workset parameters
#   stop if invalid, return FALSE if empty, return TRUE if any data
#
# i_langRs - lang_lst table rows
# i_wsParamLst - list of parameters value and (optional) value notes
#   each element is also a list of $name, $value and $txt
#   $name - parameter name (character)
#   $value - parameter value
#     it can be scalar value, vector or data frame
#   $txt - (optional) workset parameter text:
#     data frame with $lang = language code and $note = value notes
#
validateParameterValueLst <- function(i_langRs, i_wsParamLst)
{
  # validate list itself, it must non-empty list
  if (missing(i_wsParamLst)) stop("invalid (missing) workset parameters list")
  if (is.null(i_wsParamLst) || !is.list(i_wsParamLst)) stop("invalid or empty workset parameters list")
  if (is.na(i_wsParamLst) || length(i_wsParamLst) <= 0) {
    warning("empty workset parameters list")
    return(FALSE)
  }

  # validate each parameter data, it must have $name and $value
  isAllEmpty <- TRUE
  for (wsPar in i_wsParamLst) {
  
    # parameter data must be non-empty list
    if (is.null(wsPar)) stop("invalid or empty workset parameter")
    if (!is.list(wsPar)) stop("invalid or empty workset parameter")
    if (is.na(wsPar) || length(wsPar) <= 0) stop("invalid or empty workset parameter")

    isAllEmpty <- FALSE   # non-empty parameter found
    
    # it must have $name and $value
    if (is.null(wsPar$"name") || is.null(wsPar$"value")) stop("workset parameter must have $name and $value")

    if (is.na(wsPar$"name") || !is.character(wsPar$"name") || length(wsPar$"name") <= 0) {
      stop("workset parameter must have $name NOT NULL")
    }
    
    if (is.null(wsPar$"txt")) next  # parameter value notes is optional

    # if $txt present then it must be data frame with $lang and $note
    if (!is.data.frame(wsPar$"txt")) {
      stop("workset parameter notes must be a data frame: ", wsPar$"name")
    }
    if (is.null(wsPar$"txt"$"lang") || is.null(wsPar$"txt"$"note")) {
      stop("workset parameter notes must have $lang and $note columns: ", wsPar$"name")
    }
    
    # $txt language code must NOT NULL and in the lang_lst table
    if (any(is.na(wsPar$"txt"$"lang"))) {
      stop("workset parameter notes must have $lang NOT NULL: ", wsPar$"name")
    }
    if (!all(wsPar$"txt"$"lang" %in% i_langRs$lang_code)) {
      stop("invalid language of workset parameter notes: ", wsPar$"name")
    }
  }
  
  if (isAllEmpty) {
    warning("empty workset parameters list")
    return(FALSE)
  }
  
  return(TRUE)  # valid and not empty
}  

