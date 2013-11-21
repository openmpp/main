\name{updateWorksetParameter}
\alias{updateWorksetParameter}
\title{
Update parameters working set with new values and value notes
}
\description{
Update parameters working set with new values and value notes
}
\usage{
updateWorksetParameter(dbCon, worksetId, ...)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{worksetId}{
    id of parameters working set, must be positive integer
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
Working set must be not read-only in order to update parameters by \code{\link{updateWorksetParameter}}.
And working set must be read-only to run the model, so, typically you want to wrap \code{\link{updateWorksetParameter}}
by \code{setReadonlyDefaultWorkset} or \code{setReadonlyWorkset} calls.
}
\value{
  Return id of working set or 0L if not found
}
\references{
OpenM++ documentation: http://ompp.sourceforge.net/wiki/
}
\author{
amc1999
}
\note{
To run examples you must have modelOne database m1.sqlite in your home directory
}

\seealso{
  \code{\link{createWorkset}}
  \code{\link{createWorksetBasedOnRun}}
  \code{\link{setReadonlyWorkset}}
}
\examples{
  #
  # model parameters:
  #   age by sex parameter double[4, 2] 
  #   salary by age parameter int[3, 4]
  #
  
  # age by sex parameter value and notes
  ageSex <- list(
  
    name = "ageSex",  # parameter name
  
    txt = data.frame(
      lang = c("EN", "FR"),
      note = c(
        "age by sex value notes", # EN value notes
        NA                        # NA == no FR value notes 
      ),
      stringsAsFactors = FALSE
    ),
    
    value = c(
      10,
      rep(c(1, 2, 3), times = 2),
      20
    )
  )
  
  theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  setId <- 3L
  if (setReadonlyWorkset(theDb, FALSE, setId) != setId) stop("workset not found: ", setId)

  updateWorksetParameter(theDb, setId, ageSex)
  setReadonlyWorkset(theDb, TRUE, setId)
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new ageSex parameter value
  #
}
\keyword{ OpenM++ }
\keyword{ database }
