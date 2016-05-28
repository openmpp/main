##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

# 
# Update parameters working set with new values and value notes
#   return set id of updated workset or 0 on error
#
# dbCon     - database connection
# defRs     - model definition database rows
# worksetId - id of parameters working set
# ...       - list of parameters value and (optional) value notes
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
updateWorksetParameter <- function(dbCon, defRs, worksetId, ...)
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  if (missing(worksetId)) stop("invalid (missing) workset id")
  if (is.null(worksetId) || is.na(worksetId) || !is.integer(worksetId)) stop("invalid or empty workset id")
  if (worksetId <= 0L) stop("workset id must be positive: ", worksetId)

  # get list of languages and validate parameters data
  wsParamLst <- list(...)
  if (length(wsParamLst) <= 0) stop("invalid (missing) workset parameters list")

  if (!validateParameterValueLst(defRs$langLst, FALSE, wsParamLst)) return(0L)

  # execute in transaction scope
  isTrxCompleted <- FALSE;
  tryCatch({
    dbBegin(dbCon)
    
    # find model by workset id, it must not be readonly workset
    setRs <- lockWorksetUsingReadonly(dbCon, defRs, TRUE, worksetId, TRUE, -1L)
    if (setRs$is_readonly != -1L) {
      stop("workset is read-only (or invalid): ", worksetId)
    }
    
    # check if supplied parameters are in model: parameter_name in parameter_dic table
    # check if supplied parameters are in workset_parameter table
    setParamRs <- dbGetQuery(
      dbCon, 
      paste(
        "SELECT set_id, parameter_hid FROM workset_parameter WHERE set_id = ", worksetId, 
        sep=""
      )
    )

    for (wsParam in wsParamLst) {

      if (!wsParam$name %in% defRs$paramDic$parameter_name) {
        stop("parameter not found in model parameters list: ", wsParam$name)
      }
      
      paramRow <- defRs$paramDic[which(defRs$paramDic$parameter_name == wsParam$name), ]
      
      if (!paramRow$parameter_hid %in% setParamRs$parameter_hid) {
        stop("parameter is not in workset: ", wsParam$name)
      }
    }

    #
    # update parameters value and value notes
    #
    for (wsParam in wsParamLst) {
      
      # get parameter row
      paramRow <- defRs$paramDic[which(defRs$paramDic$parameter_name == wsParam$name), ]
      
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
        setId = worksetId, 
        paramHid = paramRow$parameter_hid, 
        dbTableName = paste(
            paramRow$db_prefix, defRs$modelDic$workset_prefix, paramRow$db_suffix, 
            sep = ""
          ),
        dims = data.frame(name = dimNames, size = dimLen, stringsAsFactors = FALSE)
      )
      
      # update parameter value
      updateWorksetParameterValue(dbCon, paramDef, wsParam$value)

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
