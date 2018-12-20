<!-- parameter info dialog -->
<template>

<om-mcw-dialog :id="id" ref="noteDlg" :scrollable="true" acceptText="OK">
  <span slot="header">{{paramDescr}}</span>
  <div v-if="(paramNote !== '')">{{paramNote}}</div>
  <br/>
  <div class="note-table mono">
    <div class="note-row">
      <span class="note-cell">Name:</span><span class="note-cell">{{paramName}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell">Type:</span><span class="note-cell">{{typeTitle}}</span>
    </div>
    <div v-if="paramSize.rank !== 0" class="note-row">
      <span class="note-cell">Dimensions:</span><span class="note-cell">{{paramSize.dimSize}}</span>
    </div>
    <div v-else class="note-row">
      <span class="note-cell">Rank:</span><span class="note-cell">{{paramSize.rank}}</span>
    </div>
    <div v-if="isSubCount" class="note-row">
      <span class="note-cell">SubValues:</span><span class="note-cell">{{subCount}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell">Digest:</span><span class="note-cell">{{paramDigest}}</span>
    </div>
  </div>
</om-mcw-dialog>

</template>

<script>
import { mapGetters } from 'vuex'
import { GET } from '@/store'
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

      this.$refs.noteDlg.open() // show param info dialog
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
