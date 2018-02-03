<template>

<div id="model-page" class="main-container mdc-typography">

  <div class="hdr-row mdc-typography--body1">

    <span v-if="loadDone">

      <span v-if="!isWsView" 
        @click="showRunInfoDlg()" 
        class="cell-icon-link material-icons"
        alt="Description and notes" title="Description and notes">
          <span v-if="isSuccessTheRun">event_note</span>
          <span v-else>priority_high</span>
      </span>
      <!-- isWsView -->
      <span v-else>

        <span
          @click="showWsInfoDlg()" 
          class="cell-icon-link material-icons"
          alt="Description and notes" title="Description and notes">event_note</span>

        <span v-if="!isWsEdit">
          <span
            @click="doWsEditToggle()" 
            class="cell-icon-link material-icons"
            alt="Edit input set of parameters" title="Edit input set of parameters">mode_edit</span>
          <span v-if="!isRunPanel"
            @click="showRunPanel()" 
            class="cell-icon-link material-icons"
            alt="Run the model" title="Run the model">directions_run</span>
          <span v-else
            class="cell-icon-empty material-icons" 
            alt="Run the model" title="Run the model">directions_run</span>
        </span>
        <!-- isWsEdit -->
        <span v-else>
          <span
            @click="doWsEditToggle()" 
            class="cell-icon-link material-icons"
            alt="Save input set of parameters" title="Save input set of parameters">save</span>
          <span
            class="cell-icon-empty material-icons" 
            alt="Run the model" title="Run the model">directions_run</span>
        </span>

      </span>
    </span>

    <!-- !loadDone -->
    <span v-else 
      class="cell-icon-empty material-icons" 
      title="Information not available" alt="Information not available" aria-hidden="true">event_note</span>

    <span v-if="loadDone" class="hdr-text">
      <span v-if="isNotEmptyHdr">
        <span v-if="!isWsView && !isSuccessTheRun" class="cell-status mdc-typography--body2">{{statusOfTheRun}}</span>
        <span class="mono">{{lastTimeOfHdr}}&nbsp;</span><span class="mdc-typography--body2">{{nameOfHdr}}</span>
        <span>{{ descrOfHdr }}</span>
      </span>
      <span v-else>
        <span class="mdc-typography--body2">{{emptyHdrMsg}}</span>
      </span>
    </span>
    
    <span class="hdr-text mdc-typography--body2">
      <refresh-model
        :digest="digest"
        :refresh-tickle="refreshTickle"
        @done="doneModelLoad" @wait="()=>{}">
      </refresh-model>
      <refresh-run v-if="(runDigest || '') !== ''"
        :model-digest="digest"
        :run-digest="runDigest"
        :refresh-tickle="refreshTickle"
        :refresh-run-tickle="refreshRunTickle"
        @done="doneRunLoad" @wait="()=>{}">
      </refresh-run>
      <refresh-run-list
        :digest="digest"
        :refresh-tickle="refreshTickle"
        :refresh-run-list-tickle="refreshRunListTickle"
        @done="doneRunListLoad" @wait="()=>{}">
      </refresh-run-list>
      <refresh-workset v-if="(wsName || '') !== ''"
        :model-digest="digest"
        :workset-name="wsName"
        :refresh-tickle="refreshTickle"
        :refresh-ws-tickle="refreshWsTickle"
        @done="doneWsLoad" @wait="()=>{}">
      </refresh-workset>
      <refresh-workset-list
        :digest="digest"
        :refresh-tickle="refreshTickle"
        :refresh-ws-list-tickle="refreshWsListTickle"
        @done="doneWsListLoad" @wait="()=>{}">
      </refresh-workset-list>
      <update-workset-status v-if="(wsName || '') !== ''"
        :model-digest="digest"
        :workset-name="wsName"
        :enable-edit="!isWsEdit"
        :save-ws-status-tickle="saveWsStatusTickle"
        @done="doneWsStatusUpdate" @wait="()=>{}">
      </update-workset-status>
    </span>

  </div>

  <template v-if="isRunPanel">

    <div v-if="isEmptyRunStep" class="panel-frame mdc-typography--body1">
      <div>
        <span
          @click="hideRunPanel()" 
          class="cell-icon-link material-icons"
          alt="Close" title="Close">close</span>
        <span class="panel-item">Enter the name for new model run:</span>
      </div>
      <div>
        <span
          :class="{'mdc-text-field--disabled': isWsEdit}"
          class="mdc-text-field mdc-text-field--box mdc-text-field--fullwidth">
          <input type="text"
            id="run-name-input"
            ref="runNameInput"
            maxlength="255"
            :disabled="isWsEdit"
            :value="autoNewRunName"
            alt="Name of new model run"
            title="Name of new model run"
            class="mdc-text-field__input"></input>
        </span>
      </div>
      <div>
        <om-mcw-button 
          @click="doModelRun"
          :raised="true"
          :disabled="isWsEdit"
          class="panel-item"
          :alt="'Run the model with input set ' + wsName"
          :title="'Run the model with input set ' + wsName">
          <i class="material-icons mdc-button__icon">directions_run</i>Run the model</om-mcw-button>
        <input type="number"
          id="sub-count-input" ref="subCountInput" size="4" maxlength="4" min="1" max="9999"
          :value="newRunSubCount"
          :disabled="isWsEdit"
          class="panel-sub-count" alt="Number of sub-values" title="Number of sub-values"></input>
        <span class="panel-item">Sub-Values</span>
      </div>
    </div>

    <div v-if="isInitRunStep" class="panel-frame mdc-typography--body1">
      <div>
        <span
          @click="hideRunPanel()" 
          class="cell-icon-link material-icons"
          alt="Close" title="Close">close</span>
        <span class="mdc-typography--body2">Running: </span><span class="mdc-typography--body1">{{newRunName}}</span>
      </div>
      <div>
        <new-run-init
          :model-digest="digest"
          :new-run-name="newRunName"
          :workset-name="wsName"
          :sub-count="newRunSubCount"
          @done="doneNewRunInit"
          @wait="()=>{}">
        </new-run-init>
      </div>
    </div>

    <div v-if="isProcRunStep || isFinalRunStep" class="panel-frame mdc-typography--body1">
      <div>
        <span
          @click="hideRunPanel()" 
          class="cell-icon-link material-icons"
          alt="Close" title="Close">close</span>
        <span class="mdc-typography--body1">{{newRunName}}</span>
      </div>
      <div class="panel-line">
        <new-run-progress  v-if="!isFinalRunStep"
          :model-digest="digest"
          :new-run-key="newRunState.RunKey"
          :start="newRunLogStart"
          :count="newRunLogSize"
          @done="doneNewRunProgress"
          @wait="()=>{}">
        </new-run-progress>
        <span class="mono"><span v-if="newRunTimeDt">{{newRunTimeDt}} = </span>{{newRunState.StartDateTime}} - {{newRunState.UpdateDateTime}}</span>
        <span class="mono"><i>[{{newRunState.RunKey}}.log]</i></span>
      </div>
      <div>
        <div v-for="ln in newRunLineLst" :key="ln.offset" class="mono mdc-typography--caption">{{ln.text}}</div>
      </div>
    </div>

  <!-- isRunPanel -->
  </template>

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
    <router-view
      @tab-mounted="doTabMounted"
      @run-select="doRunSelect"
      @workset-select="doWsSelect"></router-view>
  </main>

  <run-info-dialog ref="theRunInfoDlg" id="the-run-info-dlg"></run-info-dialog>
  <workset-info-dialog ref="theWsInfoDlg" id="the-ws-info-dlg"></workset-info-dialog>

