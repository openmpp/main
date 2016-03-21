\name{selectTaskRun}
\alias{selectTaskRun}
\title{
Select task run
}
\description{
Select modeling task run: status and results
}
\usage{
selectTaskRun(dbCon, taskRunId)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{taskRunId}{
    id of modeling task run
  }
}
\details{
This call return task run status, time and id's of input working sets and output results.
}
\value{
  List of database rows:
  \item{taskRunLst}{ task_run_lst row: task run id, status and time }
  \item{taskRunSet}{ task_run_set rows: input workset id's and result run id's }
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
  \code{\link{getTaskIdByName}}
  \code{\link{getTaskFirstRunId}}
  \code{\link{getTaskLastRunId}}
  \code{\link{selectTask}}
  \code{\link{selectTaskList}}
  \code{\link{selectTaskRunList}}
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
  # then first one is selected: where timestamp = min(timestamp) for "modelOne"
  #
  defRs <- getModel(theDb, "modelOne")
  
  taskId <- getTaskIdByName(theDb, defRs, "taskOne")
  if (taskId <= 0L) stop("task: ", "taskOne", " not found for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)
  
  # last (most recent) run of that task
  lastId <- getTaskLastRunId(theDb, taskId)
  if (lastId <= 0L) stop("task run(s) not found, task: ", taskId)
  
  # select task run status, input and output
  taskRunRs <- selectTaskRun(theDb, lastId)
  
  # select list of all task runs
  taskRunLstRs <- selectTaskRunList(theDb, taskId)
  
  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
