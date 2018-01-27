import Vue from 'vue'
import Vuex from 'vuex'
import * as Mdf from '@/modelCommon'

Vue.use(Vuex)

// getters names
export const GET = {
  UI_LANG: 'uiLang',
  WORD_LIST: 'wordList',
  MODEL_LIST: 'modelList',
  MODEL_LIST_COUNT: 'modelListCount',
  THE_MODEL: 'theModel',
  THE_RUN_TEXT: 'theRunText',
  RUN_TEXT_LIST: 'runTextList',
  THE_WORKSET_TEXT: 'theWorksetText',
  WORKSET_TEXT_LIST: 'worksetTextList',
  OMPP_SRV_URL: 'omppServerUrl'
}

// exported actions names
export const SET = {
  UI_LANG: 'setUiLang',
  WORD_LIST: 'setWordList',
  MODEL_LIST: 'setModelList',
  THE_MODEL: 'setModel',
  EMPTY_MODEL: 'setEmptyModel',
  EMPTY_MODEL_LIST: 'setEmptyModelList',
  THE_RUN_TEXT: 'setRunText',
  THE_RUN_TEXT_BY_IDX: 'setRunTextByIdx',
  RUN_TEXT_LIST: 'setRunTextList',
  EMPTY_RUN_TEXT_LIST: 'setEmptyRunTextList',
  THE_WORKSET_TEXT: 'setWorksetText',
  THE_WORKSET_TEXT_BY_IDX: 'setWorksetTextByIdx',
  WORKSET_TEXT_LIST: 'setWorksetTextList',
  EMPTY_WORKSET_TEXT_LIST: 'setEmptyWorksetTextList'
}

// internal mutations names
const UPDATE = {
  UI_LANG: 'updUiLang',
  WORD_LIST: 'updWordList',
  WORD_LIST_ON_NEW: 'updWordListOnNew',
  MODEL_LIST: 'updModelList',
  THE_MODEL: 'updModel',
  THE_RUN_TEXT: 'updRunText',
  THE_RUN_TEXT_ON_NEW: 'updRunTextOnNew',
  THE_RUN_TEXT_BY_IDX: 'updRunTextByIdx',
  RUN_TEXT_LIST: 'updRunTextList',
  RUN_TEXT_LIST_ON_NEW: 'updRunTextListOnNew',
  THE_WORKSET_TEXT: 'updWorksetText',
  THE_WORKSET_TEXT_ON_NEW: 'updWorksetTextOnNew',
  THE_WORKSET_TEXT_BY_IDX: 'updWorksetTextByIdx',
  WORKSET_TEXT_LIST: 'updWorksetTextList',
  WORKSET_TEXT_LIST_ON_NEW: 'updWorksetTextListOnNew'
}

// store state: model
const state = {
  uiLang: '',
  wordList: Mdf.emptyWordList(),
  modelList: [],
  theModel: Mdf.emptyModel(),
  theRunText: Mdf.emptyRunText(),
  runTextList: [],
  theWorksetText: Mdf.emptyWorksetText(),
  worksetTextList: []
}

// getters
const getters = {
  [GET.UI_LANG]: state => state.uiLang,
  [GET.WORD_LIST]: state => state.wordList,
  [GET.THE_MODEL]: state => state.theModel,
  [GET.MODEL_LIST]: state => state.modelList,
  [GET.MODEL_LIST_COUNT]: state => state.modelList.length,
  [GET.RUN_TEXT_LIST]: state => state.runTextList,
  [GET.THE_RUN_TEXT]: state => state.theRunText,
  [GET.WORKSET_TEXT_LIST]: state => state.worksetTextList,
  [GET.THE_WORKSET_TEXT]: state => state.theWorksetText,
  [GET.OMPP_SRV_URL]: state => (process.env.OMPP_SRV_URL_ENV || '')
}

