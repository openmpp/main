\name{setReadonlyWorkset}
\alias{setReadonlyWorkset}
\title{
Set or clear read-only status of model parameters working set
}
\description{
Set or clear read-only status of model parameters working set
}
\usage{
setReadonlyWorkset(dbCon, defRs, isReadonly, worksetId)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{defRs}{
    model definition: database rows describing model input parameters and output tables
  }
  \item{isReadonly}{
    if TRUE then mark working set as read-only else clear read-only status
  }
  \item{worksetId}{
    id of parameters working set, must be positive integer
  }
}
\details{
Workset must be NOT read-only in order to update parameters by \code{\link{updateWorksetParameter}}.
And workset must be read-only to run the model, so, typically you want to wrap \code{\link{updateWorksetParameter}}
by \code{setReadonlyDefaultWorkset} or \code{setReadonlyWorkset} calls.

Workset is a working set of model parameters and can be a full set, 
which include values of all model parameters or subset and include only some parameters.

Each model must have "default" workset. 
Default workset is a first workset of the model with set_id = min(set_id) for that model. 
Default workset always include ALL model parameters (it is a full set).

You must use \code{getModel} function in order to find model definition \code{defRs}.
}
\value{
  Return working set id or 0L if not found
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
  \code{\link{copyWorksetParameterFromRun}}
  \code{\link{setReadonlyDefaultWorkset}}
  \code{\link{updateWorksetParameter}}
}
\examples{
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended

  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")
  
  # reset read-only status of workset  
  setId <- 3L
  if (setReadonlyWorkset(theDb, defRs, FALSE, setId) <= 0L) {
    stop("workset not found: ", setId, " for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)
  }
  
  # you can update model parameters now:
  #
  # updateWorksetParameter(theDb, defRs, setId, ageSex)
  
  # make workset read-only in order to run the model
  setReadonlyWorkset(theDb, defRs, TRUE, setId)
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new parameter values in workset id = 3
  #
}
\keyword{ OpenM++ }
\keyword{ database }
