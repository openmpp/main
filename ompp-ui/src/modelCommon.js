// model common functions mixin

const ModelCommon = {
  // if this is a Model with Name and Digest properties
  hasNameDigest (md) {
    if (!md) return false
    if (!md.hasOwnProperty('Model')) return false
    return md.Model.hasOwnProperty('Name') && md.Model.hasOwnProperty('Digest')
  },

  // if this is a model
  isModel (md) {
    if (!this.hasNameDigest(md)) return false
    return (md.Model.Name || '') !== '' && (md.Model.Digest || '') !== ''
  },

  // if this is an empty model: model with empty name and digest
  isEmptyModel (md) {
    if (!this.hasNameDigest(md)) return false
    return (md.Model.Name || '') === '' || (md.Model.Digest || '') === ''
  },

  // return empty Model
  emptyModel () { return { Model: { Name: '', Digest: '' } } },

  // return true if each list element isModel()
  isModelList (ml) {
    if (!ml) return false
    if (!ml.hasOwnProperty('length')) return false
    for (let k = 0; k < ml.length; k++) {
      if (!this.isModel(ml[k])) return false
    }
    return true
  },

  // number of model parameters
  paramCount (md) { return this.isParamTextList(md) ? md.ParamTxt.length : 0 },

  // number of model output tables
  outTableCount (md) { return this.isTableTextList(md) ? md.TableTxt.length : 0 },

  // run count: number of run text entries
  runTextCount (rtl) { return this.isRunTextList(rtl) ? rtl.length : 0 },

  // workset count: number of worksettext entries
  worksetTextCount (wtl) { return this.isWorksetTextList(wtl) ? wtl.length : 0 },

  // name of the model
  modelName (md) {
    if (!md) return ''
    if (!md.hasOwnProperty('Model')) return ''
    return (md.Model.Name || '')
  },

  // digest of the model
  modelDigest (md) {
    if (!md) return ''
    if (!md.hasOwnProperty('Model')) return ''
    return (md.Model.Digest || '')
  },

  // make model title
  modelTitle (md) {
    if (!this.isModel(md)) return ''
    const descr = this.descrOfDescrNote(md)
    if (descr !== '') return md.Model.Name + ': ' + descr
    return md.Model.Name
  },

  // return date-time string: truncate timestamp
  dtStr (ts) { return (ts || '').substr(0, 19) },

  // description if object has DescrNote
  descrOfDescrNote (tdn) {
    if (!tdn) return ''
    if (!tdn.hasOwnProperty('DescrNote')) return ''
    return (tdn.DescrNote.Descr || '')
  },

  // notes if object has DescrNote
  noteOfDescrNote (tdn) {
    if (!tdn) return ''
    if (!tdn.hasOwnProperty('DescrNote')) return ''
    return (tdn.DescrNote.Note || '')
  },

  // return true if notes of DescrNote not empty
  isNoteOfDescrNote (tdn) {
    if (!tdn) return false
    if (!tdn.hasOwnProperty('DescrNote')) return false
    return (tdn.DescrNote.Note || '') !== ''
  },

  // description if object has Txt[0] description-notes
  descrOfTxt (tdn) {
    if (!tdn) return ''
    if (!tdn.hasOwnProperty('Txt')) return ''
    if (!tdn.Txt.hasOwnProperty('length')) return ''
    return (tdn.Txt.length > 0) ? (tdn.Txt[0].Descr || '') : ''
  },

  // notes if object has Txt[0] description-notes
  noteOfTxt (tdn) {
    if (!tdn) return ''
    if (!tdn.hasOwnProperty('Txt')) return ''
    if (!tdn.Txt.hasOwnProperty('length')) return ''
    return (tdn.Txt.length > 0) ? (tdn.Txt[0].Note || '') : ''
  },

  // return true if notes of Txt[0] not empty
  isNoteOfTxt (tdn) {
    if (!tdn) return false
    if (!tdn.hasOwnProperty('Txt')) return false
    if (!tdn.Txt.hasOwnProperty('length')) return false
    return (tdn.Txt.length > 0) ? (tdn.Txt[0].Note || '') !== '' : false
  },

  // hard coded enum code for total enum id
  ALL_WORD_CODE: 'all',

  // return empty language-specific model words list
  emptyWordList () {
    return { ModelName: '', ModelDigest: '', LangCode: '', LangWords: [], ModelLangCode: '', ModelWords: [] }
  },

  // find label by code in language-specific model words list, return code if labe not found
  wordByCode (mw, code) {
    if (!code) return ''
    if (!mw) return code
    // seacrh in model-specific list of words
    if (mw.ModelWords && (mw.ModelWords.length || 0) > 0) {
      for (let k = 0; k < mw.ModelWords.length; k++) {
        if (mw.ModelWords[k].Code === code) return mw.ModelWords[k].Label
      }
    }
    // seacrh in common language-specific list of words
    if (mw.LangWords && (mw.LangWords.length || 0) > 0) {
      for (let k = 0; k < mw.LangWords.length; k++) {
        if (mw.LangWords[k].Code === code) return mw.LangWords[k].Label
      }
    }
    return code   // not found: return original code
  },

  // is model has type text list and each element is TypeTxt
  isTypeTextList (md) {
    if (!this.isModel(md)) return false
    if (!md.hasOwnProperty('TypeTxt')) return false
    if (!md.TypeTxt.hasOwnProperty('length')) return false
    for (let k = 0; k < md.TypeTxt.length; k++) {
      if (!this.isType(md.TypeTxt[k].Type)) return false
    }
    return true
  },

  // return true if this is non empty Type
  isType (t) {
    if (!t) return false
    if (!t.hasOwnProperty('TypeId') || !t.hasOwnProperty('Name') || !t.hasOwnProperty('Digest')) return false
    return (t.Name || '') !== '' && (t.Digest || '') !== ''
  },

  // return true if argument has length > 0
  isLength (arr) { return arr && (arr.length || 0) > 0 },

  // return empty TypeTxt
  emptyTypeText () {
    return {
      Type: { TypeId: 0, Name: '', Digest: '' },
      DescrNote: { LangCode: '', Descr: '', Note: '' },
      TypeEnumTxt: [] }
  },

  // find TypeTxt by TypeId
  typeTextById (md, typeId) {
    if (!this.isModel(md) || typeId === void 0 || typeId === null) {  // model empty or type id empty: return empty result
      return this.emptyTypeText()
    }
    for (let k = 0; k < md.TypeTxt.length; k++) {
      if (!this.isType(md.TypeTxt[k].Type)) continue
      if (md.TypeTxt[k].Type.TypeId === typeId) return md.TypeTxt[k]
    }
    return this.emptyTypeText()  // not found
  },

  // find type size by TypeId: TypeTxt.TypeEnumTxt.length
  typeEnumSizeById (md, typeId) {
    const t = this.typeTextById(md, typeId)
    if (!this.isType(t.Type)) return 0
    if (!t.hasOwnProperty('TypeEnumTxt')) return 0
    return t.TypeEnumTxt.length || 0
  },

  // return true if this is non empty Enum
  isEnum (t) {
    if (!t || !t.Enum) return false
    return !(t.Enum.EnumId === void 0 || t.Enum.EnumId === null)
  },

  // find enum code by enum id or empty string if not found
  enumCodeById (typeTxt, enumId) {
    if (!typeTxt || !this.isLength(typeTxt.TypeEnumTxt)) return ''
    for (let k = 0; k < typeTxt.TypeEnumTxt.length; k++) {
      if (this.isEnum(typeTxt.TypeEnumTxt[k]) && typeTxt.TypeEnumTxt[k].Enum.EnumId === enumId) {
        return (typeTxt.TypeEnumTxt[k].Enum.Name || '')
      }
    }
    return ''   // not found
  },

  // find enum description or code by enum id or empty string if not found
  enumDescrOrCodeById (typeTxt, enumId) {
    if (!typeTxt || !this.isLength(typeTxt.TypeEnumTxt)) return ''
    for (let k = 0; k < typeTxt.TypeEnumTxt.length; k++) {
      if (this.isEnum(typeTxt.TypeEnumTxt[k]) && typeTxt.TypeEnumTxt[k].Enum.EnumId === enumId) {
        return this.descrOfDescrNote(typeTxt.TypeEnumTxt[k]) || (typeTxt.TypeEnumTxt[k].Enum.Name || '')
      }
    }
    return ''   // not found
  },

  // is model has parameter text list and each element is Param
  isParamTextList (md) {
    if (!this.isModel(md)) return false
    if (!md.hasOwnProperty('ParamTxt')) return false
    if (!md.ParamTxt.hasOwnProperty('length')) return false
    for (let k = 0; k < md.ParamTxt.length; k++) {
      if (!this.isParam(md.ParamTxt[k].Param)) return false
    }
    return true
  },

  // return true if this is non empty Param
  isParam (p) {
    if (!p) return false
    if (!p.hasOwnProperty('ParamId') || !p.hasOwnProperty('Name') || !p.hasOwnProperty('Digest')) return false
    return (p.Name || '') !== '' && (p.Digest || '') !== ''
  },

  // return empty ParamTxt
  emptyParamText () {
    return {
      Param: { ParamId: 0, Name: '', Digest: '' },
      DescrNote: { LangCode: '', Descr: '', Note: '' } }
  },

  // find ParamTxt by name
  paramTextByName (md, name) {
    if (!this.isModel(md) || (name || '') === '') { // model empty or name empty: return empty result
      return this.emptyParamText()
    }
    for (let k = 0; k < md.ParamTxt.length; k++) {
      if (!this.isParam(md.ParamTxt[k].Param)) continue
      if (md.ParamTxt[k].Param.Name === name) return md.ParamTxt[k]
    }
    return this.emptyParamText()  // not found
  },

  // find parameter size by name: number of parameter rows
  paramSizeByName (md, name) {
    // if this is not a parameter then size =0 else at least =1
    const p = this.paramTextByName(md, name)
    if (!this.isParam(p.Param)) return 0

    // zero rank parameter
    if (!p.hasOwnProperty('ParamDimsTxt') || !p.ParamDimsTxt.hasOwnProperty('length')) return 1

    // multiply all dimension sizes, assume =1 for built-in types
    let size = 1
    for (let k = 0; k < p.ParamDimsTxt.length; k++) {
      if (!p.ParamDimsTxt[k].hasOwnProperty('Dim')) return 0
      let n = this.typeEnumSizeById(md, p.ParamDimsTxt[k].Dim.TypeId)
      if ((n || 0) > 0) size *= n
    }
    return size
  },

  // is model has output table text list and each element is Table
  isTableTextList (md) {
    if (!this.isModel(md)) return false
    if (!md.hasOwnProperty('TableTxt')) return false
    if (!md.TableTxt.hasOwnProperty('length')) return false
    for (let k = 0; k < md.TableTxt.length; k++) {
      if (!this.isTable(md.TableTxt[k].Table)) return false
    }
    return true
  },

  // return true if this is non empty Table
  isTable (t) {
    if (!t) return false
    if (!t.hasOwnProperty('TableId') || !t.hasOwnProperty('Name') || !t.hasOwnProperty('Digest')) return false
    return (t.Name || '') !== '' && (t.Digest || '') !== ''
  },

  // return empty TableTxt
  emptyTableText () {
    return {
      Table: { TableId: 0, Name: '', Digest: '' },
      TableDimsTxt: [],
      TableExprTxt: [],
      TableAccTxt: [],
      LangCode: '',
      TableDescr: '',
      TableNote: '',
      ExprDescr: '',
      ExprNote: ''
    }
  },

  // find output TableTxt by name
  tableTextByName (md, name) {
    if (!this.isModel(md) || (name || '') === '') { // model empty or name empty: return empty result
      return this.emptyTableText()
    }
    for (let k = 0; k < md.TableTxt.length; k++) {
      if (!this.isTable(md.TableTxt[k].Table)) continue
      if (md.TableTxt[k].Table.Name === name) return md.TableTxt[k]
    }
    return this.emptyTableText()  // not found
  },

  // find output table size by name or empty value
  tableSizeByName (md, name) {
    // if this is not output table then size =empty value
    let ret = this.emptyTableSize()
    const t = this.tableTextByName(md, name)
    if (!this.isTable(t.Table)) return ret

    // count of dimensions, expressions and accumulatotrs: table rank must be same as dimension count
    if (t.hasOwnProperty('TableDimsTxt')) {
      if ((t.Table.Rank || 0) !== (t.TableDimsTxt.length || 0)) return ret
      ret.rank = (t.Table.Rank || 0)
    }
    if (t.hasOwnProperty('TableExprTxt')) ret.exprCount = (t.TableExprTxt.length || 0)
    if (t.hasOwnProperty('TableAccTxt')) ret.allAccCount = (t.TableAccTxt.length || 0)
    if (ret.allAccCount > 0) {
      ret.accCount = ret.allAccCount
      for (let k = 0; k < ret.allAccCount; k++) {
        if (ret.accCount > 0 && t.TableAccTxt[k].hasOwnProperty('Acc') && !!t.TableAccTxt[k].Acc.IsDerived) ret.accCount--
      }
    }

    // multiply all dimension sizes, include "total" item, assume =1 for built-in types
    ret.dimTotal = 1
    for (let k = 0; k < t.TableDimsTxt.length; k++) {
      if (!t.TableDimsTxt[k].hasOwnProperty('Dim')) return 0
      ret.dimSize.push(t.TableDimsTxt[k].Dim.DimSize || 0)
      if (ret.dimSize[k] > 0) ret.dimTotal *= ret.dimSize[k]
    }
    return ret
  },

  // return empty table size
  emptyTableSize () {
    return {rank: 0, exprCount: 0, accCount: 0, allAccCount: 0, dimTotal: 0, dimSize: []}
  },

  // if this is run text
  isRunText (rt) {
    if (!rt) return false
    if (!rt.hasOwnProperty('ModelName') || !rt.hasOwnProperty('ModelDigest')) return false
    if (!rt.hasOwnProperty('Name') || !rt.hasOwnProperty('Digest')) return false
    if (!rt.hasOwnProperty('SubCount') || !rt.hasOwnProperty('Status')) return false
    if (!rt.hasOwnProperty('Txt')) return false
    if (!rt.Txt.hasOwnProperty('length')) return false
    return true
  },

  // if this is not empty run text: model run text name and satus not empty
  isNotEmptyRunText (rt) {
    if (!this.isRunText(rt)) return false
    return (rt.ModelName || '') !== '' && (rt.ModelDigest || '') !== '' &&
      (rt.Name || '') !== '' && (rt.Status || '') !== ''
  },

  // return empty run text
  emptyRunText () {
    return {
      ModelName: '', ModelDigest: '', Name: '', Digest: '', SubCount: 0, Status: '', Txt: []
    }
  },

  // return true if each list element isRunText()
  isRunTextList (rtl) {
    if (!rtl) return false
    if (!rtl.hasOwnProperty('length')) return false
    for (let k = 0; k < rtl.length; k++) {
      if (!this.isRunText(rtl[k])) return false
    }
    return true
  },

  // if this is workset text
  isWorksetText (wt) {
    if (!wt) return false
    if (!wt.hasOwnProperty('ModelName') || !wt.hasOwnProperty('ModelDigest')) return false
    if (!wt.hasOwnProperty('Name')) return false
    if (!wt.hasOwnProperty('IsReadonly') || !wt.hasOwnProperty('BaseRunDigest')) return false
    if (!wt.hasOwnProperty('Txt')) return false
    if (!wt.Txt.hasOwnProperty('length')) return false
    return true
  },

  // return empty workset text
  emptyWorksetText () {
    return {
      ModelName: '', ModelDigest: '', Name: '', IsReadonly: false, BaseRunDigest: '', Txt: []
    }
  },

  // return true if each list element isWorksetText()
  isWorksetTextList (wtl) {
    if (!wtl) return false
    if (!wtl.hasOwnProperty('length')) return false
    for (let k = 0; k < wtl.length; k++) {
      if (!this.isWorksetText(wtl[k])) return false
    }
    return true
  }
}

export default ModelCommon
