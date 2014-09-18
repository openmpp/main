\name{getDefaultWorksetId}
\alias{getDefaultWorksetId}
\title{
Return default working set id
}
\description{
Return default working set id for the model
}
\usage{
getDefaultWorksetId(dbCon, defRs)
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
Return id of model "default" working set of parameters. 

If model have no default working set then execution stops.

Each model must have "default" workset. 
Default workset is a first workset of the model with set_id = min(set_id) for that model. 
}
\value{
  Working set id: positive integer on success or negative on error. Stop if not found.
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
  \code{\link{getWorksetRunIds}}
}
\examples{
  theDb <- dbConnect("SQLite", "~/modelOne.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")

  # get default working set id by name, it expected to be exist
  setId <- getDefaultWorksetId(theDb, defRs)
  
  # try to find "myData" working set id
  setId <- getWorksetIdByName(theDb, defRs, "myData")
  if (setId <= 0L) warning("workset not found: ", "myData")

  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
