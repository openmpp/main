\name{selectRunParameter}
\alias{selectRunParameter}
\title{
Select parameter values from model run result
}
\description{
Select parameter values from model run result
}
\usage{
selectRunParameter(dbCon, defRs, runId, paramName)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{defRs}{
    model definition: database rows describing model input parameters and output tables
  }
  \item{runId}{
    id of model run results, must be positive integer
  }
  \item{paramName}{
    parameter name, i.e.: "sexAge"
  }
}
\details{
That call return values of input parameter from model run results with id \code{runId}.

You must use \code{getModel} function in order to find model definition \code{defRs}.
}
\value{
  Data frame of database rows with parameter dimensions and values:
  \item{dim0,...,dimN}{ dimension items enum ids (not returned if rank is zero) }
  \item{value}{ parameter value }
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
  \code{\link{getFirstRunId}}
  \code{\link{selectRunAccumulator}}
  \code{\link{selectRunOutputValue}}
}
\examples{
  #
  # connect to database
  #
  theDb <- dbConnect("SQLite", "~/modelOne.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))
  
  # get definition for "modelOne" model
  #
  # use model name only to identify the model
  # if there are multiple versions of the model in database 
  # then first one is selected: where timestamp = min(timestamp) for "modelOne"
  #
  defRs <- getModel(theDb, "modelOne")
  
  # select parameter "ageSex" value from model run results with id = 11L
  #  
  runId <- 11L
  
  paramValueRs <- selectRunParameter(theDb, defRs, runId, "ageSex")
  
  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
