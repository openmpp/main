#
# To run this test you must have modelOne database modelOne.sqlite in your current directory
# It must contain:
#   "modelOne" model
#   results of two model run's, usually run_id = 11 and 12
#   "taskOne" modeling taskOne, usually task_id = 1
#   results of "taskOne" run
#
library("openMpp")
library("RSQLite")

#
# open db connection: SQLite model database
#
theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))   # recommended
#
# use ODBC if model database is:  MySQL, MariaDB, PostgreSQL, MSSQL, Oracle, DB2
#
# library("RODBCDBI")
# theDb <- dbConnect(RODBCDBI::ODBC(), "your-dsn-name", "your-user-name", "your-password")
#

# find the model: get model definition from database
#
# model can be found by name and (optional) digest
# digest is used to identify exact version of the model 
# if digest is missing or NA or NULL 
# and there are multiple versions of the model with the same name
# then first model with min(model_id) is used
#

# get model by name: use such call if you have only one version of the model
defRs <- getModel(theDb, "modelOne")

# use model digest to identify exact version of the model
defRs <- getModel(theDb, "modelOne", "_201208171604590148_")

# 
# get model run results (first run of that model, usually run id = 11)
#
runId <- getFirstRunId(theDb, defRs)

# get model last (mot recent) run id, positive integer expected
lastRunId <- getLastRunId(theDb, defRs)
if (lastRunId <= 0L) stop("model run(s) not found: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

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
#   age by sex:    double[4, 2] 
#   salary by age: int[3, 4]
#   salary level:  int enum[3]
#   base salary:   int enum scalar value
#   starting seed: int scalar value
#   file path:     string parameter
#

# age by sex parameter value and notes
ageSex <- list(
  name = "ageSex",  # parameter name
  subCount = 1L,    # no sub-values, only one parameter value
  subId = 0L,       # no sub-values, only one parameter value
  value = c(
    10,
    rep(c(1, 2, 3), times = 2),
    20
  ),
  txt = data.frame(
    lang = c("EN", "FR"),
    note = c(
      "age by sex value notes", # EN value notes
      NA                        # NA == no FR value notes 
    ),
    stringsAsFactors = FALSE
  )
)

# salary by age parameter, one sub-value by default
salaryAge <- list(
  name = "salaryAge",
  value = c(
    100L,
    rep(c(10L, 20L, 30L), times = 3),
    200L,
    300L
  ),
  txt = data.frame(
    lang = c("EN", "FR"),
    note = c("salary by age value notes", "FR salary by age value notes"),
    stringsAsFactors = FALSE
  )
)

# salary level (low, medium, high) by full-or-part time job
salaryFull <- list(
  name = "salaryFull", value = c(33L, 33L, 22L),
  txt = data.frame(
    lang = c("EN"), note = c("salary level for full or part time job"), stringsAsFactors = FALSE
  )
)

# base salary parameter enum value
baseSalary <- list(
  name = "baseSalary", value = 22L,
  txt = data.frame(
    lang = c("EN"), note = c("base salary notes"), stringsAsFactors = FALSE
  )
)

# starting seed parameter value and notes
startingSeed <- list(
  name = "StartingSeed", value = 127L,
  txt = data.frame(
    lang = c("EN"), note = c("random generator starting seed"), stringsAsFactors = FALSE
  )
)

# file path parameter value and notes, "filePath" is string parameter
filePath <- list(
  name = "filePath", value = "file R path",
  txt = data.frame(
    lang = c("EN"), note = c("file path string parameter"), stringsAsFactors = FALSE
  )
)

#
# name, description and notes for this set of model parameters
# for test only: make workset name unique
#
setName <- format(Sys.time(), "myData_%Y_%m_%d_%H_%M_%S")

setDef <- data.frame(
  name = setName,
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
# it is a full set and includes all "modelOne" parameters: 
#   "ageSex", "salaryAge", "salaryFull", "baseSalary", "StartingSeed"
#
setId <- createWorkset(theDb, defRs, setDef, ageSex, salaryAge, salaryFull, baseSalary, startingSeed, filePath)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

# find working set id by name
# and make workset read-only
#   if we want to use workset as input for model the it MUST be read-only
#
setId <- getWorksetIdByName(theDb, defRs, setName)
if (setId <= 0L) warning("workset not found: ", setName)

setId <- setReadonlyWorkset(theDb, defRs, TRUE, setId)
if (setId <= 0L) stop("failed to set read-only status of workset: ", setName)

#
# create another workset with different description and notes in English an French
# workset name will be generated automatically
#
setDef$name <- NA
setDef$descr <- c("other set of parameters", "FR other set of parameters")
setDef$note <- NA

setId <- createWorkset(theDb, defRs, setDef, ageSex, salaryAge, baseSalary, salaryFull, startingSeed, filePath)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

# 
# create new working set of model parameters based on existing model run results
#
# that new workset is a subset and include only one model parameter: "salaryAge"
# it is based on existing model run (i.e.: first run of that model)
# and the rest of model parameters (i.e.: "ageSex") would get values from that run
#
runId <- getFirstRunId(theDb, defRs)

setId <- createWorksetBasedOnRun(theDb, defRs, runId, NA, salaryAge)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

# 
# create new working set of model parameters based on existing model run results
#
# that new workset is an empty subset, which does not include any parameters yet
# it is based on some existing model run 
# and all model parameters (i.e.: "ageSex") would get values from that run
#
firstRunId <- getFirstRunId(theDb, defRs)

# for test only: make workset name unique
setName <- format(Sys.time(), "otherSet_%Y_%m_%d_%H_%M_%S")
setDef$name <- setName
setDef$descr <- c("initially empty set of parameters")

setId <- createWorksetBasedOnRun(theDb, defRs, firstRunId, setDef)
if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

#
# copy ageSex parameter values from other model run (second model run)
# all sub-values of ageSex parameter will be copied
#
secondRunId <- 1L + getFirstRunId(theDb, defRs)

ageSexCopy <- list(
  name = "ageSex",  # parameter name to copy
  txt = data.frame(
    lang = c("EN"),
    note = c("age by sex value copy from other model run"),
    stringsAsFactors = FALSE
  )
)

setId <- copyWorksetParameterFromRun(theDb, defRs, setId, secondRunId, ageSexCopy)
if (setId <= 0L) stop("copy parameter failed: ", ageSexCopy$name)

#
# create new working set with 2 sub-values for parameter "starting seed"
#   for test only: make workset name unique
#
setName <- format(Sys.time(), "setWithSubValues_%Y_%m_%d_%H_%M_%S")
setDef$name <- setName
setDef$descr <- c("set with 4 sub-values")

seedSubVal <- list(
  name = "StartingSeed", 
  subCount = 2L,        # two sub-values for that parameter
  subId = 0L,           # sub-value index =0 
  value = 100L          # sub-value[0] = 100
)

setId <- createWorksetBasedOnRun(theDb, defRs, firstRunId, setDef, seedSubVal)
if (setId <= 0L) stop("workset creation failed: ", setName)

#
# add seed parameter sub-value =1
#
seedSubVal <- list(name = "StartingSeed", subId = 1L, value = 200L)

setId <- updateWorksetParameter(theDb, defRs, setId, seedSubVal)
if (setId <= 0L) stop("failed to update workset parameter: ", seedSubVal$name)

#
# find defult workset and set/clear read-only status
#   default workset is a first workset of the model: id = min(set_id)
#   please note: 
#   it is not recommended to change parameters in model default workset
#
defaultSetId <- getDefaultWorksetId(theDb, defRs)
if (defaultSetId <= 0L) stop("no any worksets exists for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

setId <- setReadonlyWorkset(theDb, defRs, FALSE, defaultSetId)
if (setId <= 0L) stop("failed to clear read-only status of default workset for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

setId <- setReadonlyDefaultWorkset(theDb, defRs, TRUE)
if (setId <= 0L) stop("failed to set read-only status of default workset for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

#
# modeling task: named set of model inputs (of working sets)
# it is a convenient way to run the model with multiple inputs
#

# find modeling task id by name
taskName <- "taskOne"

taskId <- getTaskIdByName(theDb, defRs, taskName)
if (taskId <= 0L) stop("task: ", taskName, " not found for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

# find modeling task id by name for any model
taskId <- getTaskIdByName(theDb, NA, taskName)
if (taskId <= 0L) stop("task not found by name: ", taskName)

# 
# create new modeling task
# task name will be generated automatically
#
myTaskTxt <- data.frame(
  name = NA,
  lang = c("EN", "FR"),
  descr = c("my first modeling task", "description in French"),
  note = c("this is a test task and include two model input data sets with id 2 and 4", NA),
  stringsAsFactors = FALSE
)

# create new task, initially empty
taskId <- createTask(theDb, defRs, myTaskTxt, c(2, 4))
if (taskId <= 0L) stop("task creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

# update task with additional input worksets set_id = c(2, 4)
# it is does nothing because task already contains those worksets
taskId <- updateTask(theDb, defRs, taskId, setIds = c(2, 4))
if (taskId <= 0L) stop("task update failed, id: ", taskId, ", ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)

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
