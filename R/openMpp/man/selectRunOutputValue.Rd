\name{selectRunOutputValue}
\alias{selectRunOutputValue}
\title{
Select output table expression values from model run result
}
\description{
Select output table expression values from model run result
}
\usage{
selectRunOutputValue(dbCon, defRs, runId, tableName, exprName = NA)
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
  \item{tableName}{
    output table name, i.e.: "sexAge"
  }
  \item{exprName}{
    (optional) output table expression name, i.e.: "expr0"
    if missing or NA then return all expressions
  }
}
\details{
That call return values of output table expressions from model run results with id \code{runId}.

You can get either value of single expression by specifying it name, i.e.: \code{"expr2"} 
or all expressions if \code{exprName} argument is missing.

You must use \code{getModel} function in order to find model definition \code{defRs}.
}
\value{
  Data frame of database rows with output table expression(s) dimensions and values:
  \item{dim0,...,dimN}{ dimension items enum ids (not returned if rank is zero) }
  \item{expr_id}{ expression number (zero based) }
  \item{value}{ output table expression(s) value }
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
  \code{\link{selectRunParameter}}
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
  
  # select output table expression(s) value of "salarySex" from model run results with id = 11L
  #  
  runId <- 11L
  
  expr2_ValueRs <- selectRunOutputValue(theDb, defRs, runId, "salarySex", "expr2")
  allExprValueRs <- selectRunOutputValue(theDb, defRs, runId, "salarySex")
  
  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
