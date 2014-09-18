\name{getWorksetRunIds}
\alias{getWorksetRunIds}
\title{
Return ids of model run results for specified working set
}
\description{
Return ids of model run results where input parameters are from specified working set
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
To run examples you must have modelOne database modelOne.sqlite in your home directory
}

\seealso{
  \code{\link{getModel}}
  \code{\link{getWorksetIdByName}}
}
\examples{
  theDb <- dbConnect("SQLite", "~/modelOne.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  runIdRs <- getWorksetRunIds(theDb, 2L)

  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
