#
# To run this test you must have modelOne database modelOne.sqlite in your current directory
# It must contain:
#   "modelOne" model
#   results of two model run's, usually run_id = 11 and 12
#   "taskOne" modeling taskOne, usually task_id = 1
#   results of "taskOne" run
#
library("openMpp")

#
# open db connection
#
theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))   # recommended

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
# get model run results (first run of that model, usually run id = 11)
#
runId <- getFirstRunId(theDb, defRs)

# get model last (mot recent) run id, positive integer expected
lastRunId <- getLastRunId(theDb, defRs)
if (lastRunId <= 0L) stop("model run(s) not found: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

# select output table expression(s) values of "salarySex" from model run results
#  
salarySex_expr2Rs <- selectRunOutputValue(theDb, defRs, runId, "salarySex", "expr2")
salarySex_allExprRs <- selectRunOutputValue(theDb, defRs, runId, "salarySex")

# select accumulator values of "salarySex" from model run results
#  
salarySex_allAccRs <- selectRunAccumulator(theDb, defRs, runId, "salarySex")
salarySex_acc0Rs <- selectRunAccumulator(theDb, defRs, runId, "salarySex", "acc0")

# select input parameter "ageSex" value from model run results
#  
ageSex_paramRs <- selectRunParameter(theDb, defRs, runId, "ageSex")

#
# "modelOne" model parameters:
#   age by sex parameter double[4, 2] 
#   salary by age parameter int[3, 4]
#   starting seed parameter integer value
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

# starting seed parameter value and notes
startingSeed <- list(

  name = "StartingSeed",

  txt = data.frame(
    lang = c("EN"),
    note = c("random generator starting seed"),
    stringsAsFactors = FALSE
  ),
  value = 127L
)

#
# name, description and notes for this set of model parameters
#
paramSetTxt <- data.frame(
  name = "myData",
  lang = c("EN", "FR"),
  descr = c("default set of parameters", "FR default set of parameters"),
  note = c("default set of parameters notes", NA),
  stringsAsFactors = FALSE
)

# 
# create new working set of model parameters
#
# workset (working set of model input parameters):
#   it can be a full set, which include all model parameters 
#   or subset and include only some parameters
# each model must have "default" workset:
#   default workset must include ALL model parameters (it is a full set)
#   default workset is a first workset of the model: id = min(set_id)
# if workset is a subset (does not include all model parameters)
#   then it must be based on model run results, specified by run_id
# workset can be editable or read-only
#   if workset is editable then you can modify input parameters or workset description, notes, etc.
#   if workset is read-only then you can run the model using that workset as input
#

# create new working set of model parameters
# it is a full set and includes all "modelOne" parameters: "ageSex", "salaryAge", "StartingSeed"
#
setId <- createWorkset(theDb, defRs, paramSetTxt, ageSex, salaryAge, startingSeed)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

# find working set id by name
#
setId <- getWorksetIdByName(theDb, defRs, "myData")
if (setId <= 0L) warning("workset not found: ", "myData")

# create another workset with different description and notes in English an French
# workset name will be generated automatically
#
paramSetTxt$name <- NA
paramSetTxt$descr <- c("other set of parameters", "FR other set of parameters")
paramSetTxt$note <- NA

setId <- createWorkset(theDb, defRs, paramSetTxt, ageSex, salaryAge, startingSeed)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

# 
# create new working set of model parameters based on existing model run results
#
# that new workset is a subset and include only one model parameter: "salaryAge"
# it is based on existing model run (i.e.: first run of that model)
# and the rest of model parameters (i.e.: "ageSex") would get values from that run
#
runId <- getFirstRunId(theDb, defRs)

setId <- createWorksetBasedOnRun(theDb, defRs, runId, NA, salaryAge)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

# 
# create new working set of model parameters based on existing model run results
#
# that new workset is a subset, which does not include any parameters yet
# it is based on some existing model run 
# and all model parameters (i.e.: "ageSex") would get values from that run
#
runId <- getFirstRunId(theDb, defRs)

paramSetTxt$name <- "otherSet"
paramSetTxt$descr <- c("initially empty set of parameters")

setId <- createWorksetBasedOnRun(theDb, defRs, runId, paramSetTxt)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

#
# copy ageSex parameter values from other model run (run_id = 12)
#
otherRunId <- 1L + getFirstRunId(theDb, defRs)

ageSexCopy <- list(

  name = "ageSex",  # parameter name

  txt = data.frame(
    lang = c("EN"),
    note = c("age by sex value copy from other model run"),
    stringsAsFactors = FALSE
  )
)

copyWorksetParameterFromRun(theDb, defRs, setId, otherRunId, ageSexCopy)

#
# update parameters working set with new values and value notes 
#   reset read-only status of workset before the update
#   and make workset read-only after update
#
setId <- getDefaultWorksetId(theDb, defRs)
if (setId <= 0L) stop("no any worksets exists for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

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

updateWorksetParameter(theDb, defRs, setId, ageSex, startingSeed)
setReadonlyDefaultWorkset(theDb, defRs, TRUE)

#
# modeling task: named set of model inputs (of working sets)
# it is a convenient way to run the model with multiple inputs
#

# find modeling task id by name
taskName <- "taskOne"

taskId <- getTaskIdByName(theDb, defRs, taskName)
if (taskId <= 0L) stop("task: ", taskName, " not found for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

# find modeling task id by name for any model
taskId <- getTaskIdByName(theDb, NA, taskName)
if (taskId <= 0L) stop("task not found by name: ", taskName)

# 
# create new modeling task
#
myTaskTxt <- data.frame(
  name = "myTask",
  lang = c("EN", "FR"),
  descr = c("my first modeling task", "description in French"),
  note = c("this is a test task and include two model input data sets with id 2 and 4", NA),
  stringsAsFactors = FALSE
)
  
# create new task, initially empty
taskId <- createTask(theDb, defRs, myTaskTxt, c(2, 4))
if (taskId <= 0L) stop("task creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

# update task with additional input worksets set_id = c(2, 4)
# it is does nothing because task already contains those worksets
taskId <- updateTask(theDb, defRs, taskId, setIds = c(2, 4))
if (taskId <= 0L) stop("task update failed, id: ", taskId, ", ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

#
# select list of modeling tasks and task run(s)
#
taskName <- "taskOne"
taskId <- getTaskIdByName(theDb, defRs, taskName)

firstId <- getTaskFirstRunId(theDb, taskId)
if (firstId <= 0L) stop("task run(s) not found, task: ", taskId, " ", taskName)

lastId <- getTaskLastRunId(theDb, taskId)
if (lastId <= 0L) stop("task run(s) not found, task: ", taskId, " ", taskName)

# select task name, description and input list
taskId <- getTaskIdByName(theDb, defRs, "taskOne")

taskRs <- selectTask(theDb, taskId)

# select list of all tasks for the model
taskLstRs <- selectTaskList(theDb, defRs)

# select task run status, input and output
taskRunRs <- selectTaskRun(theDb, lastId)

# select list of all task runs
taskRunLstRs <- selectTaskRunList(theDb, taskId)

#
# all done: release database connection
#
dbDisconnect(theDb)
