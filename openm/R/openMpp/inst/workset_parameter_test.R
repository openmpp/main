#
# To run this test you must have modelOne database m1.sqlite in your home directory
# It must contain "modelOne" model and run_id = 11 (see runId usage below)
#
library("openMpp")

#
# open db connection
#
theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

# find the model: get model definition from database
#
# model can be found by name and (optional) timestamp
# timestamp is used to identify exact version of the model 
# in case of multiple versions of the model with the same name
# if timestamp is missing or NA or NULL and database has multiple model versions
# then min(timestamp) is used to find first version of the model
#

# get model by name: use such call if you have only one version of the model
defRs <- getModel(theDb, "modelOne")

# use model timestamp to identify exact version of the model
defRs <- getModel(theDb, "modelOne", "_201208171604590148_")

#
# "modelOne" model parameters:
#   age by sex parameter double[4, 2] 
#   salary by age parameter int[3, 4]
#

# age by sex parameter value and notes
ageSex <- list(

  name = "ageSex",  # parameter name

  txt = data.frame(
    lang = c("EN", "FR"),
    note = c(
      "age by sex value notes", # EN value notes
      NA                        # NA == no FR value notes 
    ),
    stringsAsFactors = FALSE
  ),
  
  value = c(
    10,
    rep(c(1, 2, 3), times = 2),
    20
  )
)

# salary by age parameter value and notes
salaryAge <- list(

  name = "salaryAge",

  txt = data.frame(
    lang = c("EN", "FR"),
    note = c("salary by age value notes", "FR salary by age value notes"),
    stringsAsFactors = FALSE
  ),
  value = c(
    100L,
    rep(c(10L, 20L, 30L), times = 3),
    200L,
    300L
  )
)

#
# description and notes for this set of model parameters
#
paramSetTxt <- data.frame(
  lang = c("EN", "FR"),
  descr = c("default set of parameters", "FR default set of parameters"),
  note = c("default set of parameters notes", NA),
  stringsAsFactors = FALSE
)

# 
# create new working set of model parameters
#
# workset (working set of model parameters):
#   it can be a full set, which include all model parameters 
#   or subset and include only some parameters
# each model must have "default" workset:
#   default workset is a first workset of the model: id = min(set_id)
#   default workset always include ALL model parameters (it is a full set)
# if workset is a subset (does not include all model parameters)
#   then it must be based on model run results, specified by run_id
#

# create new working set of model parameters
# it is a full set and includes all "modelOne" parameters: "ageSex" and "salaryAge"
#
setId <- createWorkset(theDb, defRs, paramSetTxt, ageSex, salaryAge)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

# create another workset with different description and notes in English an French
#
paramSetTxt$descr <- c("other set of parameters", "FR other set of parameters")
paramSetTxt$note <- NA

setId <- createWorkset(theDb, defRs, paramSetTxt, ageSex, salaryAge)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

# 
# create new working set of model parameters based on existing model run results
#
# that new workset is a subset and include only one model parameter: "salaryAge"
# it is based on existing model run with run_id = 11 
# and the rest of model parameters (i.e.: "ageSex")  would get values from that run
#
runId <- 11L

setId <- createWorksetBasedOnRun(theDb, defRs, runId, NA, salaryAge)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

#
# update parameters working set (set_id=3) with new values and value notes 
#   reset read-only status of workset before the update
#   and make workset read-only after update
#
setId <- 3L
if (setReadonlyWorkset(theDb, defRs, FALSE, setId) != setId) {
  stop("workset not found: ", setId, " for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)
}

updateWorksetParameter(theDb, defRs, setId, ageSex)
setReadonlyWorkset(theDb, defRs, TRUE, setId)

#
# update default working set of model parameters
#   default workset is a first workset of the model: id = min(set_id)
#   default workset always include ALL model parameters
#
setId <- setReadonlyDefaultWorkset(theDb, defRs, FALSE)
if (setId <= 0L) stop("no any worksets exists for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

updateWorksetParameter(theDb, defRs, setId, ageSex, salaryAge)
setReadonlyDefaultWorkset(theDb, defRs, TRUE)

# 
# get model run results with run_id = 11
#
runId <- 11L

# select output table expression(s) values of "salarySex" from model run results with id = 11L
#  
expr2_ValueRs <- selectRunOutputValue(theDb, defRs, runId, "salarySex", "Expr2")
allExprValueRs <- selectRunOutputValue(theDb, defRs, runId, "salarySex")

# select accumulator values of "salarySex" from model run results with id = 11L
#  
allAccValueAllSubRs <- selectRunAccumulator(theDb, defRs, runId, "salarySex")
acc0_ValueAllSubRs <- selectRunAccumulator(theDb, defRs, runId, "salarySex", "acc0")
acc0_ValueRs <- selectRunAccumulator(theDb, defRs, runId, "salarySex", "acc0", 0L)
allAccValue_sub0_Rs <- selectRunAccumulator(theDb, defRs, runId, "salarySex", NA, 0L)

# select parameter "ageSex" value from model run results with id = 11L
#  
paramValueRs <- selectRunParameter(theDb, defRs, runId, "ageSex")

# release database connection
#
dbDisconnect(theDb)
