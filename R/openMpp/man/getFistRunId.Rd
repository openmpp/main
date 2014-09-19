\name{getFirstRunId}
\alias{getFirstRunId}
\title{
Return id of first model run results
}
\description{
Return id of first model run results
}
\usage{
getFirstRunId(dbCon, defRs)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{defRs}{
    model definition: database rows describing model input parameters and output tables
  }
}
\details{
Return first model run results id and it is positive integer.
If model does not have any run results (model never executed) then return negative value.
}
\value{
  Run id: positive integer on success or negative on if not found.
}
\references{
OpenM++ documentation: http://ompp.sourceforge.net/wiki/
}
\author{
amc1999
}
\note{
To run examples you must have modelOne database modelOne.sqlite in your home directory
}

\seealso{
  \code{\link{getModel}}
  \code{\link{getWorksetRunIds}}
}
\examples{
  theDb <- dbConnect("SQLite", "~/modelOne.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")

  # get first run id of that model
  runId <- getFirstRunId(theDb, defRs)
  if (runId <= 0L) warning("model run results not found: ", "myData")

  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
