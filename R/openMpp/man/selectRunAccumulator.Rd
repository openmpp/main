\name{selectRunAccumulator}
\alias{selectRunAccumulator}
\title{
Select output table accumulator values from model run result
}
\description{
Select output table accumulator values from model run result
}
\usage{
selectRunAccumulator(dbCon, defRs, runId, tableName, accName = NA, subId = NA)
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
  \item{subId}{
    (optional) subsample number (zero based)
    if missing or NA then return all subsamples
  }
}
\details{
That call return values of output table accumulators from model run results with id \code{runId}.

You can get either value of single accumulator by specifying it name, i.e.: \code{"acc2"} 
or all accumulators if \code{accName} argument is missing.

You can get either accumulator values of specific subsample number \code{subId} 
or all subsamples if \code{subId} argument is missing.

You must use \code{getModel} function in order to find model definition \code{defRs}.
}
\value{
  Data frame of database rows with output table accumulator(s) dimensions and values:
  \item{dim0,...,dimN}{ dimension items enum ids (not returned if rank is zero) }
  \item{sub_id}{ subsample number }
  \item{acc0,...,accM}{ accumulator(s) value }
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
  \code{\link{getModel}}
  \code{\link{selectRunOutputValue}}
  \code{\link{selectRunParameter}}
}
\examples{
  #
  # connect to database
  #
  theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))
  
  # get definition for "modelOne" model
  #
  # use model name only to identify the model
  # if there are multiple versions of the model in database 
  # then first one is selected: where timestamp = min(timestamp) for "modelOne"
  #
  defRs <- getModel(theDb, "modelOne")
  
  # select accumulator(s) value of "salarySex" from model run results with id = 11L
  #  
  runId <- 11L
  
  allAccValueAllSubRs <- selectRunAccumulator(theDb, defRs, runId, "salarySex")
  acc0_ValueAllSubRs <- selectRunAccumulator(theDb, defRs, runId, "salarySex", "acc0")
  acc0_ValueRs <- selectRunAccumulator(theDb, defRs, runId, "salarySex", "acc0", 0L)
  allAccValue_sub0_Rs <- selectRunAccumulator(theDb, defRs, runId, "salarySex", NA, 0L)
  
  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
