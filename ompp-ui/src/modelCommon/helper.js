// db structures common functions: helpers

export const RUN_OF_RUNSET = 'run'
export const SET_OF_RUNSET = 'set'

// return date-time string: truncate timestamp
export const dtStr = (ts) => {
  return (ts || '').substr(0, 19)
}

// return true if argument has length > 0
export const isLength = (arr) => {
  return arr && (arr.length || 0) > 0
}

// return argument.length or zero if no length
export const lengthOf = (arr) => {
  return isLength(arr) ? (arr.length || 0) : 0
}
