\name{getLanguageRs}
\alias{getLanguageRs}
\title{
Return language rows from lang_lst table
}
\description{
Return language rows from lang_lst table
}
\usage{
getLanguageRs(dbCon)
}
\arguments{
  \item{dbCon}{
    database connection
}
}
\details{
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
}
\examples{
  theDb <- dbConnect("SQLite", "~/m1.sqlite", synchronous = 2)
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400"))

  langRs <- getLanguageRs(theDb)

  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
