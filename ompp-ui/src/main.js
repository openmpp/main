import Vue from 'vue'
import 'core-js/es6/promise'
import store from './store'
import router from './router'
import App from './App'

Vue.config.productionTip = false

/* eslint-disable no-new */
new Vue({
  el: '#app',
  store,
  router,
  render: h => h(App)
})
