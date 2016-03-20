\name{getLanguages}
\alias{getLanguages}
\title{
Return list of languages
}
\description{
Return list of languages: rows from lang_lst table
}
\usage{
getLanguages(dbCon)
}
\arguments{
  \item{dbCon}{
    database connection
  }
}
\details{
Read from database and return data frame of language list (lang_lst) table rows.
}
\value{
  Data frame with $lang_id, $lang_code, $lang_name columns of lang_lst table
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
}
\examples{
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended

  langRs <- getLanguages(theDb)

  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
