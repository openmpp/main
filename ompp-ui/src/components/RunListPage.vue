<template>

<div id="run-list-page" class="mdc-typography mdc-typography--body1">

  <div v-if="isRunList">
    <ul class="main-list mdc-list mdc-list--two-line">

      <li v-for="(r, idx) in runTextList" :key="'rt-' + idx.toString()" class="mdc-list-item">

        <a href="#" 
          @click="showRunInfo(r)" 
          class="material-icons mdc-list-item__start-detail note-link" 
          :title="r.Name + ' notes'"
          :alt="r.Name + ' notes'">
            <span v-if="isSuccess(r)">event_note</span>
            <span v-else>event_busy</span>
          </a>
        <a
          @click="doRunClick(idx)"
          href="#"
          class="ahref-next" 
          :title="r.Name"
          :alt="r.Name" 
          >
          <span class="mdc-list-item__text">
            <span>{{ r.Name }}</span>
            <span class="item-line mdc-list-item__text__secondary"><span class="mono mdc-typography--body1">{{lastTime(r)}} </span>{{ descrOf(r) }}</span>
          </span>
        </a>
        <!--
        <router-link
          @click.native="doRunClick(idx)"
          :to="'/model/' + digest + '/run/' + r.Digest" 
          class="ahref-next" 
          :title="r.Name"
          :alt="r.Name" 
          >
          <span class="mdc-list-item__text">
            <span>{{ r.Name }}</span>
            <span class="item-line mdc-list-item__text__secondary"><span class="mono mdc-typography--body1">{{lastTime(r)}} </span>{{ descrOf(r) }}</span>
          </span>
        </router-link>
        -->

      </li>

    </ul>
  </div>

  <om-mcw-dialog ref="noteDlg" id="run-note-dlg" acceptText="OK">
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
        <span class="note-cell mono">Time:</span>
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
import { mapGetters, mapActions } from 'vuex'
import { GET, SET } from '@/store'
import { default as Mdf } from '@/modelCommon'
import OmMcwDialog from './OmMcwDialog'

export default {
  props: {
    digest: '',
    refreshTickle: false
  },

  data () {
    return {
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
    isRunList () { return Mdf.isRunTextList(this.runTextList) },

    ...mapGetters({
      runTextList: GET.RUN_TEXT_LIST
    })
  },

  watch: {
    // refresh button handler
  },

  methods: {
    lastTime (rt) { return Mdf.dtStr(rt.UpdateDateTime) },
    isSuccess (rt) { return (rt.Status || '') === 's' },
    descrOf (rt) { return Mdf.descrOfTxt(rt) },

    // click on run: select this run as current run
    doRunClick (idx) {
      this.setTheRunTextByIdx(idx)
    },

    // return run status by code: i=init p=progress s=success x=exit e=error(failed)
    statusText (rt) {
      switch ((rt.Status || '')) {
        case 's': return 'success'
        case 'p': return 'in progress'
        case 'i': return 'not yet started'
        case 'e': return 'failed'
        case 'x': return 'exit (not completed)'
      }
      return 'unknown'
    },

    // show run info
    showRunInfo (rt) {
      this.titleNoteDlg = Mdf.descrOfTxt(rt)
      this.textNoteDlg = Mdf.noteOfTxt(rt)
      this.nameNoteDlg = rt.Name
      this.digestNoteDlg = rt.Digest
      this.statusNoteDlg = this.statusText(rt)
      this.isSucessNoteDlg = this.isSuccess(rt)
      this.subCountNoteDlg = (rt.SubCount || 0)
      this.subCompletedNoteDlg = (rt.SubCompleted || 0)
      this.createdNoteDlg = Mdf.dtStr(rt.CreateDateTime)
      this.lastNoteDlg = this.lastTime(rt)
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
      this.$refs.noteDlg.open()
    },

    ...mapActions({
      setTheRunTextByIdx: SET.THE_RUN_TEXT_BY_IDX
    })
  },

  mounted () {
    this.$emit('tab-mounted', 'run-list', '')
  },

  components: { OmMcwDialog }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";

  /* main body list */
  .main-list {
    padding-left: 0;
  }

  /* a link to next page */
  .ahref-next {
    display: block;
    width: 100%;
    height: 100%;
    text-decoration: none;
    @extend .mdc-theme--text-primary-on-background;
    &:hover {
      background: rgba(0, 0, 0, 0.1);
    }
  }

  /* notes: a link or empty (not a link) */
  .note-item {
    display: inline-block;
    vertical-align: top;
    height: 100%;
    margin: 0;
    padding-left: 0.5em;
    padding-right: 0.5em;
  }
  .note-link {
    @extend .note-item;
    text-decoration: none;
    outline: none;
    @extend .mdc-theme--text-secondary-on-background;
  }
  a.note-link:hover {
    background: rgba(0, 0, 0, 0.1);
  }
  .note-empty {
    @extend .note-item;
    cursor: default;
    @extend .mdc-theme--text-disabled-on-background;
  }

  /* note dialog table info */
  .note-table {
    display: table;
    margin-top: .5em;
  }
  .note-row {
    display: table-row;
  }
  .note-cell {
    display: table-cell;
    white-space: nowrap;
    &:first-child {
      padding-right: .5em;
    }
  }
</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
