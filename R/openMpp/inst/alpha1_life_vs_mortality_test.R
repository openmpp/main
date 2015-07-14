# use openMpp library for openM++ database access
library("openMpp")

###################################################################
# R integration example using Alpha1
###################################################################

# The strings model_exe, model_sqlite, model_args, and scenario_cases
# provide information to run this example in a particular hardware
# and file system environment.
# Comment all except one of each, and change to suit as desired.


###################################################################
# For running on a local machine using the working directory in R #
###################################################################

# For the following values to work, you must first set the R Working directory
# to the directory containing the Alpha1 executable and the SQLite database.
# In RStudio Session > Set Working Directory > Choose Directory, then navigate to location, e.g.
# /ompp_root/models/Alpha1/ompp/bin
model_exe = "Alpha1"
model_sqlite = "Alpha1.sqlite"
# Default - One simulation member and one thread
model_args = " "
# 4 simulation members and 4 threads
#model_args = " -General.Subsamples 4 -General.Threads 4"
scenario_cases = 5000L


###################################################################
# For running on a local machine using explicit paths             #
###################################################################

#model_exe = "/path/to/executable/model/Alpha1"
#model_sqlite = "/path/to/SQLite/database/Alpha1.sqlite"
# Default - One simulation member and one thread
#model_args = " "
#scenario_cases = 5000L


###################################################################
# For running on cluster (change to match your cluster)           #
###################################################################

#model_exe = "/mirror/omrun"
#model_sqlite = "/mirror/Alpha1.sqlite"
#model_args = "-n 16 /mirror/Alpha1"
#scenario_cases = 5000L


#
# Alpha1 model parameters:
#   Happiness reversal hazard: double number
#   Mortality hazard: double number
#
HappinessReversalHazard <- list(
  
  name = "HappinessReversalHazard",   # model parameter name
  value = 0.10,                       # value of parameter
  
  txt = data.frame(
    lang = c("EN", "FR"),
    note = c("An arbitrarily selected happiness value", NA),
    stringsAsFactors = FALSE
  )
)

MortalityHazard <- list(
  
  name = "MortalityHazard",   # model parameter name
  value = 0.014,              # value of parameter
  
  txt = data.frame(
    lang = c("EN", "FR"),
    note = c("An arbitrarily selected value, chosen to produce a life expectancy of about 70 years", NA),
    stringsAsFactors = FALSE
  )
)

SimulationCases <- list(name = "SimulationCases", value = scenario_cases)
SimulationSeed <- list(name = "SimulationSeed", value = 16807)

#
# name, description and notes for this set of model parameters
#
paramSetTxt <- data.frame(
  name = "AlphaDataSet",
  lang = c("EN", "FR"),
  descr = c("Alpha1 working set of parameters", "(FR) Alpha1 working set of parameters"),
  note = c(NA, NA),
  stringsAsFactors = FALSE
)

# 
# connect to database and create new working set of model parameters
#
theDb <- dbConnect(RSQLite::SQLite(), model_sqlite, synchronous = "full")
invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

defRs <- getModel(theDb, "Alpha1")      # find Alpha1 model in database

setId <- createWorkset(
  theDb, defRs, paramSetTxt, 
  MortalityHazard, HappinessReversalHazard, SimulationCases, SimulationSeed
)
if (setId <= 0L) stop("workset creation failed")

setReadonlyWorkset(theDb, defRs, TRUE, setId)  # workset must be read-only to run the model

#
# analyze Alpha1 model varying mortality hazard values
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
#   average duration of life: DurationOfLife.expr3
#
runIdRs <- getWorksetRunIds(theDb, setId)   # get result id's

lifeDurationData <- NULL
for (runId in runIdRs$run_id)
{
  lifeDurationData <- rbind(
    lifeDurationData,
    selectRunOutputValue(theDb, defRs, runId, "DurationOfLife", "expr3")
  )
}

dbDisconnect(theDb)   # close database connection

#
# display the results
#
plot(
  mortalityData$value, 
  lifeDurationData$value, 
  type="o",
  xlab="Mortality Hazard", 
  ylab="Duration of Life", 
  col="red"
)
