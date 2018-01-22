<template>

<div id="run-list-page" class="mdc-typography mdc-typography--body1">

  <div v-if="isRunList">
    <ul class="main-list mdc-list mdc-list--two-line">

      <li v-for="(r, idx) in runTextList" :key="'rt-' + idx.toString()" class="mdc-list-item">

        <span
          @click="showRunInfo(r)" 
          class="material-icons mdc-list-item__start-detail note-link" 
          :title="r.Name + ' notes'"
          :alt="r.Name + ' notes'">
            <span v-if="isSuccess(r)">event_note</span>
            <span v-else>event_busy</span>
        </span>
        <span
          @click="doRunClick(idx, r.Digest)"
          class="link-next" 
          :title="r.Name"
          :alt="r.Name" 
          >
          <span class="mdc-list-item__text">
            <span>{{ r.Name }}</span>
            <span class="item-line mdc-list-item__secondary-text"><span class="mono mdc-typography--body1">{{lastTime(r)}} </span>{{ descrOf(r) }}</span>
          </span>
        </span>

      </li>

    </ul>
  </div>

  <run-info-dialog ref="runInfoDlg" id="run-info-dlg"></run-info-dialog>

</div>
  
</template>

<script>
import { mapGetters, mapActions } from 'vuex'
import { GET, SET } from '@/store'
import * as Mdf from '@/modelCommon'
import RunInfoDialog from './RunInfoDialog'

export default {
  components: { RunInfoDialog },

  props: {
    digest: '',
    refreshTickle: false
  },

  data () {
    return {
      prevDigest: '',
      prevIndex: -1
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
    isSuccess (rt) { return Mdf.isRunSuccess(rt) },
    descrOf (rt) { return Mdf.descrOfTxt(rt) },

    // click on run: select this run as current run
    doRunClick (idx, runDigest) {
      if (idx === this.prevIndex && runDigest === this.prevDigest) return  // exit: click on same item in the list
      this.prevDigest = (runDigest || '')
      this.prevIndex = (idx || 0)
      // select new current run by index
      this.setTheRunTextByIdx(idx)
      this.$emit('run-select', runDigest)
    },

    // show run info
    showRunInfo (rt) {
      this.$refs.runInfoDlg.showRunInfo(rt)
    },

    ...mapActions({
      setTheRunTextByIdx: SET.THE_RUN_TEXT_BY_IDX
    })
  },

  mounted () {
    this.$emit('tab-mounted', 'run-list', '')
  }
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
  .link-next {
    display: block;
    width: 100%;
    height: 100%;
    text-decoration: none;
    @extend .mdc-theme--text-primary-on-background;
    &:hover {
      cursor: pointer;
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