// mutations: synchronized updates
const mutations = {
  // assign new value to current UI language
  [UPDATE.UI_LANG] (state, lang) { state.uiLang = (lang || '') },

  // assign new value to model language-specific strings (model words)
  [UPDATE.WORD_LIST] (state, mw) {
    state.wordList = Mdf.emptyWordList()
    if (!mw) return
    state.wordList.ModelName = (mw.ModelName || '')
    state.wordList.ModelDigest = (mw.ModelDigest || '')
    state.wordList.LangCode = (mw.LangCode || '')
    state.wordList.ModelLangCode = (mw.ModelLangCode || '')
    if (mw.hasOwnProperty('LangWords')) {
      if (mw.LangWords && (mw.LangWords.length || 0) > 0) state.wordList.LangWords = mw.LangWords
    }
    if (mw.hasOwnProperty('ModelWords')) {
      if (mw.ModelWords && (mw.ModelWords.length || 0) > 0) state.wordList.ModelWords = mw.ModelWords
    }
  },

  // clear model words list if new model digest not same as word list model digest
  [UPDATE.WORD_LIST_ON_NEW] (state, modelDigest) {
    if ((modelDigest || '') !== state.wordList.ModelDigest) state.wordList = Mdf.emptyWordList()
  },

  // assign new value to current model, if (md) is a model
  [UPDATE.THE_MODEL] (state, md) {
    if (Mdf.isModel(md) || Mdf.isEmptyModel(md)) state.theModel = md
  },

  // assign new value to model list, if (ml) is a model list
  [UPDATE.MODEL_LIST] (state, ml) {
    if (Mdf.isModelList(ml)) state.modelList = ml
  },

  // set current run
  [UPDATE.THE_RUN_TEXT] (state, rt) {
    state.theRunText = Mdf.isRunText(rt) ? rt : Mdf.emptyRunText()
  },

  // set current run by index in run list or empty if index out of range
  [UPDATE.THE_RUN_TEXT_BY_IDX] (state, idx) {
    // if index out of range then set current run to empty value
    if (idx < 0 || idx >= state.runTextList.length) {
      state.theRunText = Mdf.emptyRunText()
      return
    }
    // if current run same as index run then return
    if (Mdf.isNotEmptyRunText(state.theRunText) &&
      state.theRunText.ModelDigest === state.runTextList[idx].ModelDigest &&
      state.theRunText.Name === state.runTextList[idx].Name &&
      state.theRunText.Digest === state.runTextList[idx].Digest &&
      state.theRunText.Status === state.runTextList[idx].Status &&
      state.theRunText.SubCount === state.runTextList[idx].SubCount &&
      (state.theRunText.CreateDateTime || '') === (state.runTextList[idx].CreateDateTime || '') &&
      (state.theRunText.UpdateDateTime || '') === (state.runTextList[idx].UpdateDateTime || '')) {
      return  // same model run
    }
    // update current run to run at the index
    state.theRunText = JSON.parse(JSON.stringify(state.runTextList[idx]))
  },

  // clear current run if model digest not the same else set current run to first in run list
  [UPDATE.THE_RUN_TEXT_ON_NEW] (state, modelDigest) {
    if ((modelDigest || '') !== state.theRunText.ModelDigest) {
      state.theRunText = Mdf.emptyRunText()
      return
    }
    if (!Mdf.isLength(state.runTextList)) return  // model run list empty

    // if current run same as index run then return
    if (Mdf.isNotEmptyRunText(state.theRunText) &&
      state.theRunText.ModelDigest === state.runTextList[0].ModelDigest &&
      state.theRunText.Name === state.runTextList[0].Name &&
      state.theRunText.Digest === state.runTextList[0].Digest &&
      state.theRunText.Status === state.runTextList[0].Status &&
      state.theRunText.SubCount === state.runTextList[0].SubCount &&
      (state.theRunText.CreateDateTime || '') === (state.runTextList[0].CreateDateTime || '') &&
      (state.theRunText.UpdateDateTime || '') === (state.runTextList[0].UpdateDateTime || '')) {
      return  // same model run
    }

    // update current model run to the first in model run list
    state.theRunText = JSON.parse(JSON.stringify(state.runTextList[0]))
  },

  // assign new value to run list, if (rl) is a model run text list
  [UPDATE.RUN_TEXT_LIST] (state, rtl) {
    if (Mdf.isRunTextList(rtl)) state.runTextList = rtl
  },

  // clear run list if model digest not same as run list model digest
  [UPDATE.RUN_TEXT_LIST_ON_NEW] (state, modelDigest) {
    let digest = ''
    if (state.runTextList.length > 0) {
      if (Mdf.isRunText(state.runTextList[0])) digest = state.runTextList[0].ModelDigest || ''
    }
    if ((modelDigest || '') !== digest) state.runTextList = []
  },

  // set current workset
  [UPDATE.THE_WORKSET_TEXT] (state, wt) {
    state.theWorksetText = Mdf.isWorksetText(wt) ? wt : Mdf.emptyWorksetText()
  },

  // set current workset by index in workset list or empty if index out of range
  [UPDATE.THE_WORKSET_TEXT_BY_IDX] (state, idx) {
    // if index out of range then set current workset to empty value
    if (idx < 0 || idx >= state.worksetTextList.length) {
      state.theWorksetText = Mdf.emptyWorksetText()
      return
    }
    // if current workset same as index workset then return
    if (Mdf.isNotEmptyWorksetText(state.theWorksetText) &&
      state.theWorksetText.ModelDigest === state.worksetTextList[idx].ModelDigest &&
      state.theWorksetText.Name === state.worksetTextList[idx].Name) {
      return  // same workset
    }
    // update current workset to workset at the index
    state.theWorksetText = JSON.parse(JSON.stringify(state.worksetTextList[idx]))
  },

  // clear current workset if model digest not the same else set current workset to first in workset list
  [UPDATE.THE_WORKSET_TEXT_ON_NEW] (state, modelDigest) {
    if ((modelDigest || '') !== state.theWorksetText.ModelDigest) {
      state.theWorksetText = Mdf.emptyWorksetText()
      return
    }
    if (!Mdf.isLength(state.worksetTextList)) return // workset list empty

    // if current workset has same model and name then exit
    if (Mdf.isNotEmptyWorksetText(state.theWorksetText) &&
      state.theWorksetText.ModelDigest === modelDigest &&
      state.theWorksetText.Name === state.worksetTextList[0].Name) {
      return  // same workset
    }

    // update current workset to the first in workset list
    state.theWorksetText = JSON.parse(JSON.stringify(state.worksetTextList[0]))
  },

  // assign new value to workset list, if (wtl) is a model run list
  [UPDATE.WORKSET_TEXT_LIST] (state, wtl) {
    if (Mdf.isWorksetTextList(wtl)) state.worksetTextList = wtl
  },

  // clear workset list if model digest not same as workset list model digest
  [UPDATE.WORKSET_TEXT_LIST_ON_NEW] (state, modelDigest) {
    let digest = ''
    if (state.worksetTextList.length > 0) {
      if (Mdf.isWorksetText(state.worksetTextList[0])) digest = state.worksetTextList[0].ModelDigest || ''
    }
    if ((modelDigest || '') !== digest) state.worksetTextList = []
  }
}

