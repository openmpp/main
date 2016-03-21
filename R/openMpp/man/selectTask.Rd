\name{selectTask}
\alias{selectTask}
\title{
Select modeling task
}
\description{
Select modeling task text (name, description, notes) and input workset id's
}
\usage{
selectTask(dbCon, taskId)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{taskId}{
    id of modeling task
  }
}
\details{
This call return task text (name, description, notes) and input workset id's.
}
\value{
  List of database rows:
  \item{taskLst}{ task_lst row: task id and name }
  \item{taskTxt}{ task_txt rows: language, description, notes }
  \item{taskSet}{ task_set rows: task input workset id's }
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
  \code{\link{selectTaskList}}
  \code{\link{selectTaskRun}}
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
  
  # select task name, description and input list
  taskRs <- selectTask(theDb, taskId)
  
  # select list of all tasks for the model
  taskLstRs <- selectTaskList(theDb, defRs)
  
  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
