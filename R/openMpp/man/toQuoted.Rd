\name{toQuoted}
\alias{toQuoted}
\title{
  Return SQL-quoted string
}
\description{
  Return SQL-quoted string: O'Connor -> 'O''Connor'
}
\usage{
  toQuoted(srcStr)
}
\arguments{
  \item{srcStr}{
  \code{srcStr} source string
}
}
\details{
  Duplicate each apostrophe and enclose result in 'apostrophes'
}
\value{
  Return SQL-quoted string: O'Connor -> 'O''Connor'
}
\references{
OpenM++ documentation: http://ompp.sourceforge.net/wiki/
}
\author{
amc1999
}
\note{
}
\seealso{
http://ompp.sourceforge.net/wiki/
}
\examples{
someName <- toQuoted("O'Connor")
# someName
# [1] "'O''Connor'"

sql <- paste(
  "SELECT * FROM someTable WHERE name = ", toQuoted("O'Connor"), sep=""
  )

# sql
# [1] "SELECT * FROM someTable WHERE name = 'O''Connor'"
}
\keyword{ OpenM++ }
\keyword{ database }
