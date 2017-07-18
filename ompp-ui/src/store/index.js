import Vue from 'vue'
import Vuex from 'vuex'

Vue.use(Vuex)

// getters names
export const GET = {
  OMPP_SRV_URL: 'omppServerUrl',
  UI_LANG: 'uiLang',
  THE_MODEL: 'theModel'
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
    Model: {
      Name: '',
      Digest: ''
    }
  }
}

// getters
const getters = {
  [GET.UI_LANG]: state => state.uiLang,
  [GET.OMPP_SRV_URL]: state => (process.env.OMPP_SRV_URL_ENV || ''),
  [GET.THE_MODEL]: state => state.theModel
}

// mutations: synchronized updates
const mutations = {
  [SET.UI_LANG] (state, lang) { state.uiLang = lang || '' },

  [SET.THE_MODEL] (state, md) {
    if (!md) return
    if (!md.hasOwnProperty('Model')) return
    if (!md.Model.hasOwnProperty('Name') || !md.Model.hasOwnProperty('Digest')) return
    state.theModel = md
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
