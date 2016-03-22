\name{getTaskIdByName}
\alias{getTaskIdByName}
\title{
Return modeling task id by name
}
\description{
Return modeling task id by name
}
\usage{
getTaskIdByName(dbCon, defRs = NA, taskName)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{defRs}{
    (optional) model definition: database rows describing model input parameters and output tables
  }
  \item{taskName}{
    name of modeling task
  }
}
\details{
Return modeling task id by name. 
If there is no task with that name then return is negative.
If there are multiple tasks with that name then return min(task id) with such name.
If model definition \code{defRs} argument supplied then search for within specific model.

Modeling task is named set of model inputs and contains name and vector of model workset id's. 
Please see \code{\link{createWorkset}} for mode details about workset (workset: working set of model input parameters).

Modeling task is a convinient way to bundle together multiple inputs of the model. 
After task created you can run the model with specifing task name or task id 
and model will iterate through task input worksets and produce output result for each of those inputs.
}
\value{
  modeling task id: positive integer on success or negative if not found.
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
  \code{\link{createTask}}
  \code{\link{selectTask}}
  \code{\link{selectTaskRun}}
}
\examples{
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended

  # try to find "taskOne" task id for any model
  taskId <- getTaskIdByName(theDb, taskName = "taskOne")
  if (taskId <= 0L) warning("task not found: ", "taskOne")
  
  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")

  # try to find "taskOne" task id for "modelOne"
  taskId <- getTaskIdByName(theDb, defRs, "taskOne")
  if (taskId <= 0L) warning("task not found: ", "taskOne")

  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
