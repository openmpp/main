\name{createWorksetBasedOnRun}
\alias{createWorksetBasedOnRun}
\title{
Create new working set of model parameters
}
\description{
Create new working set of model parameters
}
\usage{
createWorksetBasedOnRun(dbCon, baseRunId, worksetTxt, ...)
}

\arguments{
  \item{dbCon}{
    database connection
  }
  \item{baseRunId}{
    id of model run results, must be positive integer
  }
  \item{worksetTxt}{
    if not NA then workset text data frame:
    \itemize{
      \item \code{$lang} language code 
      \item \code{$descr} working set description 
      \item \code{$note} (optional) working set notes 
    }
  }
  \item{\dots}{
    list of parameters value and (optional) value notes.
    
    Each element is also a list of $name, $value and $txt:
    \itemize{
      \item \code{$name} parameter name (character) 
      \item \code{$value} parameter value
        it can be scalar value, vector or data frame
        size of $value must be equal to production of dimension sizes
        if data frame then 
          it must have $dim0, $dim1,..., $value columns
          and each column length is equal to production of dimension sizes
      \item \code{$txt} (optional) workset parameter text:
        data frame with $lang = language code and $note = value notes
    }
  }
}

\details{
That call allow you to create new working set of model parameters as combination of 
existing parameters from previous model run and some new parameters passed through \dots argument(s)

Working set must be read-only to run the model, so, typically you want to call \code{\link{setReadonlyWorkset}}
after \code{createWorksetBasedOnRun}.
}
\value{
  Return id of new working set or 0L if not found
}
\references{
OpenM++ documentation: http://ompp.sourceforge.net/wiki/
}
\author{
amc1999
}
\note{
To run examples you must have modelOne database m1.sqlite in your home directory.
It must contain run_id = 16 (see runId usage below)
}

\seealso{
  \code{\link{createWorkset}}
  \code{\link{updateWorksetParameter}}
  \code{\link{setReadonlyWorkset}}
}
\examples{
  #
  # model parameters:
  #   age by sex parameter double[4, 2] 
  #   salary by age parameter int[3, 4]
  #
  
  # salary by age parameter value and notes
  salaryAge <- list(

    name = "salaryAge",

    txt = data.frame(
      lang = c("EN", "FR"),
      note = c("salary by age value notes", "FR salary by age value notes"),
      stringsAsFactors = FALSE
    ),
    value = c(
      100L,
      rep(c(10L, 20L, 30L), times = 3),
      200L,
      300L
    )
  )
  
  #
  # description and notes for this set of model parameters
  #
  paramSetTxt <- data.frame(
    lang = "EN",
    descr = "new set of parameters",
    note = "new set of parameters with updated salary by age",
    stringsAsFactors = FALSE
  )

  theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  #
  # pass new value for salaryAge parameter
  # for all other parameters use previous model run with id = 16 
  #
  setId <- createWorksetBasedOnRun(theDb, 16L, NA, salaryAge)
  if (setId <= 0L) stop("workset creation failed: ", modelName)
  
  setReadonlyWorkset(theDb, TRUE, setId)
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new salaryAge parameter value
  #
}
\keyword{ OpenM++ }
\keyword{ database }
