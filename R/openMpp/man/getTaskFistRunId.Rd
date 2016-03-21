\name{getTaskFirstRunId}
\alias{getTaskFirstRunId}
\title{
Return first id of modeling task run
}
\description{
Return first id of modeling task run
}
\usage{
getTaskFirstRunId(dbCon, taskId)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{taskId}{
    modeling task id
  }
}
\details{
Return id of the first run for modeling task, it is positive integer.
If there is no run(s) for that modeling task then return negative value.
}
\value{
  Task run id: positive integer on success or negative on if not found.
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
  \code{\link{getTaskLastRunId}}
  \code{\link{getModel}}
  \code{\link{getWorksetRunIds}}
}
\examples{
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended

  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")
  
  # get task id of "taskOne"
  taskId <- getTaskIdByName(theDb, defRs, "taskOne")
  if (firstId <= 0L) warning("task run(s) not found, task id: ", taskId)

  # get first task run id
  firstId <- getTaskFirstRunId(theDb, taskId)
  if (firstId <= 0L) stop("task run(s) not found")

  # get last (most recent) task run id
  lastId <- getTaskLastRunId(theDb, taskId)
  if (lastId <= 0L) warning("task run(s) not found")
  
  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
