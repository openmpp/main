# use openMpp library for openM++ database access
library("openMpp")

#
# Using RiskPaths model 
# to analyze contribution of delayed union formations versus decreased fertility on childlessness
#
# Input parameters:
#   AgeBaselineForm1: age baseline for first union formation
#   UnionStatusPreg1: relative risks of union status on first pregnancy
# Output value:
#   T05_CohortFertility: Cohort fertility, expression 1

##################################################################
# To run this example please uncomment and/or change values below
# to match your hardware and file system environment.
#
### For running on a local machine using the working directory in R 
#
# For the following values to work, you must first set the R Working directory
# to the directory containing the RiskPaths executable and the SQLite database.
# In RStudio Session > Set Working Directory > Choose Directory, 
# then navigate to location, e.g.: /ompp_root/models/RiskPaths/ompp/bin
#
model_exe = "./RiskPaths"
model_sqlite = "RiskPaths.sqlite"
#                8 simulation members and 4 threads
model_args = " -General.Subsamples 8 -General.Threads 4" 
#                default: 1 simulation member and 1 thread
# model_args = " " 
#
### For running on a local machine using explicit paths
#
# model_exe = "/path/to/executable/model/RiskPaths"
# model_sqlite = "/path/to/SQLite/database/RiskPaths.sqlite"
#
### For running on cluster (change to match your cluster)
#
# model_exe = "/mirror/omrun"
# model_sqlite = "/mirror/RiskPaths.sqlite"
# model_args = "-n 16 /mirror/RiskPaths -General.Subsamples 32 -General.Threads 4"
##################################################################

# 
# connect to database to model databes
#
theDb <- dbConnect(RSQLite::SQLite(), model_sqlite, synchronous = "full")
invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

defRs <- getModel(theDb, "RiskPaths")       # find RiskPaths model in database

#
# create a copy of default model parameters
# assuming first run of the model done with default set of parameters
#
baseRunId <- getFirstRunId(theDb, defRs)
if (baseRunId <= 0) 
  stop("no run results found for the model ", i_defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

paramSetTxt <- data.frame(      # name, description and notes for new working set
  name  = "RiskPathsDataSet",
  lang  = "EN",
  descr = "Analyzing childlessness",
  note  = NA,
  stringsAsFactors = FALSE
)

setId <- createWorksetBasedOnRun(     # new working set of parameters
  theDb, defRs, baseRunId, paramSetTxt
  )
copyWorksetParameterFromRun(theDb, defRs, setId, baseRunId, list(name = "AgeBaselineForm1"))
copyWorksetParameterFromRun(theDb, defRs, setId, baseRunId, list(name = "UnionStatusPreg1"))
copyWorksetParameterFromRun(theDb, defRs, setId, baseRunId, list(name = "SimulationCases"))

#
# update number of simulation cases
#
updateWorksetParameter(theDb, defRs, setId, list(name = "SimulationCases", value = 1000L))

#
# analyze RiskPaths model varying AgeBaselineForm1 and UnionStatusPreg1 parameters
#
scaleValues <- seq(from = 0.44, to = 1.00, by = 0.02)

ageFirstUnionRs <- selectRunParameter(theDb, defRs, baseRunId, "AgeBaselineForm1")
unionStatusPregRs <- selectRunParameter(theDb, defRs, baseRunId, "UnionStatusPreg1")

for (scAgeBy in scaleValues)
{
  for (scUnionBy in scaleValues)
  {
    print(c("Scale: ", scAgeBy, " ", scUnionBy))
    
    ageParam <- list(name = "AgeBaselineForm1", value = ageFirstUnionRs$value * scAgeBy)
    
    unionParam <- list(         # scale first two values of parameter vector
      name = "UnionStatusPreg1", 
      value = 
        sapply(
          1:length(unionStatusPregRs$value), 
          function(k, sc, vec) ifelse(k <= 2, vec[k] * sc, vec[k]), 
          sc = scUnionBy, 
          vec = unionStatusPregRs$value
        )
    )
    
    # update model parameters
    setReadonlyWorkset(theDb, defRs, FALSE, setId)
    updateWorksetParameter(theDb, defRs, setId, ageParam, unionParam)
    setReadonlyWorkset(theDb, defRs, TRUE, setId)

    # run the model on cluster using 16 subsamples
    # consult your cluster admin on how to use computational grid
    system2(
      model_exe, 
      paste(
        model_args,
        " -OpenM.SetId ", setId, 
        " -OpenM.LogToConsole false",
        " -OpenM.LogToFile true",
        sep = ""
      )
    )
  }
}

#
# read final results from database
#   cohort fertility: T05_CohortFertility.expr1
#
runIdRs <- getWorksetRunIds(theDb, setId)   # get result id's

scaleLen <- length(scaleValues)
childlessnessMat <- matrix(data = NA, nrow = scaleLen, ncol = scaleLen, byrow = TRUE)

runPos <- 1
for (k in 1:scaleLen)
{
  for (j in 1:scaleLen)
  {
    expr1Rs <- selectRunOutputValue(theDb, defRs, runIdRs$run_id[runPos], "T05_CohortFertility", "expr1")
    childlessnessMat[k, j] = expr1Rs$value
    runPos <- runPos + 1
  }
}

#
# display the results
#
persp(
  x = scaleValues,
  y = scaleValues,
  z = childlessnessMat,
  xlab = "Decreased union formation",
  ylab="Decreased fertility", 
  zlab="Childlessness", 
  theta = 30, phi = 30, expand = 0.5, ticktype = "detailed",
  col = "lightgreen",
  cex.axis = 0.7
)
