# use openMpp library for openM++ database access
library("openMpp")

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
theDb <- dbConnect("SQLite", "/mirror/Alpha1.sqlite", synchronous = 2)
invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

defRs <- getModel(theDb, "Alpha1")      # find Alpha1 model in database

setId <- createWorkset(theDb, defRs, paramSetTxt, MortalityHazard, HappinessReversalHazard)
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

  # run the model on cluster using 16 subsamples
  system2(
    "/mirror/omrun", 
    paste(
      "-n 16 /mirror/alpha1",
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
