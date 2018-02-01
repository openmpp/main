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

// format date-time to timestamp string: YYYY-MM-DD hh:mm:ss.0SSS
export const dtToTimeStamp = (dt) => {
  if (!dt) return ''
  let month = dt.getMonth() + 1
  let day = dt.getDate()
  let hour = dt.getHours()
  let minute = dt.getMinutes()
  let sec = dt.getSeconds()
  let ms = dt.getMilliseconds()
  //
  return dt.getFullYear().toString() + '-' +
    (month < 10 ? '0' + month.toString() : month.toString()) + '-' +
    (day < 10 ? '0' + day.toString() : day.toString()) + ' ' +
    (hour < 10 ? '0' + hour.toString() : hour.toString()) + ':' +
    (minute < 10 ? '0' + minute.toString() : minute.toString()) + ':' +
    (sec < 10 ? '0' + sec.toString() : sec.toString()) + '.' +
    ('0000' + ms.toString()).slice(-4)
}

// format date-time to timestamp string: YYYY_MM_DD_hh_mm_ss_0SSS
export const dtToUnderscoreTimeStamp = (dt) => {
  if (!dt) return ''
  let month = dt.getMonth() + 1
  let day = dt.getDate()
  let hour = dt.getHours()
  let minute = dt.getMinutes()
  let sec = dt.getSeconds()
  let ms = dt.getMilliseconds()
  //
  return dt.getFullYear().toString() + '_' +
    (month < 10 ? '0' + month.toString() : month.toString()) + '_' +
    (day < 10 ? '0' + day.toString() : day.toString()) + '_' +
    (hour < 10 ? '0' + hour.toString() : hour.toString()) + '_' +
    (minute < 10 ? '0' + minute.toString() : minute.toString()) + '_' +
    (sec < 10 ? '0' + sec.toString() : sec.toString()) + '_' +
    ('0000' + ms.toString()).slice(-4)
}
