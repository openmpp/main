import Vue from 'vue'
import Router from 'vue-router'
import HomePage from '@/components/HomePage'
import ModelPage from '@/components/ModelPage'
import InfoPage from '@/components/InfoPage'
import None404Page from '@/components/None404Page'
import One from '@/components/One'

Vue.use(Router)

export default new Router({
  mode: 'history',
  routes: [
    {
      path: '/',
      component: HomePage
    },
    {
      path: '/model/:digest',
      component: ModelPage,
      props: true
    },
    {
      path: '/info',
      component: InfoPage
    },
    {
      path: '/run',
      component: One
    },
    {
      path: '/service',
      component: One
    },
    {
      path: '/history',
      component: One
    },
    {
      path: '/login',
      component: One
    },
    {
      path: '/settings',
      component: One
    },
    {
      path: '*',
      component: None404Page
    }
  ]
})
