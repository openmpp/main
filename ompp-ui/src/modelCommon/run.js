// db structures common functions: model run and run list

// run count: number of run text entries in the run text list
export const runTextCount = (rtl) => {
  return isRunTextList(rtl) ? rtl.length : 0
}

// return true if each list element isRunText()
export const isRunTextList = (rtl) => {
  if (!rtl) return false
  if (!rtl.hasOwnProperty('length')) return false
  for (let k = 0; k < rtl.length; k++) {
    if (!isRunText(rtl[k])) return false
  }
  return true
}

// if this is run text
export const isRunText = (rt) => {
  if (!rt) return false
  if (!rt.hasOwnProperty('ModelName') || !rt.hasOwnProperty('ModelDigest')) return false
  if (!rt.hasOwnProperty('Name') || !rt.hasOwnProperty('Digest')) return false
  if (!rt.hasOwnProperty('SubCount') || !rt.hasOwnProperty('Status')) return false
  if (!rt.hasOwnProperty('Param') || !rt.hasOwnProperty('Txt')) return false
  if (!rt.Param.hasOwnProperty('length') || !rt.Txt.hasOwnProperty('length')) return false
  return true
}

// if this is not empty run text: model run text name and status not empty
export const isNotEmptyRunText = (rt) => {
  if (!isRunText(rt)) return false
  return (rt.ModelName || '') !== '' && (rt.ModelDigest || '') !== '' &&
    (rt.Name || '') !== '' && (rt.Status || '') !== '' && (rt.SubCount || 0) !== 0
}

// return empty run text
export const emptyRunText = () => {
  return {
    ModelName: '',
    ModelDigest: '',
    Name: '',
    Digest: '',
    SubCount: 0,
    Status: '',
    Param: [],
    Txt: []
  }
}

// retrun true if run completed successfuly
export const isRunSuccess = (rt) => {
  return isRunText(rt) && rt.Status === RUN_SUCCESS
}

export const RUN_SUCCESS = 's'      // run completed successfuly
export const RUN_IN_PROGRESS = 'p'  // run in progress
export const RUN_INITIAL = 'i'      // run not started yet
export const RUN_FAILED = 'e'       // run falied (comleted with error)
export const RUN_EXIT = 'x'         // run exit and not completed

// return run status description by code: i=init p=progress s=success x=exit e=error(failed)
export const statusText = (rt) => {
  switch ((rt.Status || '')) {
    case RUN_SUCCESS: return 'success'
    case RUN_IN_PROGRESS: return 'in progress'
    case RUN_INITIAL: return 'not yet started'
    case RUN_FAILED: return 'failed'
    case RUN_EXIT: return 'exit (not completed)'
  }
  return 'unknown'
}