// actions: combine multiple mutations or async updates
const actions = {
  [SET.UI_LANG] ({ commit, dispatch }, lang) {
    commit(UPDATE.UI_LANG, lang)
    commit(UPDATE.WORD_LIST, Mdf.emptyWordList())
  },

  // set new value to model words list
  [SET.WORD_LIST] ({ commit, dispatch }, mw) {
    commit(UPDATE.WORD_LIST, mw)
  },

  // set new value to current model, clear run list and workset list
  [SET.THE_MODEL] ({ commit, dispatch }, md) {
    let digest = Mdf.modelDigest(md)
    commit(UPDATE.THE_MODEL, md)
    commit(UPDATE.WORD_LIST_ON_NEW, digest)
    commit(UPDATE.RUN_TEXT_LIST_ON_NEW, digest)
    commit(UPDATE.THE_RUN_TEXT_ON_NEW, digest)
    commit(UPDATE.WORKSET_TEXT_LIST_ON_NEW, digest)
    commit(UPDATE.THE_WORKSET_TEXT_ON_NEW, digest)
  },

  // clear the model: set current model to empty value
  [SET.EMPTY_MODEL] ({ commit, dispatch }) {
    dispatch(SET.THE_MODEL, Mdf.emptyModel())
  },

  // set new value to model list and clear current model
  [SET.MODEL_LIST] ({ commit, dispatch }, ml) {
    dispatch(SET.EMPTY_MODEL)
    commit(UPDATE.MODEL_LIST, ml)
  },

  // clear model list: set model list and current model to empty value
  [SET.EMPTY_MODEL_LIST] ({ commit, dispatch }) {
    dispatch(SET.MODEL_LIST, [])
  },

  // set current run
  [SET.THE_RUN_TEXT] ({ commit, dispatch }, rt) {
    commit(UPDATE.THE_RUN_TEXT, rt)
  },

  // set current run by index in run list or empty if index out of range
  [SET.THE_RUN_TEXT_BY_IDX] ({ commit, dispatch }, idx) {
    commit(UPDATE.THE_RUN_TEXT_BY_IDX, idx)
  },

  // set new value to run list
  [SET.RUN_TEXT_LIST] ({ commit, dispatch }, rl) {
    commit(UPDATE.RUN_TEXT_LIST, rl)
    dispatch(SET.THE_RUN_TEXT_BY_IDX, 0)
  },

  // clear run list: set to empty value
  [SET.EMPTY_RUN_TEXT_LIST] ({ commit, dispatch }) {
    dispatch(SET.RUN_TEXT_LIST, [])
  },

  // set current workset
  [SET.THE_WORKSET_TEXT] ({ commit, dispatch }, wt) {
    commit(UPDATE.THE_WORKSET_TEXT, wt)
  },

  // set current workset by index in workset list or empty if index out of range
  [SET.THE_WORKSET_TEXT_BY_IDX] ({ commit, dispatch }, idx) {
    commit(UPDATE.THE_WORKSET_TEXT_BY_IDX, idx)
  },

  [SET.WORKSET_TEXT_LIST] ({ commit, dispatch }, wl) {
    commit(UPDATE.WORKSET_TEXT_LIST, wl)
    dispatch(SET.THE_WORKSET_TEXT_BY_IDX, 0)
  },

  // clear workset list: set to empty value
  [SET.EMPTY_WORKSET_TEXT_LIST] ({ commit, dispatch }) {
    dispatch(SET.WORKSET_TEXT_LIST, [])
  }
}

// export store itself
const store = new Vuex.Store({
  state,
  getters,
  mutations,
  actions,
  strict: process.env.NODE_ENV !== 'production'
})

export default store
