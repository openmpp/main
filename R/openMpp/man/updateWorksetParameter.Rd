\name{updateWorksetParameter}
\alias{updateWorksetParameter}
\title{
Update parameters working set with new values and value notes
}
\description{
Update parameters working set with new values and value notes
}
\usage{
updateWorksetParameter(dbCon, defRs, worksetId, ...)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{defRs}{
    model definition: database rows describing model input parameters and output tables
  }
  \item{worksetId}{
    id of parameters working set, must be positive integer
  }
  \item{\dots}{
    list of parameters value and value notes.
    
    Each element is also a list of $name, $value and $txt:
    \itemize{
      \item \code{$name} parameter name (character) 
      \item \code{$value} (optional) parameter value
        it can be scalar value. vector or data frame
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
That call allow you to update input parameter value and value notes in specific workset.

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

Working set must be not read-only in order to update parameters by \code{\link{updateWorksetParameter}}.
And working set must be read-only to run the model, so, typically you want to wrap \code{\link{updateWorksetParameter}}
by \code{setReadonlyDefaultWorkset} or \code{setReadonlyWorkset} calls.

You must use \code{getModel} function in order to find model definition \code{defRs}.
}
\value{
  Return id of working set or 0L on error
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
  \code{\link{createWorkset}}
  \code{\link{createWorksetBasedOnRun}}
  \code{\link{copyWorksetParameterFromRun}}
  \code{\link{setReadonlyWorkset}}
  \code{\link{setReadonlyDefaultWorkset}}
}
\examples{
  #
  # model parameters:
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
  
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended
  
  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")
  
  # reset read-only status of workset  
  setId <- 3L
  if (setReadonlyWorkset(theDb, defRs, FALSE, setId) <= 0L) {
    stop("workset not found: ", setId, " for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)
  }
  
  # update parameter ageSex with new value and value notes
  updateWorksetParameter(theDb, defRs, setId, ageSex)
  
  # make workset read-only in order to run the model
  setReadonlyWorkset(theDb, defRs, TRUE, setId)
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new ageSex parameter value
  #
}
\keyword{ OpenM++ }
\keyword{ database }
