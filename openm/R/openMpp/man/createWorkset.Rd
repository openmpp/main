\name{createWorkset}
\alias{createWorkset}
\title{
Create new working set of model parameters
}
\description{
Create new working set of model parameters
}
\usage{
createWorkset(dbCon, modelName, modelTimestamp, worksetTxt, ...)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{modelName}{
    model name, ie: "modelOne"
  }
  \item{modelTimestamp}{
    model timestamp, ie: "_201208171604590148_"
    if timestamp is NA or NULL then used MIN(timestamp)
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
Model can be identified by by name and model timestamp string, which serves as model version. 
If you have only one version of the model in database then you can omit timestamp argument. 
In that case MIN(timestamp) will be used.

Working set must contain ALL model parameters and you must pass it to \code{createWorkset} 
through \dots argument list. If you already have result of model run in your database 
and want to modify only few input parameters then call \code{\link{createWorksetBasedOnRun}} 
to create working set using parameters from previous model run and supply some new values. 

Working set must be read-only to run the model, so, typically you want to call \code{\link{setReadonlyWorkset}}
after \code{createWorkset}.
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
To run examples you must have modelOne database m1.sqlite in your home directory
}

\seealso{
  \code{\link{createWorksetBasedOnRun}}
  \code{\link{updateWorksetParameter}}
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
    lang = c("EN", "FR"),
    descr = c("full set of parameters", "description in French"),
    note = c("full set of parameters notes", NA),
    stringsAsFactors = FALSE
  )

  theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  setId <- createWorkset(theDb, "modelOne", NA, paramSetTxt, ageSex, salaryAge)
  if (setId <= 0L) stop("workset creation failed: ", modelName)
  
  setReadonlyWorkset(theDb, TRUE, setId)
  
  dbDisconnect(theDb)
  #
  # you can run the model now with new parameters
  #
}
\keyword{ OpenM++ }
\keyword{ database }
