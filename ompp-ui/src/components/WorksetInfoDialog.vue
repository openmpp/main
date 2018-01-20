<!-- model workset info dialog -->
<template>

<om-mcw-dialog :id="id" ref="noteDlg" class="note-dlg" acceptText="OK">
  <span slot="header">{{wsDescr}}</span>
  <div>{{wsNote}}</div>
  <div class="note-table">
    <div class="note-row">
      <span class="note-cell mono">Name:</span><span class="note-cell mono">{{wsName}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell mono">Read only:</span><span class="note-cell mono">{{isRo}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell mono">Updated:</span><span class="note-cell mono">{{lastDt}}</span>
    </div>
    <div v-if="isBaseRun" class="note-row">
      <span class="note-cell mono">Base run:</span><span class="note-cell mono">{{baseRunDigest}}</span>
    </div>
    <div v-if="!isBaseRun && paramCount > 0" class="note-row">
      <span class="note-cell mono">Parameters:</span><span class="note-cell mono">{{paramCount}}</span>
    </div>
  </div>
</om-mcw-dialog>

</template>

<script>
import * as Mdf from '@/modelCommon'
import OmMcwDialog from './OmMcwDialog'

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
      wsDescr: '',
      wsNote: '',
      wsName: '',
      isRo: false,
      baseRunDigest: '',
      isBaseRun: false,
      lastDt: '',
      paramCount: 0
    }
  },

  methods: {
    // show model workset info
    showWsInfo (wt) {
      if (!Mdf.isNotEmptyWorksetText(wt)) {
        console.log('Empty model run text')
        return
      }

      // workset properties
      this.wsDescr = Mdf.descrOfTxt(wt)
      this.wsNote = Mdf.noteOfTxt(wt)
      this.wsName = wt.Name
      this.isRo = !!wt.IsReadonly
      this.baseRunDigest = wt.BaseRunDigest || ''
      this.isBaseRun = this.baseRunDigest !== ''
      this.paramCount = Mdf.lengthOf(wt.Param)
      this.lastDt = Mdf.dtStr(wt.UpdateDateTime)

      this.$refs.noteDlg.open()   // show workset info dialog
    }
  }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
  
  /* note dialog, fix handsontable z-index: 101; */
  .note-dlg {
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
