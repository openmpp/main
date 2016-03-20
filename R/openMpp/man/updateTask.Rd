\name{updateTask}
\alias{updateTask}
\title{
Update modeling task
}
\description{
Update modeling task with new text (name, description, notes) or additional input working sets
}
\usage{
updateTask(dbCon, defRs, taskId, taskTxt = NA, setIds = NA)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{defRs}{
    model definition: database rows describing model input parameters and output tables
  }
  \item{taskId}{
    modeling task id, must already exist
  }
  \item{taskTxt}{
    if not NA then modeling task text data frame:
    \itemize{
      \item \code{$name} (optional) modeling task name
      \item \code{$lang} language code 
      \item \code{$descr} modeling task description 
      \item \code{$note} (optional) modeling task notes 
    }
  }
  \item{setIds}{
    if not NA then modeling task inputs:
      vector of integer id's of workset (workset: working set of model input parameters)
    
    It is possible to supply single integer workset id or data frame with $set_id vector
  }
}

\details{
Using this \code{updateTask} call you can update existing modeling task with new text data (name, description, or notes)
and/or insert additional input working set(s) into this task.

Modeling task is named set of model inputs and contains name and vector of model workset id's. 
Please see \code{\link{createWorkset}} for mode details about workset (workset: working set of model input parameters).

Modeling task is a convinient way to bundle together multiple inputs of the model. 
After task created you can run the model with specifing task name or task id 
and model will iterate through task input worksets and produce output result for each of those inputs.
}
\value{
  Return task id or 0L on error
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
  \code{\link{createTask}}
}
\examples{
  #
  # name, description and notes for new modeling task
  #
  myTaskTxt <- data.frame(
    name = "myTask",
    lang = c("EN", "FR"),
    descr = c("my first modeling task", "description in French"),
    note = c("this is a test task and include two model input data sets with id 2 and 4", NA),
    stringsAsFactors = FALSE
  )
  
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended
  
  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")
  
  # create new task, initially empty
  taskId <- createTask(theDb, defRs)
  if (taskId <= 0L) stop("task creation failed: ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)
  
  # update task with new text (name, description, notes) and two input data set_id = c(2, 4)
  taskId <- updateTask(theDb, defRs, taskId, myTaskTxt, c(2, 4))
  if (taskId <= 0L) stop("task update failed, id: ", taskId, ", ", defRs$modelDic$model_name, " ", defRs$modelDic$model_ts)

  dbDisconnect(theDb)
  #
  # you can run the model now with new  modeling task:
  #   modelOne -OpenM.TaskName myTask
  #
}
\keyword{ OpenM++ }
\keyword{ database }
