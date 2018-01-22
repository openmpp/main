<!-- parameter info dialog -->
<template>

<om-mcw-dialog :id="id" ref="noteDlg" class="note-dlg" acceptText="OK">
  <span slot="header">{{paramDescr}}</span>
  <div v-if="(paramNote !== '')">{{paramNote}}</div>
  <br/>
  <div class="note-table">
    <div class="note-row">
      <span class="note-cell mono">Name:</span><span class="note-cell mono">{{paramName}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell mono">Type:</span><span class="note-cell mono">{{typeTitle}}</span>
    </div>
    <div v-if="paramSize.rank !== 0" class="note-row">
      <span class="note-cell mono">Dimensions:</span><span class="note-cell mono">{{paramSize.dimSize}}</span>
    </div>
    <div v-else class="note-row">
      <span class="note-cell mono">Rank:</span><span class="note-cell mono">{{paramSize.rank}}</span>
    </div>
    <div v-if="isSubCount" class="note-row">
      <span class="note-cell mono">SubValues:</span><span class="note-cell mono">{{subCount}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell mono">Digest:</span><span class="note-cell mono">{{paramDigest}}</span>
    </div>
  </div>
</om-mcw-dialog>
 
</template>

<script>
import { mapGetters } from 'vuex'
import { GET } from '@/store'
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
      paramName: '',
      subCount: 0,
      paramDescr: '',
      paramNote: '',
      typeTitle: '',
      paramDigest: '',
      isSubCount: false,
      paramSize: Mdf.emptyParamSize()
    }
  },

  computed: {
    ...mapGetters({
      theModel: GET.THE_MODEL
    })
  },

  methods: {
    showParamInfo (name, nSub) {
      if ((name || '') === '') {
        console.log('Empty parameter name')
        return
      }
      this.paramName = name
      this.subCount = nSub || 0

      // find parameter, paramemeter size and text info
      let txt = Mdf.paramTextByName(this.theModel, this.paramName)
      this.paramSize = Mdf.paramSizeByName(this.theModel, this.paramName)
      this.isSubCount = (this.subCount || 0) > 0

      // find parameter type
      let t = Mdf.typeTextById(this.theModel, (txt.Param.TypeId || 0))
      this.typeTitle = Mdf.descrOfDescrNote(t)
      if ((this.typeTitle || '') === '') this.typeTitle = t.Type.Name || ''

      this.paramDescr = Mdf.descrOfDescrNote(txt)
      this.paramNote = Mdf.noteOfDescrNote(txt)
      this.paramDigest = txt.Param.Digest || ''

      this.$refs.noteDlg.open()   // show param info dialog
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
