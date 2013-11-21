\name{setReadonlyDefaultWorkset}
\alias{setReadonlyDefaultWorkset}
\title{
Set or clear read-only status for default working set of model parameters
}
\description{
Set or clear read-only status for default working set of model parameters
}
\usage{
setReadonlyDefaultWorkset(dbCon, isReadonly, modelName, modelTimestamp = NULL)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{isReadonly}{
    if TRUE then mark default working set as read-only else clear read-only status
  }
  \item{modelName}{
    model name, ie: "modelOne"
  }
  \item{modelTimestamp}{
    model timestamp, ie: "_201208171604590148_"
    if timestamp is NA or NULL then used MIN(timestamp)
  }
}
\details{
Working set must be not read-only in order to update parameters by \code{\link{updateWorksetParameter}}.
And working set must be read-only to run the model, so, typically you want to wrap \code{\link{updateWorksetParameter}}
by \code{setReadonlyDefaultWorkset} or \code{setReadonlyWorkset} calls.

Default working set of model parameters is a first working set, with MIN(set_id). It always contains all model parameters.

Model can be identified by by name and model timestamp string, which serves as model version. 
If you have only one version of the model in database then you can omit timestamp argument. 
In that case MIN(timestamp) will be used.
}
\value{
  Return working set id of default workset or 0L if not found
}
\references{
OpenM++ documentation: http://ompp.sourceforge.net/wiki/
}
\author{
amc1999
}
\note{
To run examples you must have modelOne database m1.sqlite in your home directory
}

\seealso{
  \code{\link{updateWorksetParameter}}
  \code{\link{setReadonlyWorkset}}
}
\examples{
  theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  setId <- setReadonlyDefaultWorkset(theDb, FALSE, "modelOne")
  if (setId <= 0L) stop("no any worksets exists for modelOne")

  # you can update model parameters now:
  #
  # updateWorksetParameter(theDb, setId, ageSex, salaryAge)
  
  setReadonlyDefaultWorkset(theDb, TRUE, "modelOne")
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new parameters in default workset
  #
}
\keyword{ OpenM++ }
\keyword{ database }
