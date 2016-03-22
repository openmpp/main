\name{setTaskWaitCompleted}
\alias{setTaskWaitCompleted}
\title{
Set modeling task "wait completed" status
}
\description{
Set modeling task "wait completed" status
}
\usage{
setTaskWaitCompleted(dbCon, taskRunId, isWaitCompleted = FALSE)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{taskRunId}{
    id of modeling task run
  }
  \item{isWaitCompleted}{
    if true then signal to running model "task is ready to be completed"
  }
}

\details{
Using this \code{setTaskWaitCompleted} call to signal currently running model "task is ready to be completed".

Task run status can be one of:
\itemize{
  \item \code{i} = not yet started
  \item \code{p} = run in progress
  \item \code{w} = run in progress, wait for additional input
  \item \code{s} = completed successfully
  \item \code{e} = failed (error)
  \item \code{x} = reserved
}
  
Status \code{w} means task can be dynamically updated by external script.
Model executable is waiting for additional input into or for "ready to be completed" signal.
Use \code{setTaskWaitCompleted} function to mark task as "ready to be completed".
Use \code{updateTask} function to insert additional task input.

Model executable must be running with \code{-OpenM.TaskWait true} argument in order to handle dynamical task input.
For example:

  \code{modelOne -OpenM.TaskName taskOne -OpenM.TaskWait true}
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
\keyword{ OpenM++ }
\keyword{ database }
