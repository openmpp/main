// db structures common functions: description and notes

// description if object has DescrNote
export const descrOfDescrNote = (tdn) => {
  if (!tdn) return ''
  if (!tdn.hasOwnProperty('DescrNote')) return ''
  return (tdn.DescrNote.Descr || '')
}

// notes if object has DescrNote
export const noteOfDescrNote = (tdn) => {
  if (!tdn) return ''
  if (!tdn.hasOwnProperty('DescrNote')) return ''
  return (tdn.DescrNote.Note || '')
}

// return true if notes of DescrNote not empty
export const isNoteOfDescrNote = (tdn) => {
  if (!tdn) return false
  if (!tdn.hasOwnProperty('DescrNote')) return false
  return (tdn.DescrNote.Note || '') !== ''
}

// description if object has Txt[0] description-notes
export const descrOfTxt = (tdn) => {
  if (!tdn) return ''
  if (!tdn.hasOwnProperty('Txt')) return ''
  if (!tdn.Txt.hasOwnProperty('length')) return ''
  return (tdn.Txt.length > 0) ? (tdn.Txt[0].Descr || '') : ''
}

// notes if object has Txt[0] description-notes
export const noteOfTxt = (tdn) => {
  if (!tdn) return ''
  if (!tdn.hasOwnProperty('Txt')) return ''
  if (!tdn.Txt.hasOwnProperty('length')) return ''
  return (tdn.Txt.length > 0) ? (tdn.Txt[0].Note || '') : ''
}

// return true if notes of Txt[0] not empty
export const isNoteOfTxt = (tdn) => {
  if (!tdn) return false
  if (!tdn.hasOwnProperty('Txt')) return false
  if (!tdn.Txt.hasOwnProperty('length')) return false
  return (tdn.Txt.length > 0) ? (tdn.Txt[0].Note || '') !== '' : false
}
