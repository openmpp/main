<!-- model run info dialog -->
<template>

<om-mcw-dialog :id="id" ref="noteDlg" :scrollable="true" acceptText="OK">
  <span slot="header">{{runDescr}}</span>
  <div v-if="runNote">{{runNote}}</div>
  <div class="note-table mono">
    <div class="note-row">
      <span class="note-cell">Run name:</span><span class="note-cell">{{runName}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell">Status:</span><span class="note-cell">{{statusDescr}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell">Sub-values:</span>
      <span class="note-cell">{{subCount}}</span>
    </div>
    <div v-if="!isSucess" class="note-row">
      <span class="note-cell">Sub-values completed:</span>
      <span class="note-cell">{{subCompleted}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell">Started:</span>
      <span class="note-cell">{{createdDt}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell">Completed:</span>
      <span class="note-cell">{{lastDt}}</span>
    </div>
    <div v-if="timeDt" class="note-row">
      <span class="note-cell">Duration:</span>
      <span class="note-cell">{{timeDt}}</span>
    </div>
    <div v-if="runDigest" class="note-row">
      <span class="note-cell">Digest:</span><span class="note-cell">{{runDigest}}</span>
    </div>
  </div>
</om-mcw-dialog>

</template>

<script>
import * as Mdf from '@/modelCommon'
import OmMcwDialog from '@/om-mcw/OmMcwDialog'

export default {
  components: { OmMcwDialog },

  props: {
    id: {
      type: String,
      required: true
    }
  },

  data () {
    return {
      runDescr: '',
      runNote: '',
      runName: '',
      runDigest: '',
      statusDescr: '',
      isSucess: false,
      subCount: 0,
      subCompleted: 0,
      createdDt: '',
      lastDt: '',
      timeDt: ''
    }
  },

  methods: {
    // show model run info
    showRunInfo (rt) {
      if (!Mdf.isNotEmptyRunText(rt)) {
        console.log('Empty model run text')
        return
      }

      // run properties
      this.runDescr = Mdf.descrOfTxt(rt)
      this.runNote = Mdf.noteOfTxt(rt)
      this.runName = rt.Name
      this.runDigest = rt.Digest
      this.statusDescr = Mdf.statusText(rt)
      this.isSucess = Mdf.isRunSuccess(rt)
      this.subCount = (rt.SubCount || 0)
      this.subCompleted = (rt.SubCompleted || 0)
      this.createdDt = Mdf.dtStr(rt.CreateDateTime)
      this.lastDt = Mdf.dtStr(rt.UpdateDateTime)
      this.timeDt = Mdf.toIntervalStr(this.createdDt, this.lastDt)

      this.$refs.noteDlg.open() // show run info dialog
    }
  }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";

  /* note dialog */
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
