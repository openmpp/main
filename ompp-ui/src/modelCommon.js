// model common functions mixin

const ModelCommon = {
  // if this is a model
  isModel (md) {
    if (!md) return false
    if (!md.hasOwnProperty('Model')) return false
    return md.Model.hasOwnProperty('Name') && md.Model.hasOwnProperty('Digest')
  },

  // if this is an empty model
  isModelEmpty (md) {
    if (!this.isModel(md)) return false
    return (md.Model.Name || '') === '' || (md.Model.Digest || '') === ''
  },

  // retrun empty model object
  emptyModel () { return { Model: { Name: '', Digest: '' } } },

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

  // description if model has text info
  modelDescr (md) {
    if (!md) return ''
    if (!md.hasOwnProperty('DescrNote')) return ''
    return (md.DescrNote.Descr || '')
  },

  // make model title
  modelTitle (md) {
    if (!md) return ''
    if (!md.hasOwnProperty('Model')) return ''
    if ((md.Model.Digest || '') === '') return ''
    if (!md.hasOwnProperty('DescrNote')) return (md.Model.Name || '')
    return (md.Model.Name || '') +
      (((md.DescrNote.Descr || '') !== '') ? ': ' + (md.DescrNote.Descr || '') : '')
  },

  // is model notes not empty
  isModelNote (md) {
    if (!md) return false
    if (!md.hasOwnProperty('Model') || !md.hasOwnProperty('DescrNote')) return false
    return (md.DescrNote.Note || '') !== ''
  },

  // notes if model has text info
  modelNote (md) {
    if (!md) return ''
    if (!md.hasOwnProperty('DescrNote')) return ''
    return (md.DescrNote.Note || '')
  },

  // return true if each list element isModel()
  isModelList (ml) {
    if (!ml) return false
    if (!ml.hasOwnProperty('length')) return false
    for (let k = 0; k < ml.length; k++) {
      if (!this.isModel(ml[k])) return false
    }
    return true
  }
}

export default ModelCommon
