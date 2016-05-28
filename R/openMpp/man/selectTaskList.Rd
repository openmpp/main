\name{selectTaskList}
\alias{selectTaskList}
\title{
Select list of modeling tasks
}
\description{
Select list of modeling tasks for the model
}
\usage{
selectTaskList(dbCon, defRs)
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
This call return list of tasks for specified model.
}
\value{
  List of database rows:
  \item{taskLst}{ task_lst rows: task id and name }
  \item{taskTxt}{ task_txt rows: language, description, notes }
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
  # then first one is selected: where model id = min(model_id) for "modelOne"
  #
  defRs <- getModel(theDb, "modelOne")
  
  taskId <- getTaskIdByName(theDb, defRs, "taskOne")
  if (taskId <= 0L) stop("task: ", "taskOne", " not found for model: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_digest)
  
  # select task name, description and input list
  taskRs <- selectTaskList(theDb, taskId)
  
  # select list of all tasks for the model
  taskLstRs <- selectTaskList(theDb, defRs)
  
  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
