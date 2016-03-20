\name{createWorksetBasedOnRun}
\alias{createWorksetBasedOnRun}
\title{
Create new working set of model parameters
}
\description{
Create new working set of model parameters
}
\usage{
createWorksetBasedOnRun(dbCon, defRs, baseRunId, worksetTxt, ...)
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
  \item{worksetTxt}{
    if not NA then workset text data frame:
    \itemize{
      \item \code{$name} (optional) working set name
      \item \code{$lang} language code 
      \item \code{$descr} working set description 
      \item \code{$note} (optional) working set notes 
    }
  }
  \item{\dots}{
    list of parameters value and (optional) value notes.
    
    Each element is also a list of $name, $value and $txt:
    \itemize{
      \item \code{$name} parameter name (character) 
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

Workset is a working set of model parameters and can be a full set, 
which include values of all model parameters or subset and include only some parameters.

Each model must have "default" workset. 
Default workset is a first workset of the model with set_id = min(set_id) for that model. 
Default workset always include ALL model parameters (it is a full set).

If you want to create new workset as a full set of model parameters
then you must pass ALL model parameters into \code{createWorkset} through \dots argument list. 

If you already have result of model run in your database 
and want to modify only some input parameters (subset) then call \code{\link{createWorksetBasedOnRun}} 
in order to create workset using parameters from previous model run and supply some new values. 

You can create subset of model parameters ONLY based on existing run results. 
Otherwise you have to pass ALL (full set) of parameters in order to create workset.

Each workset has unique set id (positive integer) and also may have a name. 
To find set id by name use \code{\link{getWorksetIdByName}} call.

Working set must be read-only to run the model, so, typically you want to call \code{\link{setReadonlyWorkset}}
after \code{createWorksetBasedOnRun}.

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
To run examples you must have modelOne database modelOne.sqlite in current directory.
It must contain run_id = 11 and 12 (see runId usage below)
}

\seealso{
  \code{\link{getModel}}
  \code{\link{getFirstRunId}}
  \code{\link{getDefaultWorksetId}}
  \code{\link{getWorksetIdByName}}
  \code{\link{createWorkset}}
  \code{\link{copyWorksetParameterFromRun}}
  \code{\link{setReadonlyWorkset}}
  \code{\link{setReadonlyDefaultWorkset}}
  \code{\link{updateWorksetParameter}}
}
\examples{
  #
  # model parameters:
  #   age by sex parameter double[4, 2] 
  #   salary by age parameter int[3, 4]
  #   starting seed parameter integer value
  #
  
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
  # name, description and notes for this set of model parameters
  #
  paramSetTxt <- data.frame(
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
  # pass new value for salaryAge parameter
  # for all other parameters use previous model run with id = 11 
  #
  setId <- createWorksetBasedOnRun(theDb, defRs, 11L, paramSetTxt, salaryAge)
  if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)
  
  #
  # copy ageSex parameter value from model run with id = 12
  #
  copyWorksetParameterFromRun(theDb, defRs, setId, 12L, list(name = "ageSex"))
  
  # make workset read-only in order to run the model
  setReadonlyWorkset(theDb, defRs, TRUE, setId)
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new salaryAge parameter value and ageSex value from other run results
  #
}
\keyword{ OpenM++ }
\keyword{ database }
