# use openMpp library for openM++ database access
library("openMpp")
library("RSQLite")

#
# R integration example using NewCaseBased model
#   loop over MortalityHazard parameter 
#   to analyze DurationOfLife
#

##################################################################
# To run this example please uncomment and/or change values below
# to match your hardware and file system environment:
#
# model_exe    <- path to the model executable, i.e.: "./NewCaseBased" or "NewCaseBased.exe"
# model_sqlite <- path to the model.sqlite database:  "NewCaseBased.sqlite"
# model_args   <- optional arguments to control model run, for example:
#       -General.Subsamples 16 <- number of simation members
#       -General.Threads 4     <- number of computatinal threads
#
### For running on a local machine using the working directory in R 
#
# For the following values to work, you must first set the R Working directory
# to the directory containing the NewCaseBased executable and the SQLite database.
# In RStudio Session > Set Working Directory > Choose Directory, 
# then navigate to location, e.g.: /ompp_root/models/NewCaseBased/ompp/bin
#
model_exe = "./NewCaseBased"
model_sqlite = "NewCaseBased.sqlite"
model_args = " -General.Subsamples 16 -General.Threads 4" 
# model_args = " "  # default: 1 simulation member and 1 thread
#
### For running on a local machine using explicit paths
#
# model_exe = "/path/to/executable/model/NewCaseBased"
# model_sqlite = "/path/to/SQLite/database/NewCaseBased.sqlite"
#
### For running on cluster (change to match your cluster)
#
# model_exe = "/mirror/omrun"
# model_sqlite = "/mirror/NewCaseBased.sqlite"
# model_args = "-n 8 /mirror/NewCaseBased -General.Subsamples 16 -General.Threads 2"
##################################################################

#
# NewCaseBased model parameters:
#   Mortality hazard: double number
#   Simulation cases: number of simulation cases
#   Simulation seed:  random seed
#
MortalityHazard <- list(
  
  name = "MortalityHazard",   # model parameter name
  value = 0.014,              # value of parameter
  
  txt = data.frame(
    lang = c("EN", "FR"),
    note = c("An arbitrarily selected value, chosen to produce a life expectancy of about 70 years", NA),
    stringsAsFactors = FALSE
  )
)

SimulationCases <- list(name = "SimulationCases", value = 5000L)
SimulationSeed <- list(name = "SimulationSeed", value = 16807)

#
# name, description and notes for this set of model parameters
#
paramSetTxt <- data.frame(
  name = "LifeVsMortality",
  lang = c("EN", "FR"),
  descr = c("NewCaseBased working set of parameters", "(FR) NewCaseBased working set of parameters"),
  note = c(NA, NA),
  stringsAsFactors = FALSE
)

# 
# connect to database and create new working set of model parameters
#
theDb <- dbConnect(RSQLite::SQLite(), model_sqlite, synchronous = "full")
invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))   # recommended

defRs <- getModel(theDb, "NewCaseBased")      # find NewCaseBased model in database

setId <- createWorkset(
  theDb, defRs, paramSetTxt, 
  MortalityHazard, SimulationCases, SimulationSeed
)
if (setId <= 0L) stop("workset creation failed")

setReadonlyWorkset(theDb, defRs, TRUE, setId)  # workset must be read-only to run the model

#
# analyze NewCaseBased model varying mortality hazard values
#
mortalityData <- data.frame( 
  value = seq(from = 0.014, by = 0.005, length.out = 20)
)

for (mortality in mortalityData$value)
{
  print(c("Mortality hazard: ", mortality))

  system2(
    model_exe,
    paste(
      model_args,
      " -Parameter.MortalityHazard ", mortality,
      " -OpenM.SetId ", setId, 
      " -OpenM.LogToConsole false",
      " -OpenM.LogToFile true",
      sep = ""
    )
  )
}

#
# read final results from database
#   average duration of life: DurationOfLife.meas3
#
runIdRs <- getWorksetRunIds(theDb, setId)   # get result id's

lifeDurationData <- NULL
for (runId in runIdRs$run_id)
{
  lifeDurationData <- rbind(
    lifeDurationData,
    selectRunOutputValue(theDb, defRs, runId, "DurationOfLife", "meas3")
  )
}

dbDisconnect(theDb)   # close database connection

#
# display the results
#
plot(
  mortalityData$value, 
  lifeDurationData$expr_value, 
  type = "o",
  xlab = "Mortality Hazard", 
  ylab = "Duration of Life", 
  col = "red"
)
