##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

#
# For internal use only, it MUST be in transaction scope
#
# insert or update workset parameter notes
# dbCon - database connection
# i_paramDef - list with $setId and $paramHid
#   $setId    - workset id
#   $paramHid - parameter Hid
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
      "INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note)",
      " SELECT", 
      " set_id, ", 
      i_paramDef$paramHid, ", ",
      " :lang, :note",
      " FROM workset_lst",
      " WHERE set_id = ", i_paramDef$setId,
      sep = ""
    )
  sqlDel <-
    paste(
      "DELETE FROM workset_parameter_txt",
      " WHERE set_id = ", i_paramDef$setId, 
      " AND parameter_hid = ", i_paramDef$paramHid, 
      sep = ""
    )
    
  # execute delete and insert
  dbExecute(dbCon, sqlDel)
  dbExecute(
    dbCon, 
    sqlIns,
    params = i_wsParamTxt[which(!is.na(i_wsParamTxt$lang) & !is.na(i_wsParamTxt$note)), ]
  )
}

#
# For internal use only, it MUST be in transaction scope
#
# insert or update workset parameter values
# dbCon - database connection
# i_paramDef - list with:
#   $setId       - workset id
#   $subId       - sub-value index
#   $dbTableName - workset parameter table name
#   $dims        - data frame for each dimension
#     $name - dimension name
#     $size - dimension size
# i_value         - workset parameter values
#   it can be scalar value, vector or data frame
#   if scalar then $dims$size must be c(NA) or c(0) or c(1)
#   if vector then prod($dims$size) must be equal to vector length
#   if data frame then 
#     it must have $dim0, $dim1,..., $value columns
#     and prod($dims$size) must be equal to length($value)
#
updateWorksetParameterValue <- function(dbCon, i_paramDef, i_value = NULL)
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
  dimCount <- length(i_paramDef$dims$size)
  valSize <- prod(i_paramDef$dims$size, na.rm = TRUE)
  
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
    if (any(i_paramDef$dims$size <= 0, na.rm = TRUE)) stop("size of dimensions must be a positive")
    
    if (!all(i_paramDef$dims$name %in% names(i_value))) {
      stop("value data frame must have dimension columns: ", paste(i_paramDef$dims$name, sep = "", collapse=", "))
    }
  }
  
  #
  # make data frame to insert
  #
  dbDf <- NULL
  if (isScalar) dbDf <- data.frame(value = i_value, stringsAsFactors = FALSE)
  
  if (isVector) {
    dbDf <- data.frame(value = i_value, stringsAsFactors = FALSE)
    
    # make items for all dimensions
    for (k in 1L:dimCount) {
      dbDf[i_paramDef$dims$name[k]] <- 
        rep(
          seq.int(from = 0L, length.out = i_paramDef$dims$size[k]), 
          times = prod(head(i_paramDef$dims$size, k - 1L)), 
          each = prod(tail(i_paramDef$dims$size, dimCount - k))
        )
    }
  }
  
  if (isFrame) dbDf <- i_value
  
  # use smallint as boolean type
  if (is.logical(dbDf$value)) dbDf$value <- ifelse(dbDf$value, 1, 0)
  
  #
  # DELETE FROM param_workset_tbl WHERE set_id = 1234 AND sub_id = 7
  #
  # INSERT INTO param_workset_tbl (set_id, sub_id, dim0, dim1, param_value) 
  # VALUES (1234, 7, :dim0, :dim1, :value)
  #
  sqlDel <-
    paste(
      "DELETE FROM ", i_paramDef$dbTableName, 
      " WHERE set_id = ", i_paramDef$setId,
      " AND sub_id = ", i_paramDef$subId,
      sep = ""
    )
  sqlIns <-
    ifelse(isScalar,
      paste(
        "INSERT INTO ", i_paramDef$dbTableName, " (set_id, sub_id, param_value)",
        " VALUES", 
        " (", i_paramDef$setId, ", ", i_paramDef$subId, ", :value)",
        sep = ""
      ),
      paste(
        "INSERT INTO ", i_paramDef$dbTableName, 
        " (set_id, sub_id, ",
        paste(i_paramDef$dims$name, sep = "", collapse=", "), ", ",
        " param_value)",
        " VALUES (",
        i_paramDef$setId, ", ",
        i_paramDef$subId, ", ",
        paste(paste(":", i_paramDef$dims$name, sep = ""), sep = "", collapse = ", "), ", ",
        " :value)",
        sep = ""
      )
    )
  
  # execute delete and insert
  dbExecute(dbCon, sqlDel)
  dbExecute(dbCon, sqlIns, params = dbDf)
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
#   $name     - parameter name (character)
#   $subCount - (optional) sub-value count, default: 1
#   $subId    - (optional) sub-value index, default: 0
#   $value    - parameter value
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

    # validate sub-value count
    nCount <- 1L
    if (!is.null(wsPar$subCount) && !is.na(wsPar$subCount)) {
      if (!is.numeric(wsPar$subCount)) stop("workset parameter $subCount must be numeric, parameter: ", wsPar$name)

      nCount <- as.integer(wsPar$subCount)
      if (nCount < 1) stop("invalid number sub-values for parameter ", wsPar$name)
    }

    # validate sub-value index
    nSubId <- 0L
    if (!is.null(wsPar$subId) && !is.na(wsPar$subId)) {
      if (!is.numeric(wsPar$subId)) stop("workset parameter $subId must be numeric, parameter: ", wsPar$name)

      nSubId <- as.integer(wsPar$subId)
      if (nSubId < 0) stop("invalid sub-value index for parameter ", wsPar$name)
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

