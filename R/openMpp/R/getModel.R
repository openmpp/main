##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

#
# Return model metadata: parameters and output tables definition
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
#   $tableExpr - table_expr rows of the model
#
# dbCon          - database connection
# modelName      - model name, ie: "modelOne"
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
      " model_id, model_name, model_ts, model_prefix, parameter_prefix, workset_prefix, acc_prefix, value_prefix",
      " FROM model_dic",
      " WHERE model_name = ", toQuoted(modelName),
      " AND model_ts = ", toQuoted(modelTimestamp),
      " ORDER BY 1",
      sep=""
    ),
    paste(
      "SELECT",
      " model_id, model_name, model_ts, model_prefix, parameter_prefix, workset_prefix, acc_prefix, value_prefix",
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

  # output table expressions (analysis dimension items) rows: table_expr
  defRs[["tableExpr"]] <- dbGetQuery(
    dbCon, 
    paste(
      "SELECT model_id, table_id, expr_id, expr_name, expr_decimals",
      " FROM table_expr",
      " WHERE model_id = ", defRs$modelDic$model_id,
      " ORDER BY 1, 2, 3",
      sep = ""
    )
  )
  if (nrow(defRs$tableExpr) <= 0) stop("definition for output table(s) expressions not found")

  return(defRs)
}
