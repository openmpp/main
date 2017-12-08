import Vue from 'vue'
import Vuex from 'vuex'
import { default as Mdf } from '@/modelCommon'

Vue.use(Vuex)

// getters names
export const GET = {
  UI_LANG: 'uiLang',
  THE_MODEL: 'theModel',
  MODEL_LIST: 'modelList',
  MODEL_LIST_COUNT: 'modelListCount',
  THE_RUN_TEXT: 'theRunText',
  RUN_TEXT_LIST: 'runTextList',
  THE_WORKSET_TEXT: 'theWorksetText',
  WORKSET_TEXT_LIST: 'worksetTextList',
  OMPP_SRV_URL: 'omppServerUrl'
}

// exported actions names
export const SET = {
  UI_LANG: 'uiLang',
  THE_MODEL: 'theModel',
  MODEL_LIST: 'modelList',
  EMPTY_MODEL: 'emptyModel',
  EMPTY_MODEL_LIST: 'emptyModelList',
  THE_RUN_TEXT_BY_IDX: 'theRunTextByIdx',
  RUN_TEXT_LIST: 'runTextList',
  EMPTY_RUN_TEXT_LIST: 'emptyRunTextList',
  THE_WORKSET_TEXT_BY_IDX: 'theWorksetTextByIdx',
  WORKSET_TEXT_LIST: 'worksetTextList',
  EMPTY_WORKSET_TEXT_LIST: 'emptyWorksetTextList'
}

// internal mutations names
const UPDATE = {
  UI_LANG: 'uiLang',
  THE_MODEL: 'theModel',
  MODEL_LIST: 'modelList',
  THE_RUN_TEXT_ON_NEW: 'theRunTextOnNew',
  THE_RUN_TEXT_BY_IDX: 'theRunTextByIdx',
  RUN_TEXT_LIST: 'runTextList',
  RUN_TEXT_LIST_ON_NEW: 'runTextListOnNew',
  THE_WORKSET_TEXT_ON_NEW: 'theWorksetTextOnNew',
  THE_WORKSET_TEXT_BY_IDX: 'theWorksetTextByIdx',
  WORKSET_TEXT_LIST: 'worksetTextList',
  WORKSET_TEXT_LIST_ON_NEW: 'worksetTextListOnNew'
}

// getters
const getters = {
  [GET.UI_LANG]: state => state.uiLang,
  [GET.THE_MODEL]: state => state.theModel,
  [GET.MODEL_LIST]: state => state.modelList,
  [GET.MODEL_LIST_COUNT]: state => state.modelList.length,
  [GET.THE_RUN_TEXT]: state => state.theRunText,
  [GET.RUN_TEXT_LIST]: state => state.runTextList,
  [GET.THE_WORKSET_TEXT]: state => state.theWorksetText,
  [GET.WORKSET_TEXT_LIST]: state => state.worksetTextList,
  [GET.OMPP_SRV_URL]: state => (process.env.OMPP_SRV_URL_ENV || '')
}

// store state: model
const state = {
  uiLang: '',
  theModel: Mdf.emptyModel(),
  modelList: [],
  theRunText: Mdf.emptyRunText(),
  runTextList: [],
  theWorksetText: Mdf.emptyWorksetText(),
  worksetTextList: []
}

// mutations: synchronized updates
const mutations = {
  [UPDATE.UI_LANG] (state, lang) { state.uiLang = (lang || '') },

  // assign new value to current model, if (md) is a model
  [UPDATE.THE_MODEL] (state, md) {
    if (Mdf.isModel(md) || Mdf.isEmptyModel(md)) state.theModel = md
  },

  // assign new value to model list, if (ml) is a model list
  [UPDATE.MODEL_LIST] (state, ml) {
    if (Mdf.isModelList(ml)) state.modelList = ml
  },

  // clear current run if model digest not the same else set current run to first in run list
  [UPDATE.THE_RUN_TEXT_ON_NEW] (state, modelDigest) {
    if ((modelDigest || '') !== state.theRunText.ModelDigest) {
      state.theRunText = Mdf.emptyRunText()
    } else {
      state.theRunText =
        (state.runTextList.length > 0)
        ? JSON.parse(JSON.stringify(state.runTextList[0]))
        : Mdf.emptyRunText()
    }
  },

  // set current run by index in run list or empty if index out of range
  [UPDATE.THE_RUN_TEXT_BY_IDX] (state, idx) {
    state.theRunText =
      (idx >= 0 && idx < state.runTextList.length)
      ? JSON.parse(JSON.stringify(state.runTextList[idx]))
      : Mdf.emptyRunText()
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

  // clear current workset if model digest not the same else set current workset to first in workset list
  [UPDATE.THE_WORKSET_TEXT_ON_NEW] (state, modelDigest) {
    if ((modelDigest || '') !== state.theWorksetText.ModelDigest) {
      state.theWorksetText = Mdf.emptyWorksetText()
    } else {
      state.theWorksetText =
        (state.worksetTextList.length > 0)
        ? JSON.parse(JSON.stringify(state.worksetTextList[0]))
        : Mdf.emptyWorksetText()
    }
  },

  // set current workset by index in workset list or empty if index out of range
  [UPDATE.THE_WORKSET_TEXT_BY_IDX] (state, idx) {
    state.theWorksetText =
      (idx >= 0 && idx < state.worksetTextList.length)
      ? JSON.parse(JSON.stringify(state.worksetTextList[idx]))
      : Mdf.emptyWorksetText()
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
  },

  // set new value to current model, clear run list and workset list
  [SET.THE_MODEL] ({ commit, dispatch }, md) {
    let digest = Mdf.modelDigest(md)
    commit(UPDATE.THE_MODEL, md)
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

  // set current run by index in run list or empty if index out of range
  [SET.THE_RUN_TEXT_BY_IDX] ({ commit, dispatch }, idx) {
    commit(UPDATE.THE_RUN_TEXT_BY_IDX, idx)
  },

  // set new value to run list
  [SET.RUN_TEXT_LIST] ({ commit, dispatch }, rl) {
    commit(UPDATE.RUN_TEXT_LIST, rl)
    commit(UPDATE.THE_RUN_TEXT_BY_IDX, 0)
  },

  // clear run list: set to empty value
  [SET.EMPTY_RUN_TEXT_LIST] ({ commit, dispatch }) {
    dispatch(SET.RUN_TEXT_LIST, [])
  },

  // set current workset by index in workset list or empty if index out of range
  [SET.THE_WORKSET_TEXT_BY_IDX] ({ commit, dispatch }, idx) {
    commit(UPDATE.THE_WORKSET_TEXT_BY_IDX, idx)
  },

  // set new value to workset list
  [SET.WORKSET_TEXT_LIST] ({ commit, dispatch }, wl) {
    commit(UPDATE.WORKSET_TEXT_LIST, wl)
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
