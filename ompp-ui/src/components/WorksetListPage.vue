<template>

<div id="workset-list-page" class="mdc-typography mdc-typography--body1">

  <div v-if="isWsList">
    <ul class="main-list mdc-list mdc-list--two-line">

      <li v-for="(w, idx) in worksetTextList" :key="'wt-' + idx.toString()" class="mdc-list-item">

        <span
          @click="showWsInfo(w)" 
          class="material-icons mdc-list-item__start-detail note-link" 
          :title="w.Name + ' notes'"
          :alt="w.Name + ' notes'">event_note</span>
        <a
          @click="doWsClick(idx)"
          href="#"
          class="ahref-next" 
          :title="w.Name"
          :alt="w.Name" 
          >
          <span class="mdc-list-item__text">
            <span>{{ w.Name }}</span>
            <span class="item-line mdc-list-item__secondary-text"><span class="mono mdc-typography--body1">{{lastTime(w)}} </span>{{ descrOf(w) }}</span>
          </span>
        </a>
        <!--
        <router-link
          @click.native="doWsClick(idx)"
          :to="'/model/' + digest + '/workset/' + w.Name" 
          class="ahref-next" 
          :title="w.Name"
          :alt="w.Name" 
          >
          <span class="mdc-list-item__text">
            <span>{{ w.Name }}</span>
            <span class="item-line mmdc-list-item__secondary-text><span class="mono mdc-typography--body1">{{lastTime(w)}} </span>{{ descrOf(w) }}</span>
          </span>
        </router-link>
        -->

      </li>

    </ul>
  </div>

  <om-mcw-dialog ref="noteDlg" id="ws-note-dlg" acceptText="OK">
    <span slot="header">{{titleNoteDlg}}</span>
    <div>{{textNoteDlg}}</div>
    <div class="note-table">
      <div class="note-row">
        <span class="note-cell mono">Name:</span><span class="note-cell mono">{{nameNoteDlg}}</span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">Read only:</span><span class="note-cell mono">{{isReadOnlyNoteDlg}}</span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">Updated:</span><span class="note-cell mono">{{lastNoteDlg}}</span>
      </div>
      <div v-if="(baseRunDigestNoteDlg !== '')" class="note-row">
        <span class="note-cell mono">Base run:</span><span class="note-cell mono">{{baseRunDigestNoteDlg}}</span>
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
      isReadOnlyNoteDlg: false,
      lastNoteDlg: '',
      baseRunDigestNoteDlg: ''
    }
  },

  computed: {
    isWsList () { return Mdf.isWorksetTextList(this.worksetTextList) },

    ...mapGetters({
      theWorksetText: GET.THE_WORKSET_TEXT,
      worksetTextList: GET.WORKSET_TEXT_LIST
    })
  },

  watch: {
    // refresh button handler
  },

  methods: {
    lastTime (wt) { return Mdf.dtStr(wt.UpdateDateTime) },
    descrOf (wt) { return Mdf.descrOfTxt(wt) },

    // click on workset: select this workset as current
    doWsClick (idx) {
      this.setTheWorksetTextByIdx(idx)
      console.log('name', this.theWorksetText.Name)
    },

    // show workset info
    showWsInfo (wt) {
      this.titleNoteDlg = Mdf.descrOfTxt(wt)
      this.textNoteDlg = Mdf.noteOfTxt(wt)
      this.nameNoteDlg = wt.Name
      this.isReadOnlyNoteDlg = !!wt.isReadonly
      this.lastNoteDlg = Mdf.dtStr(wt.UpdateDateTime)
      this.baseRunDigestNoteDlg = wt.BaseRunDigest || ''
      this.$refs.noteDlg.open()
    },

    ...mapActions({
      setTheWorksetTextByIdx: SET.THE_WORKSET_TEXT_BY_IDX
    })
  },

  mounted () {
    this.$emit('tab-mounted', 'workset-list', '')
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
    padding-left: 0.5rem;
    padding-right: 0.5rem;
  }
  .note-link {
    @extend .note-item;
    &:hover {
      cursor: pointer;
      background: rgba(0, 0, 0, 0.1);
    }
    @extend .mdc-theme--text-secondary-on-background;
  }
  .note-empty {
    @extend .note-item;
    cursor: default;
    @extend .mdc-theme--text-disabled-on-background;
  }

  /* note dialog table info */
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
