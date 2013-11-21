\name{getParameterDefs}
\alias{getParameterDefs}
\title{
Return OpenM++ model parameters definition
}
\description{
Return OpenM++ model parameters definition
}
\usage{
getParameterDefs(dbCon, modelName, modelTimestamp = NA)
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
}
\details{
Model can be identified by by name and model timestamp string, which serves as model version. 
If you have only one version of the model in database then you can omit timestamp argument. 
In that case MIN(timestamp) will be used.
}
\value{
  List of database rows describing model parameters
  \item{modelDic}{ model_dic row }
  \item{typeDic}{ type_dic rows of the model }
  \item{typeEnum}{ type_enum_lst rows of the model, optional, can be empty }
  \item{paramDic}{ parameter_dic rows of the model }
  \item{paramDims}{ parameter_dims rows of the model, optional, can be empty }
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

  defRs <- getParameterDefs(theDb, "modelOne", "_201208171604590148_")

  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
