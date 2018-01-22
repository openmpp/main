// db structures common functions: workset and workset list
// workset is input set of model parameters

// workset count: number of worksettext entries in workset text list
export const worksetTextCount = (wtl) => {
  return isWorksetTextList(wtl) ? wtl.length : 0
}

// if this is workset text
export const isWorksetText = (wt) => {
  if (!wt) return false
  if (!wt.hasOwnProperty('ModelName') || !wt.hasOwnProperty('ModelDigest')) return false
  if (!wt.hasOwnProperty('Name')) return false
  if (!wt.hasOwnProperty('IsReadonly') || !wt.hasOwnProperty('BaseRunDigest')) return false
  if (!wt.hasOwnProperty('Param') || !wt.hasOwnProperty('Txt')) return false
  if (!wt.Param.hasOwnProperty('length') || !wt.Txt.hasOwnProperty('length')) return false
  return true
}

// if this is not empty workset text: model name, digest and workset name not empty
export const isNotEmptyWorksetText = (wt) => {
  if (!isWorksetText(wt)) return false
  return (wt.ModelName || '') !== '' && (wt.ModelDigest || '') !== '' && (wt.Name || '') !== ''
}

// return empty workset text
export const emptyWorksetText = () => {
  return {
    ModelName: '',
    ModelDigest: '',
    Name: '',
    IsReadonly: false,
    BaseRunDigest: '',
    Param: [],
    Txt: []
  }
}

// return true if each list element isWorksetText()
export const isWorksetTextList = (wtl) => {
  if (!wtl) return false
  if (!wtl.hasOwnProperty('length')) return false
  for (let k = 0; k < wtl.length; k++) {
    if (!isWorksetText(wtl[k])) return false
  }
  return true
}