</div>
  
</template>

<script>
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import * as Mdf from '@/modelCommon'
import OmMcwButton from './OmMcwButton'
import RefreshModel from './RefreshModel'
import RefreshRun from './RefreshRun'
import RefreshRunList from './RefreshRunList'
import RefreshWorkset from './RefreshWorkset'
import RefreshWorksetList from './RefreshWorksetList'
import UpdateWorksetStatus from './UpdateWorksetStatus'
import NewRunInit from './NewRunInit'
import NewRunProgress from './NewRunProgress'
import RunInfoDialog from './RunInfoDialog'
import WorksetInfoDialog from './WorksetInfoDialog'

const EMPTY_RUN_STEP = 0      // empty state of new model: undefined
const INIT_RUN_STEP = 1       // initiate new model run: submit request to the server
const PROC_RUN_STEP = 2       // model run in progress
const FINAL_RUN_STEP = 16     // final state of model run: completed or failed
const MIN_LOG_PAGE_SIZE = 4   // min run log page size to read from the server
const MAX_LOG_PAGE_SIZE = 10  // max run log page size to read from the server

export default {
  components: {
    OmMcwButton,
    RefreshModel,
    RefreshRun,
    RefreshRunList,
    RefreshWorkset,
    RefreshWorksetList,
    UpdateWorksetStatus,
    NewRunInit,
    NewRunProgress,
    RunInfoDialog,
    WorksetInfoDialog
  },

  props: {
    digest: '',
    refreshTickle: false
  },

  data () {
    return {
      loadModelDone: false,
      loadRunDone: false,
      loadRunListDone: false,
      loadWsDone: false,
      loadWsListDone: false,
      refreshRunTickle: false,
      refreshRunListTickle: false,
      refreshWsTickle: false,
      refreshWsListTickle: false,
      saveWsStatusTickle: false,
      mountedDone: false,
      modelName: '',
      runDigest: '',
      wsName: '',
      isWsView: false,            // if true then page view is a workset else run
      isRunPanel: false,          // if true then run panel is visible
      newRunStep: EMPTY_RUN_STEP, // model run step: initial, start new, view progress
      newRunName: '',
      newRunSubCount: 1,
      newRunState: Mdf.emptyRunState(),
      newRunTimeDt: '',
      newRunLogStart: 0,
      newRunLogSize: 0,
      newRunLineLst: [],
      tabCount: 0,
      tabLst: []
    }
  },

  computed: {
    loadDone () {
      return this.mountedDone &&
        this.loadModelDone && this.loadRunDone && this.loadRunListDone && this.loadWsDone && this.loadWsListDone
    },
    isSuccessTheRun () { return Mdf.isRunSuccess(this.theRunText) },
    currentRunSetKey () { return [this.runDigest, this.wsName, this.isWsView].toString() },

    // if true then workset edit mode else readonly and model run enabled
    isWsEdit () {
      return this.isWsView && Mdf.isNotEmptyWorksetText(this.theWorksetText) && !this.theWorksetText.IsReadonly
    },
    // make new model run name
    autoNewRunName () {
      const dt = new Date()
      return (this.wsName || '') + ' ' + Mdf.dtToTimeStamp(dt)
    },

    // model new run step: empty, initialize, in progress, final
    isEmptyRunStep () { return this.newRunStep === EMPTY_RUN_STEP },
    isInitRunStep () { return this.newRunStep === INIT_RUN_STEP },
    isProcRunStep () { return this.newRunStep === PROC_RUN_STEP },
    isFinalRunStep () { return this.newRunStep === FINAL_RUN_STEP },

    // view header line
    isNotEmptyHdr () {
      return this.isWsView ? Mdf.isNotEmptyWorksetText(this.theWorksetText) : Mdf.isNotEmptyRunText(this.theRunText)
    },
    lastTimeOfHdr () {
      return this.isWsView ? Mdf.dtStr(this.theWorksetText.UpdateDateTime) : Mdf.dtStr(this.theRunText.UpdateDateTime)
    },
    nameOfHdr () {
      return this.isWsView ? (this.theWorksetText.Name || '') : (this.theRunText.Name || '')
    },
    descrOfHdr () {
      return this.isWsView ? Mdf.descrOfTxt(this.theWorksetText) : Mdf.descrOfTxt(this.theRunText)
    },
    emptyHdrMsg () {
      return this.isWsView ? 'No input set of parameters found' : 'No model run results'
    },

    ...mapGetters({
      theModel: GET.THE_MODEL,
      theRunText: GET.THE_RUN_TEXT,
      runTextList: GET.RUN_TEXT_LIST,
      theWorksetText: GET.THE_WORKSET_TEXT,
      worksetTextList: GET.WORKSET_TEXT_LIST
    })
  },

  watch: {
    // refresh button handler
    refreshTickle () {
      this.loadModelDone = false
      this.loadRunDone = false
      this.loadRunListDone = false
      this.loadWsDone = false
      this.loadWsListDone = false
    },

    // add tabs if data ready and component mounted
    loadDone () {
      if (this.loadDone) {
        this.doTabAdd(false, false, 'parameter-list')
        if (this.isSuccessTheRun) {
          this.doTabAdd(false, false, 'table-list')
          this.doTabAdd(false, false, 'run-list')
        }
        this.doTabAdd(false, false, 'workset-list')
        //
        this.doTabHeaderRefresh()
        this.doTabPathRefresh()
        this.doTabAdd(true, true, 'parameter-list')
      }
    },

    // change of selected run or workset
    currentRunSetKey () {
      if (this.loadDone) {
        this.doTabPathRefresh()
      }
    }
  },

  methods: {
    // reload event handlers: async get the model, runs and worksets
    doneModelLoad (isSuccess) {
      this.modelName = this.theModel.Name
      this.loadModelDone = true
      this.resetRunStep()
    },
    doneRunLoad (isSuccess) {
      this.loadRunDone = true
      this.isWsView = !isSuccess
    },
    doneRunListLoad (isSuccess) {
      this.loadRunListDone = true
      let ok = !!isSuccess
      if (ok) {
        this.runDigest = this.theRunText.Digest
      } else {
        this.isWsView = true
        this.loadRunDone = true   // do not refresh run: run list empty
      }
    },
    doneWsLoad (isSuccess) { this.loadWsDone = true },
    doneWsListLoad (isSuccess) {
      this.loadWsListDone = true
      let ok = !!isSuccess
      if (ok) {
        this.wsName = this.theWorksetText.Name
      } else {
        this.loadWsDone = true  // do not refresh workset: workset list empty
      }
    },
    doneWsStatusUpdate () {
      this.refreshWsTickle = !this.refreshWsTickle  // refersh current workset
    },

    // run selected from the list: reload run info
    doRunSelect (dgst) {
      if ((dgst || '') === '') {
        console.log('run digest is empty')
        return
      }
      this.runDigest = dgst
      this.isWsView = false
    },
    // workset selected from the list: reload workset info
    doWsSelect (name) {
      if ((name || '') === '') {
        console.log('workset name is empty')
        return
      }
      this.wsName = name
      this.isWsView = true
    },

    // toggle workset readonly status
    doWsEditToggle () {
      this.saveWsStatusTickle = !this.saveWsStatusTickle
    },

    // show or close model run panel
    showRunPanel () { this.isRunPanel = true },
    hideRunPanel () {
      this.isRunPanel = false
      this.resetRunStep()
    },
    // clean new run data
    resetRunStep () {
      this.newRunStep = EMPTY_RUN_STEP
      this.newRunName = ''
      this.newRunSubCount = 1
      this.newRunState = Mdf.emptyRunState()
      this.newRunTimeDt = ''
      this.newRunLogStart = 0
      this.newRunLogSize = MIN_LOG_PAGE_SIZE
      this.newRunLineLst = []
    },

    // run the model
    doModelRun () {
      // cleanup model run name and sub-count
      let name = this.$refs.runNameInput.value
      name = name.replace(/["'`$}{@\\]/g, ' ').trim()
      if ((name || '') === '') name = this.autoNewRunName

      let sub = this.$refs.subCountInput.value
      sub = sub.replace(/[^0-9]/g, ' ').trim()
      let nSub = ((sub || '') !== '') ? parseInt(sub) : 1

      this.newRunName = name  // actual values afetr cleanup
      this.newRunSubCount = nSub || 1
      this.newRunState = Mdf.emptyRunState()

      // start new model run: send request to the server
      this.newRunStep = INIT_RUN_STEP
    },

    // new model run started: response from server
    doneNewRunInit (ok, rst) {
      this.newRunStep = ok ? PROC_RUN_STEP : FINAL_RUN_STEP
      if (!!ok && Mdf.isNotEmptyRunState(rst)) {
        this.newRunState = rst
        this.newRunName = rst.RunName
        this.newSubCount = (rst.SubCount || 1)
      }
    },

    // model run progress: response from server
    doneNewRunProgress (ok, rlp) {
      if (!ok || !Mdf.isNotEmptyRunStateLog(rlp)) return  // empty run state or error

      this.newRunState = Mdf.toRunStateFromLog(rlp)
      this.newRunTimeDt = Mdf.toIntervalStr(this.newRunState.StartDateTime, this.newRunState.UpdateDateTime)

      // update log lines
      let nLen = Mdf.lengthOf(rlp.Lines)
      if (nLen > 0) {
        let log = []
        for (let k = 0; k < nLen; k++) {
          log.push({offset: rlp.Offset + k, text: rlp.Lines[k]})
        }
        this.newRunLineLst = log
      }

      // check is it final update: model run completed
      let isDone = (this.newRunState.IsFinal && rlp.Offset + rlp.Size >= rlp.TotalSize)

      if (!isDone) {
        this.newRunLogStart = rlp.Offset + rlp.Size
        if (this.newRunLogStart + this.newRunLogSize < rlp.TotalSize) {
          this.newRunLogSize = Math.min(rlp.TotalSize - this.newRunLogStart, MAX_LOG_PAGE_SIZE)
          this.newRunLogStart = rlp.TotalSize - this.newRunLogSize
        } else {
          if (this.newRunLogStart + this.newRunLogSize >= rlp.TotalSize) this.newRunLogStart = rlp.TotalSize - this.newRunLogSize
        }
        if (this.newRunLogStart < rlp.Offset) this.newRunLogStart = rlp.Offset + 1
      } else {
        this.loadRunListDone = false
        this.refreshRunListTickle = !this.refreshRunListTickle  // refersh run list and current run
        this.refreshRunTickle = !this.refreshRunTickle          // refersh run list and current run
        this.newRunStep = FINAL_RUN_STEP
      }
    },

    // show currently selected run info
    showRunInfoDlg () {
      this.$refs.theRunInfoDlg.showRunInfo(this.theRunText)
    },
    // show currently selected workset info
    showWsInfoDlg () {
      this.$refs.theWsInfoDlg.showWsInfo(this.theWorksetText)
    },

    // tab mounted: handle tabs mounted by direct link
    doTabMounted (kind, dn = '') {
      if ((kind || '') === '') {
        console.log('invalid (empty) kind of tab mounted')
        return
      }
      if (!this.loadDone) {
        return    // wait until model loaded
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

    // update all tab route paths
    doTabPathRefresh () {
      for (let k = 0; k < this.tabLst.length; k++) {
        const ti = this.makeTabInfo(this.tabLst[k].kind, this.tabLst[k].dn)
        if ((ti.path || '') !== '' && (ti.path || '') !== this.tabLst[k].path) {
          this.tabLst[k].path = ti.path
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
        path: (ti.path || ''),
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

      // path to current model and to current run or workset
      let mp = '/model/' + this.digest
      let rdn = (this.theRunText.Digest || '') !== '' ? (this.theRunText.Digest || '') : (this.theRunText.Name || '')
      let rwp = '/' +
        (this.isWsView ? Mdf.SET_OF_RUNSET + '/' + (this.theWorksetText.Name || '') : Mdf.RUN_OF_RUNSET + '/' + rdn)

      switch (kind) {
        case 'parameter-list':
          return {
            key: 'pl-' + this.digest,
            path: mp + rwp + '/parameter-list',
            title: 'Parameters: ' + Mdf.paramCount(this.theModel).toString()
          }
        case 'table-list':
          return {
            key: 'tl-' + this.digest,
            path: mp + '/run/' + rdn + '/table-list',
            title: 'Output tables: ' + Mdf.outTableCount(this.theModel).toString()
          }
        case 'run-list':
          return {
            key: 'rtl-' + this.digest,
            path: mp + '/run-list',
            title: 'Model runs: ' + Mdf.runTextCount(this.runTextList).toString()
          }
        case 'workset-list':
          return {
            key: 'wtl-' + this.digest,
            path: mp + '/workset-list',
            title: 'Input sets: ' + Mdf.worksetTextCount(this.worksetTextList).toString()
          }
        case 'parameter':
          const pt = Mdf.paramTextByName(this.theModel, dn)
          let pds = Mdf.descrOfDescrNote(pt)
          return {
            key: 'p-' + this.digest + '-' + (dn || ''),
            path: mp + rwp + '/parameter/' + (dn || ''),
            title: (pds !== '') ? pds : (dn || '')
          }
        case 'table':
          const tt = Mdf.tableTextByName(this.theModel, dn)
          let tds = Mdf.descrOfDescrNote(tt)
          return {
            key: 't-' + this.digest + '-' + (dn || ''),
            path: mp + '/run/' + rdn + '/table/' + (dn || ''),
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
    this.resetRunStep()
  }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
  @import "@material/textfield/mdc-text-field";
  
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
    margin-left: .5rem;
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

  /* model run panel */
  .panel-border {
    margin-top: .5rem;
    margin-right: 2rem;
    border-width: 1px;
    border-style: solid;
    border-color: rgba(0, 0, 0, 0.12);
  }
  .panel-frame {
    margin-top: .5rem;
    margin-right: 2rem;
    @extend .panel-border;
  }
  .panel-item {
    margin-right: .5rem;
  }
  .panel-value {
    @extend .panel-item;
    @extend .panel-border;
    @extend .mdc-typography--body1;
  }
  .panel-sub-count {
    width: 4rem;
    text-align: right;
    @extend .panel-value;
  }
  .panel-line {
    margin-top: .25rem;
  }

  /* cell material icon: a link or empty (non-link) */
  .cell-icon {
    vertical-align: middle;
    margin: 0;
    padding-left: 0;
    padding-right: 0;
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
    @extend .mdc-theme--primary-light-bg;
    @extend .mdc-theme--text-primary-on-primary;
    /* @extend .mdc-theme--text-disabled-on-background; */
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
