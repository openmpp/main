import Vue from 'vue'
import Router from 'vue-router'
import ModelListPage from '@/components/ModelListPage'
import ModelPage from '@/components/ModelPage'
import ParameterListPage from '@/components/ParameterListPage'
import TableListPage from '@/components/TableListPage'
import RunListPage from '@/components/RunListPage'
import WorksetListPage from '@/components/WorksetListPage'
import ParameterPage from '@/components/ParameterPage'
import TablePage from '@/components/TablePage'
import SessionSettings from '@/components/SessionSettingsPage'
import LicensePage from '@/components/LicensePage'
import None404Page from '@/components/None404Page'
import UnderConstruction from '@/components/UnderConstructionPage'

Vue.use(Router)

export default new Router({
  mode: 'history',
  routes: [
    {
      path: '/',
      component: ModelListPage
    },
    {
      path: '/model/:digest',
      component: ModelPage,
      props: true,
      children: [
        {
          path: ':runOrSet/:nameDigest/parameter-list',
          component: ParameterListPage,
          props: true
        },
        {
          path: 'run/:nameDigest/table-list',
          component: TableListPage,
          props: true
        },
        {
          path: 'run-list',
          component: RunListPage,
          props: true
        },
        {
          path: 'workset-list',
          component: WorksetListPage,
          props: true
        },
        {
          path: ':runOrSet/:nameDigest/parameter/:paramName',
          component: ParameterPage,
          props: true
        },
        {
          path: 'run/:nameDigest/table/:tableName',
          component: TablePage,
          props: true
        }
      ]
    },
    {
      path: '/run-list/:digest',
      component: UnderConstruction
    },
    {
      path: '/workset-list',
      component: UnderConstruction
    },
    {
      path: '/parameters',
      component: UnderConstruction
    },
    {
      path: '/outputs',
      component: UnderConstruction
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
      path: '/run-history',
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
      path: '/license',
      component: LicensePage
    },
    {
      path: '*',
      component: None404Page
    }
  ]
})
