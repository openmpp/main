<template>

<div id="model-page" class="main-container mdc-typography">

  <div class="hdr-row mdc-typography--body1">

    <span v-if="loadRunDone" class="hdr-text">
      <span v-if="isNotEmptyTheRun" 
          @click="showRunInfo()" 
          class="cell-icon-link material-icons" alt="Model run info" title="Model run info">
            <span v-if="isSuccessTheRun">directions_run</span>
            <span v-else>priority_high</span>
        </span>
        <span v-else 
          class="cell-icon-empty material-icons" 
          title="Model run info not available" alt="Model run info not available" aria-hidden="true">directions_run</span>
      <span v-if="isNotEmptyTheRun">
        <span v-if="!isSuccessTheRun" class="cell-status mdc-typography--body2">{{statusOfTheRun}}</span>
        <span class="mono">{{lastTimeOfTheRun}}&nbsp;</span><span class="mdc-typography--body2">{{nameOfTheRun}}</span>
        <span>{{ descrOfTheRun }}</span>
      </span>
      <span v-else>
        <span class="mdc-typography--body2">No model run results</span>
      </span>
    </span>
    
    <span class="hdr-text mdc-typography--body2">
      <span v-if="!loadRunDone" class="cell-icon-link material-icons" aria-hidden="true">refresh</span>
      <refresh-model
        :digest="digest"
        :refresh-tickle="refreshTickle"
        @done="doneModelLoad" @wait="()=>{}">
      </refresh-model>
      <refresh-run-list
        :digest="digest"
        :refresh-tickle="refreshTickle"
        @done="doneRunListLoad" @wait="()=>{}">
      </refresh-run-list>
      <refresh-workset-list
        :digest="digest"
        :refresh-tickle="refreshTickle"
        @done="doneWsListLoad" @wait="()=>{}">
      </refresh-workset-list>
    </span>

  </div>

  <nav class="tab-container mdc-typography--body2">
    <div
      v-for="t in tabLst" :key="t.key" :id="t.id"
      :class="t.active ? 'tab-item-active' : 'tab-item-inactive'" class="tab-item">

        <router-link
          :to="t.path"
          @click.native="doTabLink(t.id)"
          :alt="t.title"
          :title="t.title"
          class="tab-link">{{t.title}}</router-link>

        <om-mcw-button
          :compact="true" 
          @click="doTabClose(t.id)"
          :alt="'Close ' + t.title"
          :title="'Close ' + t.title" 
          class="tab-close-button"><i class="tab-close-icon material-icons mdc-button__icon">close</i>
        </om-mcw-button>

    </div>
  </nav>
  
  <main class="main-container">
    <router-view :refresh-tickle="refreshTickle" @tab-mounted="doTabMounted"></router-view>
  </main>

  <om-mcw-dialog ref="theRunNoteDlg" id="the-run-note-dlg" acceptText="OK">
    <span slot="header">{{titleNoteDlg}}</span>
    <div>{{textNoteDlg}}</div>
    <div class="note-table">
      <div class="note-row">
        <span class="note-cell mono">Run name:</span><span class="note-cell mono">{{nameNoteDlg}}</span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">Status:</span><span class="note-cell mono">{{statusNoteDlg}}</span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">Sub-values:</span>
        <span class="note-cell mono">{{subCountNoteDlg}}<span v-if="!isSucessNoteDlg">&nbsp;&nbsp;completed:&nbsp;{{subCompletedNoteDlg}}</span></span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">Run time:</span>
        <span class="note-cell mono">{{createdNoteDlg}} - {{lastNoteDlg}}<span v-if="timeNoteDlg"> = {{timeNoteDlg}}</span></span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">Digest:</span><span class="note-cell mono">{{digestNoteDlg}}</span>
      </div>
    </div>
  </om-mcw-dialog>

</div>
  
</template>

<script>
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import { default as Mdf } from '@/modelCommon'
import OmMcwButton from './OmMcwButton'
import OmMcwDialog from './OmMcwDialog'
import RefreshModel from './RefreshModel'
import RefreshRunList from './RefreshRunList'
import RefreshWorksetList from './RefreshWorksetList'

