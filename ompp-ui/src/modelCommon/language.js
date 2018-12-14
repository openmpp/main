// db structures common functions: language and language list

// hard coded enum code for total enum id
export const ALL_WORD_CODE = 'all'

// return empty language-specific model words list
export const emptyWordList = () => {
  return {
    ModelName: '',
    ModelDigest: '',
    LangCode: '',
    LangWords: [],
    ModelLangCode: '',
    ModelWords: []
  }
}

// find label by code in language-specific model words list, return code if labe not found
export const wordByCode = (mw, code) => {
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
  return code // not found: return original code
}
