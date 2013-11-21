\name{setReadonlyWorkset}
\alias{setReadonlyWorkset}
\title{
Set or clear read-only status of parameters working set by workset id
}
\description{
Set or clear read-only status of parameters working set by workset id
}
\usage{
setReadonlyWorkset(dbCon, isReadonly, worksetId)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{isReadonly}{
    if TRUE then mark working set as read-only else clear read-only status
  }
  \item{worksetId}{
    id of parameters working set, must be positive integer
  }
}
\details{
Working set must be not read-only in order to update parameters by \code{\link{updateWorksetParameter}}.
And working set must be read-only to run the model, so, typically you want to wrap \code{\link{updateWorksetParameter}}
by \code{setReadonlyDefaultWorkset} or \code{setReadonlyWorkset} calls.
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
To run examples you must have modelOne database m1.sqlite in your home directory
}

\seealso{
  \code{\link{updateWorksetParameter}}
  \code{\link{setReadonlyDefaultWorkset}}
}
\examples{
  theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  setId <- 3L
  if (setReadonlyWorkset(theDb, FALSE, setId) <= 0L) stop("workset not found: ", setId)
  
  # you can update model parameters now:
  #
  # updateWorksetParameter(theDb, setId, ageSex)
  
  setReadonlyWorkset(theDb, TRUE, setId)
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new parameters in workset id = 3
  #
}
\keyword{ OpenM++ }
\keyword{ database }
