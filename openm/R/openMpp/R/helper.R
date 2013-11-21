##
## Copyright (c) 2013 OpenM++
## This is a free software licensed under MIT license
##

#
# return quoted string: O'Connor -> 'O''Connor'
#
toQuoted <- function(srcStr) 
{
  paste(
    "'", gsub("'", "''", srcStr, fixed = TRUE), "'", 
    sep = ""
  )
}

#
# Return language rows from lang_lst table
# dbCon - database connection
#
getLanguageRs <- function(dbCon) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  # get model_dic row: single row expected
  langRs <- dbGetQuery(
    dbCon, 
    "SELECT lang_id, lang_code, lang_name FROM lang_lst ORDER BY 1"
  )
  if (nrow(langRs) <= 0) stop("invalid database: no language(s) found")

  return(langRs)
}

#
# Return model parameters definition
#   model defined by name and (optional) timestamp
#   if timestamp is NA or NULL then use min(timestamp)
#
# return list of database rows describing model parameters:
#   $modelDic - model_dic row 
#   $typeDic - type_dic rows of the model
#   $typeEnum - (optional) type_enum_lst rows of the model
#   $paramDic - parameter_dic rows of the model
#   $paramDims - (optional) parameter_dims rows of the model
#
# dbCon - database connection
# modelName - model name, ie: "modelOne"
# modelTimestamp - model timestamp, ie: "_201208171604590148_"
#
getParameterDefs <- function(dbCon, modelName, modelTimestamp = NA) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(modelName) || is.null(modelName) || !is.character(modelName)) {
    stop("invalid or empty model name")
  }

  # get model_dic row: single row expected
  sql <- ifelse(
    !missing(modelTimestamp) && !is.null(modelTimestamp) && !is.na(modelTimestamp),
    paste(
      "SELECT model_id, model_name, model_ts, model_prefix, workset_prefix",
      " FROM model_dic",
      " WHERE model_name = ", toQuoted(modelName),
      " AND model_ts = ", toQuoted(modelTimestamp),
      " ORDER BY 1",
      sep=""
    ),
    paste(
      "SELECT model_id, model_name, model_ts, model_prefix, workset_prefix",
      " FROM model_dic",
      " WHERE model_name = ", toQuoted(modelName),
      " AND model_ts = ",
      " (",
      " SELECT MIN(MMD.model_ts) FROM model_dic MMD WHERE MMD.model_name = " , toQuoted(modelName),
      " )",
      " ORDER BY 1",
      sep=""
    )
  )
  defRs <- list(modelDic = dbGetQuery(dbCon, sql))

  if (nrow(defRs$modelDic) != 1) stop("model not found: ", modelName, " ", modelTimestamp)
  
  # model types: type_dic and type_enum_lst
  defRs[["typeDic"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, mod_type_id, mod_type_name, dic_id",
      " FROM type_dic",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1, 2",
      sep = ""
    )
  )
  if (nrow(defRs$typeDic) <= 0) stop("no model types found")

  defRs[["typeEnum"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, mod_type_id, enum_id, enum_name",
      " FROM type_enum_lst",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1, 2, 3",
      sep = ""
    )
  )
  # if (nrow(defRs$typeEnum) <= 0) stop("no model types found")

  # model parameter rows, except of generated: parameter_dic and parameter_dims
  defRs[["paramDic"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank",
      " FROM parameter_dic",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " AND is_generated = 0",
      " ORDER BY 1, 2",
      sep = ""
    )
  )
  if (nrow(defRs$paramDic) <= 0) stop("Warning: parameters not found, nothing to do")

  defRs[["paramDims"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, parameter_id, dim_name, mod_type_id",
      " FROM parameter_dims",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1, 2, 3",
      sep = ""
    )
  )

  # parameter(s) with dimensions: 
  #   must be in parameter_dims
  #   dimension type must be type_enum_lst
  if (!all(
      defRs$paramDic[which(defRs$paramDic$parameter_rank > 0)]$parameter_id %in% 
      defRs$paramDims$parameter_id
    )) {
    stop("parameter dimension(s) not found")
  }
  if (!all(defRs$paramDims$mod_type_id %in% defRs$typeEnum$mod_type_id)) {
    stop("parameter dimension(s) not found")
  }
  
  return(defRs)
}
