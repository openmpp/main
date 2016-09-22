\name{selectRunAccumulator}
\alias{selectRunAccumulator}
\title{
Select output table accumulator values from model run result
}
\description{
Select output table accumulator values from model run result
}
\usage{
selectRunAccumulator(dbCon, defRs, runId, tableName, accName = NA)
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
  \item{accName}{
    (optional) output table accumulator name, i.e.: "acc0"
    if missing or NA then return all accumulators
  }
}
\details{
That call return values of output table accumulators from model run results with id \code{runId}.

You can get either value of single accumulator by specifying it name, i.e.: \code{"acc2"} 
or all accumulators if \code{accName} argument is missing.

You must use \code{getModel} function in order to find model definition \code{defRs}.
}
\value{
  Data frame of database rows with output table accumulator(s) dimensions and values:
  \item{acc_id}{ accumulator number }
  \item{sub_id}{ subsample number }
  \item{dim0,...,dimN}{ dimension items enum ids (not returned if rank is zero) }
  \item{acc_value}{ accumulator value }
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
  \code{\link{getFirstRunId}}
  \code{\link{getLastRunId}}
  \code{\link{selectRunOutputValue}}
  \code{\link{selectRunParameter}}
}
\examples{
  #
  # connect to database
  #
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended
  
  # get definition for "modelOne" model
  #
  # use model name only to identify the model
  # if there are multiple versions of the model in database 
  # then first one is selected: where model id = min(model_id) for "modelOne"
  #
  defRs <- getModel(theDb, "modelOne")
  
  # get first run id of that model
  runId <- getFirstRunId(theDb, defRs)
  if (runId <= 0L) stop("model run results not found")
  
  # select accumulator(s) value of "salarySex" from model run results
  #  
  allAccValueRs <- selectRunAccumulator(theDb, defRs, runId, "salarySex")
  acc0_ValueRs <- selectRunAccumulator(theDb, defRs, runId, "salarySex", "acc0")
  
  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
