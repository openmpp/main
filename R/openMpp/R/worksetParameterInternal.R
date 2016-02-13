##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

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
    bind.data = i_wsParamTxt[which(!is.na(i_wsParamTxt$lang) & !is.na(i_wsParamTxt$note)), ]
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
#   $dimNames - vector dimension names
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
    valSize <- ifelse(is.na(valSize) || valSize == 0L, 1L, valSize)
    if(valSize != 1) stop("value size must be 0, 1, or NA")
  }
  if (isVector) {
    if(valSize != length(i_value)) stop("invalid value size, expected: ", valSize)
  }
  if (isFrame) {
    if (is.null(i_value$"value")) stop("value data frame must have $value column")
    if(valSize != length(i_value$value)) stop("invalid value size, expected: ", valSize)
    if(dimCount != length(i_paramDef$dimNames)) stop("invalid number of dimension names, expected: ", dimCount)
    if (any(i_dimSize <= 0, na.rm = TRUE)) stop("size of dimensions must be a positive")
    
    if (!all(i_paramDef$dimNames %in% names(i_value))) {
      stop("value data frame must have dimension columns: ", paste(i_paramDef$dimNames, sep = "", collapse=", "))
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
      dbDf[i_paramDef$dimNames[k - 1L]] <- 
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
  sqlDel <-
    paste(
      "DELETE FROM ", i_paramDef$paramTableName, 
      " WHERE set_id = ", i_paramDef$setId,
      sep = ""
    )
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
        paste(i_paramDef$dimNames, sep = "", collapse=", "), ", ",
        " value)",
        " VALUES (",
        i_paramDef$setId, ", ",
        paste(paste(":", i_paramDef$dimNames, sep = ""), sep = "", collapse = ", "), ", ",
        " :value)",
        sep = ""
      )
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
#   $name  - working set name
#   $lang  - language code
#   $descr - working set description
#   $note  - (optional) working set notes
#
validateWorksetTxt <- function(i_langRs, i_wsTxt)
{
  # validate data frame itself, exit if empty
  if (missing(i_wsTxt)) return(FALSE)
  if (is.null(i_wsTxt) || is.na(i_wsTxt)) return(FALSE)
  if (!is.data.frame(i_wsTxt)) stop("workset text must be a data frame")
  if (nrow(i_wsTxt) <= 0L) return(FALSE)
  
  # workset text must have $name, $lang, $descr, $note column
  if (is.null(i_wsTxt$"name") || is.null(i_wsTxt$"lang") || 
      is.null(i_wsTxt$"descr") || is.null(i_wsTxt$"note")) {
    stop("workset text must have $name, $lang, $descr, $note columns")
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
# i_langRs     - lang_lst table rows
# i_isCreate   - if true then parameters must have $value
# i_wsParamLst - list of parameters value and (optional) value notes
#   each element is also a list of $name, $value and $txt
#   $name - parameter name (character)
#   $value - parameter value
#     it can be scalar value, vector or data frame
#   $txt - (optional) workset parameter text:
#     data frame with $lang = language code and $note = value notes
#
validateParameterValueLst <- function(i_langRs, i_isCreate, i_wsParamLst)
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
    
    # it must have $name
    if (is.null(wsPar$"name")) stop("workset parameter must have $name")

    if (is.na(wsPar$"name") || !is.character(wsPar$"name") || length(wsPar$"name") <= 0) {
      stop("workset parameter must have $name NOT NULL")
    }

    # if partameter created then it must have $value
    if (i_isCreate && is.null(wsPar$"value")) stop("workset parameter must have $value")
    
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

