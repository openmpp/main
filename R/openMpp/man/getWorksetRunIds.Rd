\name{getWorksetRunIds}
\alias{getWorksetRunIds}
\title{
Return ids of model run results for specified working set
}
\description{
Return ids of model run results where input parameters are from specified working set.
}
\usage{
getWorksetRunIds(dbCon, worksetId)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{worksetId}{
    id of parameters working set, must be positive integer
  }
}
\details{
Return ids of model run results where input parameters are from specified working set.

Please keep in mind: there is no established link in database between input data working set and model run results.
Input data can be modified or even deleted after model run. 
If you really want to find input parameters values for specific model run then \code{selectRunParameter} should be used.

It is recommended to create modeling task by \code{createTask} 
to include multiple working sets of input parameters and run the model using such input.
}
\value{
  Data frame with integer $run_id column
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
  \code{\link{createTask}}
  \code{\link{getModel}}
  \code{\link{getFirstRunId}}
  \code{\link{getDefaultWorksetId}}
  \code{\link{getWorksetIdByName}}
  \code{\link{selectRunParameter}}
}
\examples{
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended

  runIdRs <- getWorksetRunIds(theDb, 2L)

  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
