\name{createWorkset}
\alias{createWorkset}
\title{
Create new working set of model parameters
}
\description{
Create new workset FULL set of model parameters, it must include ALL model parameters 
}
\usage{
createWorkset(dbCon, defRs, setDef, ...)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{defRs}{
    model definition: database rows describing model input parameters and output tables
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
That call allow you to create new working set of input parameters which contains ALL model parameters.

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

Each workset has unique set id (positive integer) and unique name. 
To find set id by name use \code{\link{getWorksetIdByName}} call.

Working set must be read-only to run the model, so, typically you want to call \code{\link{setReadonlyWorkset}}
after \code{createWorkset}.

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
  \code{\link{getDefaultWorksetId}}
  \code{\link{getWorksetIdByName}}
  \code{\link{createWorksetBasedOnRun}}
  \code{\link{copyWorksetParameterFromRun}}
  \code{\link{setReadonlyWorkset}}
  \code{\link{setReadonlyDefaultWorkset}}
  \code{\link{updateWorksetParameter}}
}
\examples{
  #
  # model parameters:
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
  # name MUST be unique
  #
  inputSet <- data.frame(
    name = "myData",
    lang = c("EN", "FR"),
    descr = c("full set of parameters", "description in French"),
    note = c("full set of parameters notes", NA),
    stringsAsFactors = FALSE
  )
  
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended
  
  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")
  
  # create new workset with ALL model parameters
  setId <- createWorkset(theDb, defRs, inputSet, ageSex, salaryAge, salaryFull, baseSalary, startingSeed, filePath)
  if (setId <= 0L) stop("workset creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)
  
  # make workset read-only in order to run the model
  setReadonlyWorkset(theDb, defRs, TRUE, setId)
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new parameters
  #
}
\keyword{ OpenM++ }
\keyword{ database }
