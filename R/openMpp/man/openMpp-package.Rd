\name{openMpp-package}
\alias{openMpp-package}
\alias{openMpp}
\docType{package}
\title{
Read and write OpenM++ database from R
}
\description{ 
  Set input parameters and get output results of OpenM++ models using R.
}
\details{
\tabular{ll}{
Package: \tab openMpp\cr
Type: \tab Package\cr
Version: \tab 0.8\cr
Date: \tab 2016-12-27\cr
License: \tab MIT license\cr
}
  \describe{
    \item{createTask}{
      Create new modeling task
    }
    \item{createWorkset}{
      Create new working set of model parameters
    }
    \item{createWorksetBasedOnRun}{
      Create new working set of model parameters based on parameters from existing model run 
    }
    \item{copyWorksetParameterFromRun}{
      Copy parameters to working set from existing model run
    }
    \item{getDefaultWorksetId}{
      Return default working set id
    }
    \item{getFirstRunId}{
      Return id of first model run results
    }
    \item{getLastRunId}{
      Return id of last model run results
    }
    \item{getTaskFirstRunId}{
      Return first id of modeling task run
    }
    \item{getTaskLastRunId}{
      Return last id of modeling task run
    }
    \item{getLanguages}{
      Return language rows from lang_lst table
    }
    \item{getModel}{
      Find OpenM++ model definition in database
    }
    \item{getTaskIdByName}{
      Return modeling task id by name
    }
    \item{getWorksetIdByName}{
      Return working set id by name
    }
    \item{getWorksetRunIds}{
      Return ids of model run results for specified working set
    }
    \item{selectRunAccumulator}{
      Select output table accumulator values from model run result
    }
    \item{selectRunOutputValue}{
      Select output table expression values from model run result
    }
    \item{selectRunParameter}{
      Select parameter values from model run result
    }
    \item{selectTask}{
      Select modeling task text (name, description, notes) and input workset id's
    }
    \item{selectTaskList}{
      Select list of modeling tasks for the model
    }
    \item{selectTaskRun}{
      Select modeling task run: status and results
    }
    \item{selectTaskRunList}{
      Select list of modeling task runs
    }
    \item{setReadonlyDefaultWorkset}{
      Set or clear read-only status of default working set
    }
    \item{setReadonlyWorkset}{
      Set or clear read-only status for default working set of model parameters
    }
    \item{setTaskWaitCompleted}{
      Set modeling task "wait completed" status
    }
    \item{updateTask}{
      Update modeling task with new text (name, description, notes) or additional input working sets
    }
    \item{updateWorksetParameter}{
      Update parameters working set with new values and value notes
    }
    \item{toQuoted}{
      Return SQL-quoted string: O'Connor -> 'O''Connor'
    }
  }
}
\author{
amc1999

Maintainer: amc1999 <openmpp99@gmail.com>
}
\references{
OpenM++ documentation: http://ompp.sourceforge.net/wiki/
}
\keyword{ OpenM++ }
\keyword{ database }
\seealso{
  \code{\link{createTask}}
  \code{\link{createWorkset}}
  \code{\link{createWorksetBasedOnRun}}
  \code{\link{copyWorksetParameterFromRun}}
  \code{\link{getDefaultWorksetId}}
  \code{\link{getFirstRunId}}
  \code{\link{getLastRunId}}
  \code{\link{getLanguages}}
  \code{\link{getModel}}
  \code{\link{getTaskIdByName}}
  \code{\link{getTaskFirstRunId}}
  \code{\link{getTaskLastRunId}}
  \code{\link{getWorksetIdByName}}
  \code{\link{getWorksetRunIds}}
  \code{\link{selectRunAccumulator}}
  \code{\link{selectRunOutputValue}}
  \code{\link{selectRunParameter}}
  \code{\link{selectTask}}
  \code{\link{selectTaskList}}
  \code{\link{selectTaskRun}}
  \code{\link{selectTaskRunList}}
  \code{\link{setReadonlyDefaultWorkset}}
  \code{\link{setReadonlyWorkset}}
  \code{\link{setTaskWaitCompleted}}
  \code{\link{updateTask}}
  \code{\link{updateWorksetParameter}}
  \code{\link{toQuoted}}
}
\examples{
}
