\name{getModel}
\alias{getModel}
\title{
Find OpenM++ model definition in database
}
\description{
Find model in database and return OpenM++ model definition (input parameters and output tables)
}
\usage{
getModel(dbCon, modelName, modelDigest = NA)
}
\arguments{
  \item{dbCon}{
    database connection
  }
  \item{modelName}{
    model name, ie: "modelOne"
  }
  \item{modelDigest}{
    (optional) model digest, ie: "20120817_1604590148"
    if digest is NA or NULL then first model with min(model_id) is used
  }
}
\details{
Find model in database and return OpenM++ model definition (input parameters and output tables).

Model can be identified by by name and model digest string, which serves as model version. 
If you have only one version of the model in database then you can omit digest argument. 
If there are multiple models with same name in database and digest is NA or NULL then first model with min(model_id) is used.
}
\value{
  List of database rows describing model parameters:
  \item{modelDic}{ model_dic row }
  \item{langLst}{ list of languages (lang_lst table rows) }
  \item{typeDic}{ type_dic rows of the model }
  \item{typeEnum}{ type_enum_lst rows of the model, optional, can be empty }
  \item{paramDic}{ parameter_dic rows of the model }
  \item{paramDims}{ parameter_dims rows of the model, optional, can be empty }
  \item{tableDic}{ table_dic rows of the model }
  \item{tableDims}{ table_dims rows of the model, optional, can be empty }
  \item{tableAcc}{ table_acc rows of the model }
  \item{tableExpr}{ table_expr rows of the model }
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
  \code{\link{getLanguages}}
}
\examples{
  theDb <- dbConnect(RSQLite::SQLite(), "modelOne.sqlite", synchronous = "full")
  invisible(dbGetQuery(theDb, "PRAGMA busy_timeout = 86400")) # recommended
  
  # find the model: get model definition from database
  #
  # model can be found by name and (optional) digest
  # digest is used to identify exact version of the model 
  # if digest is missing or NA or NULL 
  # and there are multiple versions of the model with the same name
  # then first model with min(model_id) is used
  #
  
  # get model by name: use such call if you have only one version of the model
  defRs <- getModel(theDb, "modelOne")
  
  # use model digest to identify exact version of the model
  def2012August17 <- getModel(theDb, "modelOne", "20120817_1604590148")
  
  dbDisconnect(theDb)
}
\keyword{ OpenM++ }
\keyword{ database }
