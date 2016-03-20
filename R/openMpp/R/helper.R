##
## Copyright (c) 2014 OpenM++
## This is a free software licensed under MIT license
##

#
# return quoted string: O'Connor -> 'O''Connor'
#
toQuoted <- function(srcStr) 
{
  paste(
    "'", gsub("'", "''", srcStr, fixed = TRUE), "'", 
    sep = ""
  )
}

# 
# For internal use only
#
# Validate text parameter (workset text, task text)
#   stop if invalid, return FALSE if empty, return TRUE if any data
#
# i_msgPart - used in error messages
# i_langRs  - lang_lst table rows
# i_txt     - (optional) text data frame:
#   $name   - name (workset name, task name)
#   $lang   - language code
#   $descr  - working set description
#   $note   - (optional) working set notes
#
validateTxtFrame <- function(i_msgPart, i_langRs, i_txt)
{
  # validate data frame itself, exit if empty
  if (missing(i_txt)) return(FALSE)
  if (is.null(i_txt) || is.na(i_txt)) return(FALSE)
  if (!is.data.frame(i_txt)) stop(i_msgPart, " must be a data frame")
  if (nrow(i_txt) <= 0L) return(FALSE)
  
  # text frame must have $name, $lang, $descr, $note column
  if (is.null(i_txt$"name") || is.null(i_txt$"lang") || 
      is.null(i_txt$"descr") || is.null(i_txt$"note")) {
    stop(i_msgPart, " must have $name, $lang, $descr, $note columns")
  }
  
  # language code must NOT NULL and in the lang_lst table
  if (any(is.na(i_txt$"lang"))) {
    stop(i_msgPart, " must have $lang NOT NULL")
  }
  if (!all(i_txt$"lang" %in% i_langRs$lang_code)) {
    stop("invalid (or empty) language of ", i_msgPart)
  }

  # description must NOT NULL
  if (any(is.na(i_txt$"descr"))) {
    stop(i_msgPart, " must have $descr NOT NULL")
  }
  
  return(TRUE)  # valid and not empty
}
