\name{openMpp-package}
\alias{openMpp-package}
\alias{openMpp}
\docType{package}
\title{
Read and write OpenM++ database from R
}
\description{ 
  At the moment this package only insert or update input parameters for OpenM++ models.
}
\details{
\tabular{ll}{
Package: \tab openMpp\cr
Type: \tab Package\cr
Version: \tab 0.1\cr
Date: \tab 2013-11-20\cr
License: \tab MIT license\cr
}
  \describe{
    \item{createWorkset}{
      Create new working set of model parameters
    }
    \item{createWorksetBasedOnRun}{
      Create new working set of model parameters based on parameters from existing model run 
    }
    \item{updateWorksetParameter}{
      Update parameters working set with new values and value notes
    }
    \item{setReadonlyDefaultWorkset}{
      Set or clear read-only status of default working set
    }
    \item{setReadonlyWorkset}{
      Set or clear read-only status for default working set of model parameters
    }
    \item{getLanguageRs}{
      Return language rows from lang_lst table
    }
    \item{getParameterDefs}{
      Return OpenM++ model parameters definition
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
  \code{\link{createWorkset}}
  \code{\link{createWorksetBasedOnRun}}
  \code{\link{updateWorksetParameter}}
  \code{\link{setReadonlyDefaultWorkset}}
  \code{\link{setReadonlyWorkset}}
  \code{\link{getLanguageRs}}
  \code{\link{getParameterDefs}}
  \code{\link{toQuoted}}
}
\examples{
}
