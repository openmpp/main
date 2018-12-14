// db structures common functions: parameter and parameter list

import * as Mdl from './model'
import * as Tdf from './type'
import * as Hlpr from './helper'

// number of model parameters
export const paramCount = (md) => {
  return isParamTextList(md) ? md.ParamTxt.length : 0
}

// is model has parameter text list and each element is Param
export const isParamTextList = (md) => {
  if (!Mdl.isModel(md)) return false
  if (!md.hasOwnProperty('ParamTxt')) return false
  if (!Hlpr.hasLength(md.ParamTxt)) return false
  for (let k = 0; k < md.ParamTxt.length; k++) {
    if (!isParam(md.ParamTxt[k].Param)) return false
  }
  return true
}

// return true if this is non empty Param
export const isParam = (p) => {
  if (!p) return false
  if (!p.hasOwnProperty('ParamId') || !p.hasOwnProperty('Name') || !p.hasOwnProperty('Digest')) return false
  return (p.Name || '') !== '' && (p.Digest || '') !== ''
}

// return empty ParamTxt
export const emptyParamText = () => {
  return {
    Param: {
      ParamId: 0,
      Name: '',
      Digest: ''
    },
    DescrNote: {
      LangCode: '',
      Descr: '',
      Note: ''
    }
  }
}

// find ParamTxt by name
export const paramTextByName = (md, name) => {
  if (!Mdl.isModel(md) || (name || '') === '') { // model empty or name empty: return empty result
    return emptyParamText()
  }
  for (let k = 0; k < md.ParamTxt.length; k++) {
    if (!isParam(md.ParamTxt[k].Param)) continue
    if (md.ParamTxt[k].Param.Name === name) return md.ParamTxt[k]
  }
  return emptyParamText() // not found
}

// return empty parameter size
export const emptyParamSize = () => {
  return {
    rank: 0,
    dimTotal: 0,
    dimSize: []
  }
}

// find parameter size by name: number of parameter rows
export const paramSizeByName = (md, name) => {
  // if this is not a parameter then size =empty value else rowCount at least =1
  let ret = emptyParamSize()
  const p = paramTextByName(md, name)
  if (!isParam(p.Param)) return ret

  // parameter rank must be same as dimension count
  if (p.hasOwnProperty('ParamDimsTxt')) {
    if ((p.Param.Rank || 0) !== (p.ParamDimsTxt.length || 0)) return ret
    //
    ret.rank = (p.Param.Rank || 0)
  }

  // multiply all dimension sizes, assume =1 for built-in types
  ret.dimTotal = 1
  for (let k = 0; k < p.ParamDimsTxt.length; k++) {
    if (!p.ParamDimsTxt[k].hasOwnProperty('Dim')) {
      ret = emptyParamSize()
      return ret
    }
    let n = Tdf.typeEnumSizeById(md, p.ParamDimsTxt[k].Dim.TypeId)
    ret.dimSize.push(n || 0)
    if (ret.dimSize[k] > 0) ret.dimTotal *= ret.dimSize[k]
  }
  return ret
}

// return true if this is non empty ParamRunSet
export const isParamRunSet = (prs) => {
  if (!prs) return false
  if (!prs.hasOwnProperty('Name') || !prs.hasOwnProperty('SubCount') || !Hlpr.hasLength(prs.Txt)) return false
  return (prs.Name || '') !== ''
}

// return empty ParamRunSetPub, which is Param[i] of run text and workset text
export const emptyParamRunSet = () => {
  return {
    Name: '',
    SubCount: 0,
    Txt: []
  }
}

// find ParamRunSet by parameter name in Param[] array of run text or workset text
// return empty value if not found
export const paramRunSetByName = (rsl, name) => {
  if (!rsl || !name) return emptyParamRunSet()
  if (!rsl.hasOwnProperty('Param')) return emptyParamRunSet()
  if (!Hlpr.isLength(rsl.Param)) return emptyParamRunSet()
  for (let k = 0; k < rsl.Param.length; k++) {
    if (isParamRunSet(rsl.Param[k]) && rsl.Param[k].Name === name) {
      return JSON.parse(JSON.stringify(rsl.Param[k]))
    }
  }
  return emptyParamRunSet() // not found
}
