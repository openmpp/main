import Vue from 'vue'
import Vuex from 'vuex'
import { default as Mdf } from '@/modelCommon'

Vue.use(Vuex)

// getters names
export const GET = {
  OMPP_SRV_URL: 'omppServerUrl',
  UI_LANG: 'uiLang',
  THE_MODEL: 'theModel',
  MODEL_LIST: 'modelList'
}

// mutations names
export const SET = {
  UI_LANG: 'uiLang',
  THE_MODEL: 'theModel',
  MODEL_LIST: 'modelList',
  EMPTY_MODEL: 'emptyModel',
  EMPTY_MODEL_LIST: 'emptyModelList'
}

// store state: model
const state = {
  uiLang: '',
  theModel: { Model: { Name: '', Digest: '' } },
  modelList: []
}

// getters
const getters = {
  [GET.UI_LANG]: state => state.uiLang,
  [GET.OMPP_SRV_URL]: state => (process.env.OMPP_SRV_URL_ENV || ''),
  [GET.THE_MODEL]: state => state.theModel,
  [GET.MODEL_LIST]: state => state.modelList
}

// mutations: synchronized updates
const mutations = {
  [SET.UI_LANG] (state, lang) { state.uiLang = lang || '' },

  // assign new value to current model, if (md) is a model
  [SET.THE_MODEL] (state, md) {
    if (!Mdf.isModel(md)) return
    state.theModel = md
  },

  // clear the model: set current model to empty value
  [SET.EMPTY_MODEL] (state) { state.theModel = Mdf.emptyModel() },

  // assign new value to model list, if (ml) is a model list
  [SET.MODEL_LIST] (state, ml) {
    if (!Mdf.isModelList(ml)) return
    state.modelList = ml
  },

  // clear model list: set model list and current model to empty value
  [SET.EMPTY_MODEL_LIST] (state) {
    state.modelList = []
    state.theModel = Mdf.emptyModel()
  }
}

// async updates
const actions = {
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
