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
To run examples you must have modelOne database m1.sqlite in your home directory
}

\seealso{
  \code{\link{getModel}}
}
\examples{
  theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  langRs <- getLanguages(theDb)

  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
