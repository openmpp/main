<!-- model run info dialog -->
<template>

<om-mcw-dialog :id="id" ref="noteDlg" class="note-dlg" :scrollable="true" acceptText="OK">
  <span slot="header">{{runDescr}}</span>
  <div>{{runNote}}</div>
  <div class="note-table">
    <div class="note-row">
      <span class="note-cell mono">Run name:</span><span class="note-cell mono">{{runName}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell mono">Status:</span><span class="note-cell mono">{{statusDescr}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell mono">Sub-values:</span>
      <span class="note-cell mono">{{subCount}}<span v-if="!isSucess">&nbsp;&nbsp;completed:&nbsp;{{subCompleted}}</span></span>
    </div>
    <div class="note-row">
      <span class="note-cell mono">Time:</span>
      <span class="note-cell mono">{{createdDt}} - {{lastDt}}<span v-if="timeDt"> = {{timeDt}}</span></span>
    </div>
    <div class="note-row">
      <span class="note-cell mono">Digest:</span><span class="note-cell mono">{{runDigest}}</span>
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
