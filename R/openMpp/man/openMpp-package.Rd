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
Version: \tab 0.4\cr
Date: \tab 2014-09-17\cr
License: \tab MIT license\cr
}
  \describe{
    \item{createWorkset}{
      Create new working set of model parameters
    }
    \item{createWorksetBasedOnRun}{
      Create new working set of model parameters based on parameters from existing model run 
    }
    \item{copyWorksetParameterFromRun}{
      Copy parameters to working set from existing model run
    }
    \item{getLanguages}{
      Return language rows from lang_lst table
    }
    \item{getModel}{
      Find OpenM++ model definition in database
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
    \item{setReadonlyDefaultWorkset}{
      Set or clear read-only status of default working set
    }
    \item{setReadonlyWorkset}{
      Set or clear read-only status for default working set of model parameters
    }
    \item{toQuoted}{
      Return SQL-quoted string: O'Connor -> 'O''Connor'
    }
    \item{updateWorksetParameter}{
      Update parameters working set with new values and value notes
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
  \code{\link{createWorkset}}
  \code{\link{createWorksetBasedOnRun}}
  \code{\link{copyWorksetParameterFromRun}}
  \code{\link{getLanguages}}
  \code{\link{getModel}}
  \code{\link{getWorksetIdByName}}
  \code{\link{getWorksetRunIds}}
  \code{\link{selectRunAccumulator}}
  \code{\link{selectRunOutputValue}}
  \code{\link{selectRunParameter}}
  \code{\link{setReadonlyDefaultWorkset}}
  \code{\link{setReadonlyWorkset}}
  \code{\link{toQuoted}}
  \code{\link{updateWorksetParameter}}
}
\examples{
}
