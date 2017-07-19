import Vue from 'vue'
import Router from 'vue-router'
import HomePage from '@/components/HomePage'
import ModelPage from '@/components/ModelPage'
import InfoPage from '@/components/InfoPage'
import None404Page from '@/components/None404Page'
import UnderConstruction from '@/components/UnderConstructionPage'
import SessionSettings from '@/components/SessionSettingsPage'
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
      component: UnderConstruction
    },
    {
      path: '/service',
      component: UnderConstruction
    },
    {
      path: '/history',
      component: UnderConstruction
    },
    {
      path: '/login',
      component: UnderConstruction
    },
    {
      path: '/settings',
      component: SessionSettings
    },
    {
      path: '/one',
      component: One
    },
    {
      path: '*',
      component: None404Page
    }
  ]
})
