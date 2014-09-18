##
## Copyright (c) 2014 OpenM++
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
getLanguages <- function(dbCon) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  # get lang_lst rows: at least one row expected
  langRs <- dbGetQuery(
    dbCon, 
    "SELECT lang_id, lang_code, lang_name FROM lang_lst ORDER BY 1"
  )
  if (nrow(langRs) <= 0) stop("invalid database: no language(s) found")

  return(langRs)
}

#
# Return working set id by name
#   if model have no working set with that name then return is negative
#   if model has multiple working sets with that name then return min(set id)
#
# dbCon       - database connection
# defRs       - model definition database rows
# worksetName - name parameters working set
#
# return: id of parameters working set
#         positive integer on success, negative on error
#
getWorksetIdByName <- function(dbCon, defRs, worksetName) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  if (missing(worksetName)) stop("invalid (missing) workset name")
  if (is.null(worksetName) || is.na(worksetName) || !is.character(worksetName)) stop("invalid or empty workset name")
  
  # get first set id with specified name for that model id
  setRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT MIN(set_id) FROM workset_lst",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " AND set_name = ", toQuoted(worksetName),
      sep=""
    )
  )
  if (nrow(setRs) <= 0) return(-1L)

  return(ifelse(!is.na(setRs[1,1]), as.integer(setRs[1,1]), -1L))
}

#
# Return default working set id for the model
#   default workset is a first workset for the model
#   each model must have default workset
#
# dbCon - database connection
# defRs - model definition database rows
#
# return: id of parameters working set
#         positive integer on success, stop if not found
#
getDefaultWorksetId <- function(dbCon, defRs) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(defRs)) stop("invalid (missing) model definition")
  if (is.null(defRs) || is.na(defRs) || !is.list(defRs)) stop("invalid or empty model definition")
  
  # get first set id with specified name for that model id
  setRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT MIN(set_id) FROM workset_lst",
      " WHERE model_id = ", defRs$modelDic$model_id,
      sep=""
    )
  )
  # one row expected else model id is invalid
  if (is.null(setRs) || nrow(setRs) != 1) {
    stop("no worksets not found for model: ", i_defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)
  }
  
  return(ifelse(!is.na(setRs[1,1]), as.integer(setRs[1,1]), -1L))
}

#
# Return ids of model run results (run_id) 
# where input parameters are from specified working set
#
# dbCon     - database connection
# worksetId - id of parameters working set
#
# return: data frame with integer $run_id column
#
getWorksetRunIds <- function(dbCon, worksetId) 
{
  # validate input parameters
  if (missing(dbCon)) stop("invalid (missing) database connection")
  if (is.null(dbCon) || !is(dbCon, "DBIConnection")) stop("invalid database connection")

  if (missing(worksetId)) stop("invalid (missing) workset id")
  if (is.null(worksetId) || is.na(worksetId) || !is.integer(worksetId)) stop("invalid or empty workset id")
  if (worksetId <= 0L) stop("workset id must be positive: ", worksetId)
  
  # get run ids by predefined option key: OpenM.SetId
  runRs <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT RL.run_id",
      " FROM run_lst RL",
      " INNER JOIN run_option RO ON (RO.run_id = RL.run_id)",
      " WHERE RL.sub_completed = RL.sub_count",
      " AND RO.option_key = 'OpenM.SetId'",
      " AND RO.option_value = ", toQuoted(worksetId),
      " ORDER BY 1",
      sep=""
    )
  )
  # it can be empty result with nrow() = 0

  return(runRs)
}

#
# Return model parameters and output tables definition
#   model defined by name and (optional) timestamp
#   if timestamp is NA or NULL then min(timestamp) is used
#
# return list of database rows describing model:
#   $modelDic  - model_dic row 
#   $langLst   - languages: lang_lst rows
#   $typeDic   - type_dic rows of the model
#   $typeEnum  - (optional) type_enum_lst rows of the model
#   $paramDic  - parameter_dic rows of the model
#   $paramDims - (optional) parameter_dims rows of the model
#   $tableDic  - table_dic rows of the model
#   $tableDims - (optional) table_dims rows of the model
#   $tableAcc  - table_acc rows of the model
#   $tableUnit - table_unit rows of the model
#
# dbCon - database connection
# modelName - model name, ie: "modelOne"
# modelTimestamp - model timestamp, ie: "_201208171604590148_"
#
getModel <- function(dbCon, modelName, modelTimestamp = NA) 
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
      "SELECT",
      " model_id, model_name, model_ts, model_prefix,",
      " parameter_prefix, workset_prefix, sub_prefix, value_prefix",
      " FROM model_dic",
      " WHERE model_name = ", toQuoted(modelName),
      " AND model_ts = ", toQuoted(modelTimestamp),
      " ORDER BY 1",
      sep=""
    ),
    paste(
      "SELECT",
      " model_id, model_name, model_ts, model_prefix,",
      " parameter_prefix, workset_prefix, sub_prefix, value_prefix",
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

  # get languages
  defRs[["langLst"]] <- getLanguages(dbCon)
  
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

  # model parameter rows, except of generated: parameter_dic
  defRs[["paramDic"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank, mod_type_id",
      " FROM parameter_dic",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " AND is_generated = 0",
      " ORDER BY 1, 2",
      sep = ""
    )
  )
  if (nrow(defRs$paramDic) <= 0) stop("Warning: parameters not found, nothing to do")

  # parameter dimensions rows: parameter_dims
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

  # for all parameters with dimensions: 
  #   dimension must be in parameter_dims
  #   dimension type must be type_enum_lst
  if (!all(
      defRs$paramDic[which(defRs$paramDic$parameter_rank > 0), ]$parameter_id %in% 
      defRs$paramDims$parameter_id
    )) {
    stop("parameter dimension(s) not found")
  }
  if (!all(defRs$paramDims$mod_type_id %in% defRs$typeEnum$mod_type_id)) {
    stop("parameter dimension type(s) not found")
  }
  
  # model output tables rows: table_dic
  defRs[["tableDic"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, table_id, db_name_suffix, table_name, table_rank, is_sparse",
      " FROM table_dic",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1, 2",
      sep = ""
    )
  )
  if (nrow(defRs$tableDic) <= 0) stop("Warning: output tables not found, nothing to do")

  # dimensions of model output tables rows: table_dims
  defRs[["tableDims"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, table_id, dim_name, mod_type_id, is_total, dim_size",
      " FROM table_dims",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1, 2, 3",
      sep = ""
    )
  )
  
  # for all output tables with dimensions: 
  #   dimension must be in table_dims
  #   dimension type must be type_enum_lst
  if (!all(
      defRs$tableDic[which(defRs$tableDic$table_rank > 0), ]$table_id %in% 
      defRs$tableDims$table_id
    )) {
    stop("output table dimension(s) not found")
  }
  if (!all(defRs$tableDims$mod_type_id %in% defRs$typeEnum$mod_type_id)) {
    stop("output table dimension(s) not found")
  }

  # output table accumulators rows: table_acc
  defRs[["tableAcc"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, table_id, acc_id, acc_name",
      " FROM table_acc",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1, 2, 3",
      sep = ""
    )
  )
  if (nrow(defRs$tableAcc) <= 0) stop("definition for output table(s) accumulators not found")

  # output table expressions (analysis dimension items) rows: table_unit
  defRs[["tableUnit"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, table_id, unit_id, unit_name, unit_decimals",
      " FROM table_unit",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1, 2, 3",
      sep = ""
    )
  )
  if (nrow(defRs$tableUnit) <= 0) stop("definition for output table(s) expressions not found")

  return(defRs)
}