export default {
  components: { OmMcwButton, OmMcwDialog, RefreshModel, RefreshRunList, RefreshWorksetList },

  props: {
    digest: '',
    refreshTickle: false
  },

  data () {
    return {
      loadModelDone: false,
      loadRunDone: false,
      loadWsDone: false,
      mountedDone: false,
      tabCount: 0,
      tabLst: [],
      titleNoteDlg: '',
      textNoteDlg: '',
      nameNoteDlg: '',
      statusNoteDlg: '',
      isSucessNoteDlg: false,
      subCountNoteDlg: 0,
      subCompletedNoteDlg: 0,
      createdNoteDlg: '',
      lastNoteDlg: '',
      timeNoteDlg: '',
      digestNoteDlg: ''
    }
  },

  computed: {
    modelTabReady () { return this.loadModelDone && this.mountedDone },
    runTabReady () { return this.loadModelDone && this.loadRunDone && this.mountedDone },
    worksetTabReady () { return this.loadModelDone && this.loadWsDone && this.mountedDone },

    isNotEmptyTheRun () { return Mdf.isNotEmptyRunText(this.theRunText) },
    isSuccessTheRun () { return (this.theRunText.Status || '') === 's' },
    lastTimeOfTheRun () { return Mdf.dtStr(this.theRunText.UpdateDateTime) },
    statusOfTheRun () { return this.statusOfRun(this.theRunText) },
    nameOfTheRun () { return (this.theRunText.Name || '') },
    descrOfTheRun () { return Mdf.descrOfTxt(this.theRunText) },

    ...mapGetters({
      theModel: GET.THE_MODEL,
      theRunText: GET.THE_RUN_TEXT,
      runTextList: GET.RUN_TEXT_LIST,
      worksetTextList: GET.WORKSET_TEXT_LIST
    })
  },

  watch: {
    // refresh button handler
    refreshTickle () {
      this.loadModelDone = false
      this.loadRunDone = false
      this.loadWsDone = false
    },

    // add tabs if data ready and component mounted
    modelTabReady () {
      if (this.modelTabReady) {
        this.doTabAdd(false, false, 'parameter-list')
        this.doTabAdd(false, false, 'table-list')
        this.doTabHeaderRefresh()
      }
    },
    runTabReady () {
      if (this.runTabReady) {
        this.doTabAdd(false, false, 'run-list')
        this.doTabHeaderRefresh()
      }
    },
    worksetTabReady () {
      if (this.worksetTabReady) {
        this.doTabAdd(false, false, 'workset-list')
        this.doTabHeaderRefresh()
      }
    }
  },

  methods: {
    // reload event handlers: async get the model, runs and worksets
    doneModelLoad () { this.loadModelDone = true },
    doneRunListLoad () { this.loadRunDone = true },
    doneWsListLoad () { this.loadWsDone = true },

    // return run status by code: i=init p=progress s=success x=exit e=error(failed)
    statusOfRun (rt) {
      switch (rt.Status || '') {
        case 's': return 'success'
        case 'p': return 'in progress'
        case 'i': return 'not yet started'
        case 'e': return 'failed'
        case 'x': return 'exit (not completed)'
      }
      return 'unknown'
    },

    // show run info
    showRunInfo () {
      let rt = this.theRunText
      this.titleNoteDlg = this.descrOfTheRun
      this.textNoteDlg = Mdf.noteOfTxt(rt)
      this.nameNoteDlg = this.nameOfTheRun
      this.digestNoteDlg = rt.Digest
      this.statusNoteDlg = this.statusOfTheRun
      this.isSucessNoteDlg = this.isSuccessTheRun
      this.subCountNoteDlg = (rt.SubCount || 0)
      this.subCompletedNoteDlg = (rt.SubCompleted || 0)
      this.createdNoteDlg = Mdf.dtStr(rt.CreateDateTime)
      this.lastNoteDlg = this.lastTimeOfTheRun
      this.timeNoteDlg = ''

      let start = Date.parse(this.createdNoteDlg)
      let stop = Date.parse(this.lastNoteDlg)
      if (start && stop) {
        let s = Math.floor((stop - start) / 1000) % 60
        let m = Math.floor((stop - start) / (60 * 1000)) % 60
        let h = Math.floor((stop - start) / (60 * 60 * 1000)) % 24
        let d = Math.floor((stop - start) / (24 * 60 * 60 * 1000))
        this.timeNoteDlg =
          ((d > 0) ? d.toString() + 'd ' : '') +
          ('00' + h.toString()).slice(-2) + ':' +
          ('00' + m.toString()).slice(-2) + ':' +
          ('00' + s.toString()).slice(-2)
      }
      this.$refs.theRunNoteDlg.open()
    },

    // tab mounted: handle tabs mounted by direct link
    doTabMounted (kind, dn = '') {
      if ((kind || '') === '') {
        console.log('invalid (empty) kind of tab mounted')
        return
      }
      this.doTabAdd(true, false, kind, dn)
    },

    // click on tab link: activate that tab
    doTabLink (tabId, isRoute = false, path = '') {
      for (let k = 0; k < this.tabLst.length; k++) {
        this.tabLst[k].active = this.tabLst[k].id === tabId
      }

      // if new path is a result of tab add or close then route to new path
      if (isRoute && (path || '') !== '') {
        this.$router.push(path)
      }
    },

    // close tab and if tab was active then activate other tab
    doTabClose (tabId) {
      // do not close last tab
      if (this.tabLst.length <= 1) return

      // remove tab from the list
      let n = -1
      for (let k = 0; k < this.tabLst.length; k++) {
        if (this.tabLst[k].id === tabId) {
          if (this.tabLst[k].active) n = (k < this.tabLst.length - 1) ? k : this.tabLst.length - 2
          this.tabLst.splice(k, 1)
          break
        }
      }

      // if it was an active tab then actvate tab at the same position: previous tab or last tab
      if (n >= 0 && n < this.tabLst.length) {
        this.doTabLink(this.tabLst[n].id, true, this.tabLst[n].path)
      }
    },

    // update all tab headers
    doTabHeaderRefresh () {
      for (let k = 0; k < this.tabLst.length; k++) {
        const ti = this.makeTabInfo(this.tabLst[k].kind, this.tabLst[k].dn)
        if ((ti.title || '') !== '' && (ti.title || '') !== this.tabLst[k].title) {
          this.tabLst[k].title = ti.title
        }
      }
    },

    // if tab not exist then add new tab and make it active else activate existing tab
    doTabAdd (isActivate, isRoute, kind, dn = '') {
      // make tab key path and title
      const ti = this.makeTabInfo(kind, dn)
      if (!kind || (kind || '') === '' || !ti) {
        console.log('tab kind is empty or invalid')
        return
      }

      // find existing tab by key and activate if required
      for (let k = 0; k < this.tabLst.length; k++) {
        if (this.tabLst[k].key === ti.key) {
          if (isActivate) {
            this.doTabLink(this.tabLst[k].id, isRoute, this.tabLst[k].path) // activate existing tab
          }
          return // done: tab already exist
        }
      }

      // make new tab
      this.tabCount++
      let t = {
        id: 'tab-id-' + this.tabCount.toString(),
        kind: kind,
        dn: (dn || ''),
        key: (ti.key || ''),
        path: '/model/' + this.digest + '/' + (ti.path || ''),
        active: false,
        title: (ti.title || '')
      }

      // activate new tab
      this.tabLst.push(t)
      if (isActivate) {
        this.doTabLink(t.id, isRoute, t.path)
      } else {
        if (this.tabLst.length === 1) {
          this.doTabLink(t.id, true, t.path)
        }
      }
    },

    // return tab info: key, path and title
    makeTabInfo (kind, dn = '') {
      // tab kind must be defined
      if ((kind || '') === '') {
        return this.emptyTabInfo()
      }

      switch (kind) {
        case 'parameter-list':
          return {
            key: 'pl-' + this.digest,
            path: 'parameter-list',
            title: 'Parameters: ' + Mdf.paramCount(this.theModel).toString()
          }
        case 'table-list':
          return {
            key: 'tl-' + this.digest,
            path: 'table-list',
            title: 'Output tables: ' + Mdf.outTableCount(this.theModel).toString()
          }
        case 'run-list':
          return {
            key: 'rtl-' + this.digest,
            path: 'run-list',
            title: 'Model runs: ' + Mdf.runTextCount(this.runTextList).toString()
          }
        case 'workset-list':
          return {
            key: 'wtl-' + this.digest,
            path: 'workset-list',
            title: 'Input sets: ' + Mdf.worksetTextCount(this.worksetTextList).toString()
          }
        case 'parameter':
          const pt = Mdf.paramTextByName(this.theModel, dn)
          let pds = Mdf.descrOfDescrNote(pt)
          return {
            key: 'p-' + this.digest + '-' + (dn || ''),
            path: 'parameter/' + (dn || ''),
            title: (pds !== '') ? pds : (dn || '')
          }
        case 'table':
          const tt = Mdf.tableTextByName(this.theModel, dn)
          let tds = Mdf.descrOfDescrNote(tt)
          return {
            key: 't-' + this.digest + '-' + (dn || ''),
            path: 'table/' + (dn || ''),
            title: (tds !== '') ? tds : (dn || '')
          }
      }
      // default
      console.log('tab kind invalid', kind)
      return this.emptyTabInfo()
    },

    // empty tab info: invalid default value
    emptyTabInfo () { return { key: '', path: '', title: '' } }
  },

  mounted () {
    this.mountedDone = true
  }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";

  /* page and tab container content body */
  .main-container {
    height: 100%;
    flex: 1 1 auto;
    display: flex; 
    flex-direction: column;
    overflow-y: auto;
  }

  /* tab bar: tab items with link and close button */
  .tab-container {
    display: flex;
    flex-direction: row;
    overflow: hidden;
    margin-top: .5rem;
  }
  .tab-item {
    display: inline-flex;
    flex-grow: 0;
    flex-shrink: 1;
    overflow: hidden;
    align-items: center;
    padding-left: .5rem;
    margin-right: 1px;
    border-top-right-radius: 1rem;
  }
  .tab-item-inactive {
    @extend .tab-item;
    @extend .mdc-theme--primary-light-bg;
  }
  .tab-item-active {
    @extend .tab-item;
    @extend .mdc-theme--primary-bg;
  }
  .tab-link {
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    text-decoration: none;
    outline: none;
    @extend .mdc-theme--text-primary-on-primary;
  }
  .tab-close-button {
    border: 0;
    padding: 0 0 0 .5rem;
    min-width: 1rem;
  }
  .tab-close-icon {
    @extend .mdc-theme--text-primary-on-primary;
  }

  /* header row of model, run, workset properties */
  .hdr-row {
    flex-grow: 1;
    flex-shrink: 0;
    display: flex;
    flex-direction: row;
    overflow: hidden;
    margin-top: .5rem;
  }
  .hdr-text {
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
  }
  /*
  .hdr-table {
    display: table;
    padding-top: .5rem;
  }
  .hdr-table-row {
    display: table-row;
  }
  .hdr-table-cell {
    display: table-cell;
    padding-right: .5rem;
  }
  */

  /* cell material icon: a link or empty (non-link) */
  .cell-icon {
    vertical-align: middle;
    margin: 0;
    padding-left: 0.5rem;
    padding-right: 0.5rem;
    -webkit-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
  }
  .cell-icon-link {
    @extend .cell-icon;
    &:hover {
      cursor: pointer;
    }
    @extend .mdc-theme--text-primary-on-primary;
    @extend .mdc-theme--primary-bg;
  }
  .cell-icon-empty {
    @extend .cell-icon;
    cursor: default;
    @extend .mdc-theme--text-disabled-on-background;
  }
  .cell-status {
    text-transform: uppercase;
  }

  /* note dialog, fix handsontable z-index: 101; */
  #the-run-note-dlg {
    z-index: 201;
  }
  .note-table {
    display: table;
    margin-top: .5rem;
  }
  .note-row {
    display: table-row;
  }
  .note-cell {
    display: table-cell;
    white-space: nowrap;
    &:first-child {
      padding-right: .5rem;
    }
  }
</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
