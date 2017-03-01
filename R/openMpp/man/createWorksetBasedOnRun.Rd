\name{createWorksetBasedOnRun}
\alias{createWorksetBasedOnRun}
\title{
Create new working set of model parameters
}
\description{
Create new workset as SUBSET of model parameters based on parameters from existing model run 
}
\usage{
createWorksetBasedOnRun(dbCon, defRs, baseRunId, setDef, ...)
}

\arguments{
  \item{dbCon}{
    database connection
  }
  \item{defRs}{
    model definition: database rows describing model input parameters and output tables
  }
  \item{baseRunId}{
    id of model run results, must be positive integer
  }
  \item{setDef}{
    if not NA then workset description data frame:
    \itemize{
      \item \code{$name} working set name, must be unique
      \item \code{$lang} language code 
      \item \code{$descr} working set description 
      \item \code{$note} (optional) working set notes 
    }
  }
  \item{\dots}{
    list of parameters value and (optional) value notes.
    
    Each element is also a list of $name, $subCount, $subId, $value, $txt:
    \itemize{
      \item \code{$name} parameter name (character) 
      \item \code{$subCount} (optional) number of parameter sub-values, default: 1
      \item{subId} (optional) parameter sub-value index, default: 0
      \item \code{$value} parameter value
        it can be scalar value, vector or data frame
        size of $value must be equal to production of dimension sizes
        if data frame then 
          it must have $dim0, $dim1,..., $value columns
          and each column length is equal to production of dimension sizes
      \item \code{$txt} (optional) workset parameter text:
        data frame with $lang = language code and $note = value notes
    }
  }
}

\details{
That call allow you to create new working set of model parameters as combination of 
existing parameters from previous model run and some new parameters passed through \dots argument(s)

Workset is a working set of model input parameters and can be a full set, 
which include values of all model parameters or subset and include only some parameters.

Each model must have "default" workset. 
Default workset is a first workset of the model with set_id = min(set_id) for that model. 
Default workset always include ALL model parameters (it is a full set).

If you want to create new workset as a full set of model parameters
then you must pass ALL model parameters into \code{createWorksetBasedOnRun} through \dots argument list. 

If you already have result of model run in your database 
and want to modify only some input parameters (subset) then call \code{\link{createWorksetBasedOnRun}} 
in order to create workset using parameters from previous model run and supply some new values. 

You can create subset of model parameters ONLY based on existing run results. 
Otherwise you have to pass ALL (full set) of parameters in order to create workset.

Each workset has unique set id (positive integer) and unique name. 
To find set id by name use \code{\link{getWorksetIdByName}} call.

Working set must be read-only to run the model, so, typically you want to call \code{\link{setReadonlyWorkset}}
after \code{createWorksetBasedOnRun}.

Parameter(s) can have can have multiple sub-values (by default only one value, no sub-values).
If you want to have multiple sub-values then $subCount must be >1 (default =1).
If parameter have multiple sub-values then you can specify sub-value index as $subId (default =0).

You must use \code{getModel} function in order to find model definition \code{defRs}.
}
\value{
  Return id of new working set or 0L on error
}
\references{
OpenM++ documentation: http://ompp.sourceforge.net/wiki/
}
\author{
amc1999
}
\note{
To run examples you must have modelOne database modelOne.sqlite in current directory
}

\seealso{
  \code{\link{getModel}}
  \code{\link{getFirstRunId}}
  \code{\link{getLastRunId}}
  \code{\link{getDefaultWorksetId}}
  \code{\link{getWorksetIdByName}}
  \code{\link{createWorkset}}
  \code{\link{copyWorksetParameterFromRun}}
  \code{\link{setReadonlyWorkset}}
  \code{\link{setReadonlyDefaultWorkset}}
  \code{\link{updateWorksetParameter}}
}
\examples{
  # StartingSeed parameter have two sub-values
  #
  seedSubVal <- list(
    name = "StartingSeed", 
    subCount = 2L,        # two sub-values for that parameter
    subId = 0L,           # sub-value index =0 
    value = 100L          # sub-value[0] = 100
  )
  
  #
  # name, description and notes for this set of model parameters
  # name MUST be unique
  #
  inputSet <- data.frame(
    name = "myOtherData",
    lang = "EN",
    descr = "new set of parameters",
    note = "new set of parameters with updated salary by age",
    stringsAsFactors = FALSE
  )
  
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended
  
  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")
  
  #
  # pass new value for StartingSeed parameter
  # for all other parameters use previous model run with id = 101 
  #
  setId <- createWorksetBasedOnRun(theDb, defRs, 101L, inputSet, seedSubVal)
  if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)
  
  #
  # add seed parameter sub-value =1
  #
  seedSubVal <- list(name = "StartingSeed", subId = 1L, value = 200L)

  setId <- updateWorksetParameter(theDb, defRs, setId, seedSubVal)
  if (setId <= 0L) stop("failed to update workset parameter: ", seedSubVal$name)

  #
  # copy ageSex parameter value from model run with id = 102
  #
  copyWorksetParameterFromRun(theDb, defRs, setId, 102L, list(name = "ageSex"))
  
  # make workset read-only in order to run the model
  setReadonlyWorkset(theDb, defRs, TRUE, setId)
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new salaryAge parameter value and ageSex value from other run results
  #
}
\keyword{ OpenM++ }
\keyword{ database }
