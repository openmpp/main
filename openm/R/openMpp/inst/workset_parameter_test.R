#
# To run this test you must have modelOne database m1.sqlite in your home directory
# It must contain run_id = 16 (see runId usage below)
#
library("openMpp")

#
# model parameters:
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
# open db connection
#
theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

# 
# create new working set of model parameters
#   model defined by name and (optional) timestamp
#   if timestamp is NA or NULL then use min(timestamp)
# workset must include ALL model parameters 
# (or create workset based on model run results to include only some parameters)
#
modelName <- "modelOne"

setId <- createWorkset(theDb, modelName, NA, paramSetTxt, ageSex, salaryAge)
if (setId <= 0L) stop("workset creation failed: ", modelName)

# create new working set of model parameters
# use model timestamp to identify exact version of the model
#
modelName <- "modelOne"
modelTs <- "_201208171604590148_"

paramSetTxt$descr <- c("other set of parameters", "FR other set of parameters")
paramSetTxt$note <- NA

setId <- createWorkset(theDb, modelName, modelTs, paramSetTxt, ageSex, salaryAge)
if (setId <= 0L) stop("workset creation failed: ", modelName, modelTs)

# 
# create new working set of model parameters based on existing model run results
#
runId <- 16L

setId <- createWorksetBasedOnRun(theDb, runId, NA, salaryAge)
if (setId <= 0L) stop("workset creation failed for base run id: ", runId)

#
# update parameters working set with new values and value notes 
# reset read-only status of workset before the update
# and make workset read-only after update
#
setId <- 3L
if (setReadonlyWorkset(theDb, FALSE, setId) != setId) stop("workset not found: ", setId)

updateWorksetParameter(theDb, setId, ageSex)
setReadonlyWorkset(theDb, TRUE, setId)

#
# update default working set of model parameters
#   default workset is a first workset: id = min(set_id)
#   model defined by name and (optional) timestamp
#   if timestamp missing then use min(timestamp)
#

# using model name only
modelName <- "modelOne"

setId <- setReadonlyDefaultWorkset(theDb, FALSE, modelName)
if (setId <= 0L) stop("no any worksets exists for model: ", modelName)

updateWorksetParameter(theDb, setId, ageSex, salaryAge)
setReadonlyDefaultWorkset(theDb, TRUE, modelName)

# using model name and timestamp
modelName <- "modelOne"
modelTs <- "_201208171604590148_"

setId <- setReadonlyDefaultWorkset(theDb, FALSE, modelName, modelTs)
if (setId <= 0L) stop("no any worksets exists for model: ", modelName, modelTs)

updateWorksetParameter(theDb, setId, ageSex, salaryAge)
setReadonlyDefaultWorkset(theDb, TRUE, modelName, modelTs)

# get model parameters definition
defRs <- getParameterDefs(theDb, modelName, modelTs)

#dbGetQuery(theDb, "SELECT * FROM workset_lst")
#dbGetQuery(theDb, "SELECT * FROM workset_parameter")
#dbGetQuery(theDb, "SELECT * FROM workset_parameter_txt")
#dbGetQuery(theDb, "SELECT COUNT(*) FROM modelone_201208171604590148_w0_ageSex")
#dbGetQuery(theDb, "SELECT * FROM modelone_201208171604590148_w0_ageSex")
#dbGetQuery(theDb, "SELECT * FROM modelone_201208171604590148_w1_salaryAge")

dbDisconnect(theDb)
