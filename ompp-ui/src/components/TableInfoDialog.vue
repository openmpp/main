<!-- output table info dialog -->
<template>

<om-mcw-dialog :id="id" ref="noteDlg" :scrollable="true" acceptText="OK">
  <span slot="header">{{tableDescr}}</span>
  <div v-if="(tableNote !== '')">{{tableNote}}</div>
  <div v-if="(exprDescr !== '' || exprNote || '' !== '')">
    <br v-if="(tableNote !== '')"/>
    <div>{{exprDescr}}</div>
    <div>{{exprNote}}</div>
  </div>
  <br/>
  <div class="note-table mono">
    <div class="note-row">
      <span class="note-cell">Name:</span><span class="note-cell">{{tableName}}</span>
    </div>
    <div v-if="tableSize.rank !== 0" class="note-row">
      <span class="note-cell">Dimensions:</span><span class="note-cell">{{tableSize.dimSize}}</span>
    </div>
    <div v-else class="note-row">
      <span class="note-cell">Rank:</span><span class="note-cell">{{tableSize.rank}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell">Expressions:</span><span class="note-cell">{{tableSize.exprCount}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell">Accumulators:</span><span class="note-cell">{{tableSize.accCount}}</span>
    </div>
    <div v-if="isSubCount" class="note-row">
      <span class="note-cell">SubValues:</span><span class="note-cell">{{subCount}}</span>
    </div>
    <div class="note-row">
      <span class="note-cell">Digest:</span><span class="note-cell">{{tableDigest}}</span>
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
      tableName: '',
      subCount: 0,
      tableDescr: '',
      tableNote: '',
      exprDescr: '',
      exprNote: '',
      tableDigest: '',
      isSubCount: false,
      tableSize: Mdf.emptyTableSize()
    }
  },

  computed: {
    ...mapGetters({
      theModel: GET.THE_MODEL
    })
  },

  methods: {
    showTableInfo (name, nSub) {
      if ((name || '') === '') {
        console.log('Empty output table name')
        return
      }
      this.tableName = name
      this.subCount = nSub || 0

      // find table and table size and table text info
      let txt = Mdf.tableTextByName(this.theModel, this.tableName)
      this.tableSize = Mdf.tableSizeByName(this.theModel, this.tableName)
      this.isSubCount = (this.subCount || 0) > 0

      this.tableDescr = txt.TableDescr || ''
      this.tableNote = txt.TableNote || ''
      this.exprDescr = txt.ExprDescr || ''
      this.exprNote = txt.ExprNote || ''
      this.tableDigest = txt.Table.Digest || ''

      this.$refs.noteDlg.open() // show table info dialog
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
