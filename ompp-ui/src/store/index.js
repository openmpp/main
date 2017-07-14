import Vue from 'vue'
import Vuex from 'vuex'

Vue.use(Vuex)

// getters names
export const GET = {
  OMPP_SRV_URL: 'omppServerUrl',
  UI_LANG: 'uiLang',
  THE_MODEL: 'theModel',
  TXT: 'txt'
}

// mutations names
export const SET = {
  UI_LANG: 'uiLang',
  THE_MODEL: 'theModel'
}

// store state: application model
const state = {
  // ui language, if not empty then selected by user
  uiLang: '',
  // current model
  theModel: {
    Name: '',
    Digest: ''
  },
  txt: 'store text'
}

// getters
const getters = {
  [GET.UI_LANG]: state => state.uiLang,
  [GET.OMPP_SRV_URL]: state => (process.env.NODE_ENV === 'development' ? 'http://localhost:4040' : ''),
  [GET.THE_MODEL]: state => state.theModel,
  [GET.TXT]: state => state.txt
}

// mutations: synchronized updates
const mutations = {
  [SET.UI_LANG] (state, lang) { state.uiLang = lang || '' },

  [SET.THE_MODEL] (state, md) {
    if (!md) return
    if (!md.hasOwnProperty('Name') || !md.hasOwnProperty('Digest')) return
    state.theModel.Name = md.Name || ''
    state.theModel.Digest = md.Digest || ''
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
