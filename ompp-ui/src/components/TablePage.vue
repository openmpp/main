<template>

<div id="table-page" class="main-container mdc-typography  mdc-typography--body1">

  <div v-if="loadDone" class="hdr-row mdc-typography--body1">
    
    <span
      @click="showTableInfo()" 
      class="cell-icon-link material-icons" 
      :alt="tableName + ' info'"
      :title="tableName + ' info'">event_note</span>
    <span class="mdc-typography--body2">{{ tableName }}: </span>
    <span>{{ tableDescr() }}</span>

  </div>
  <div v-else class="hdr-row mdc-typography--body2">
    <span class="cell-icon-link material-icons" aria-hidden="true">refresh</span>
    <span v-if="loadWait" class="material-icons om-mcw-spin">star</span>
    <span class="mdc-typography--body2">{{msg}}</span>
  </div>

  <div class="ht-container">
    <HotTable ref="ht" :root="htRoot" :settings="htSettings"></HotTable>
  </div>

  <om-mcw-dialog ref="noteDlg" id="note-dlg" acceptText="OK">
    <span slot="header">{{titleNoteDlg}}</span>
    <div>{{textNoteDlg}}</div>
    <div v-if="((tableText.ExprDescr || '') !== '' || (tableText.ExprNote || '') !== '')">
      <br v-if="((textNoteDlg || '') !== '')"/>
      <div>{{tableText.ExprDescr || ''}}</div>
      <div>{{tableText.ExprNote || ''}}</div>
    </div>
    <br/>
    <div class="note-table">
      <div class="note-row">
        <span class="note-cell mono">Name:</span><span class="note-cell mono">{{tableName}}</span>
      </div>
      <div v-if="tableSize.rank !== 0" class="note-row">
        <span class="note-cell mono">Dimensions:</span><span class="note-cell mono">{{tableSize.dimSize}}</span>
      </div>
      <div v-else class="note-row">
        <span class="note-cell mono">Rank:</span><span class="note-cell mono">{{tableSize.rank}}</span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">Expressions:</span><span class="note-cell mono">{{tableSize.exprCount}}</span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">Accumulators:</span><span class="note-cell mono">{{tableSize.accCount}}</span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">SubValues:</span><span class="note-cell mono">{{subCount}}</span>
      </div>
      <div class="note-row">
        <span class="note-cell mono">Digest:</span><span class="note-cell mono">{{tableText.Table.Digest}}</span>
      </div>
    </div>
  </om-mcw-dialog>

</div>
  
</template>

<script>
import axios from 'axios'
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import { default as Mdf } from '@/modelCommon'
import OmMcwDialog from './OmMcwDialog'
import HotTable from '@/vue-handsontable-official/src/HotTable'

const kind = {
  EXPR: 0,  // output table expression(s)
  ACC: 1,   // output table accumulator(s)
  ALL: 2    // output table all-accumultors view
}

export default {
  props: {
    digest: '',
    tableName: '',
    refreshTickle: false
  },

  data () {
    return {
      loadDone: false,
      loadWait: false,
      titleNoteDlg: '',
      textNoteDlg: '',
      tableText: Mdf.emptyTableText(),
      tableSize: Mdf.emptyTableSize(),
      subCount: 0,
      htRoot: '',
      htSettings: {
        manualColumnMove: true,
        manualColumnResize: true,
        manualRowResize: true,
        preventOverflow: 'horizontal',
        renderAllRows: true,
        stretchH: 'all',
        rowHeaders: true,
        fillHandle: false,
        readOnly: true,
        colHeaders: [],
        data: []
      },
      dimProp: [],
      exprProp: [],
      accProp: [],
      totalEnumLabel: '',
      kind: kind.EXPR,
      pageStart: 0,
      pageSize: 20,
      isNextPage: false,
      msg: ''
    }
  },

  computed: {
    ...mapGetters({
      theModel: GET.THE_MODEL,
      theRunText: GET.THE_RUN_TEXT,
      wordList: GET.WORD_LIST,
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  watch: {
    // refresh button handler
    refreshTickle () {
      this.doRefreshDataPage()
    }
  },

  methods: {
    tableDescr () { return this.tableText.TableDescr || '' },

    // show table info
    showTableInfo () {
      this.titleNoteDlg = this.tableText.TableDescr || ''
      this.textNoteDlg = this.tableText.TableNote || ''
      this.$refs.noteDlg.open()
    },

    // update table data from response data page
    setData (d) {
      // if response is empty or invalid: clean table
      let len = (!!d && (d.length || 0) > 0) ? d.length : 0
      if (!d || len <= 0) {
        this.htSettings.data = []
        return
      }
      if (this.isNextPage) len--  // if page size limited and response row count > limit

      // set page data
      switch (this.kind) {
        case kind.EXPR:
          this.htSettings.data = this.makeExprPage(len, d)
          break
        case kind.ACC:
          this.htSettings.data = this.makeAccPage(len, d)
          break
        case kind.ALL:
          this.htSettings.data = this.makeAllAccPage(len, d)
          break
        default:
          this.htSettings.data = []
          console.log('Invalid kind of table view: must be or of: EXPR, ACC, ALL')
      }
    },

    // make expression data page, columns are: dimensions, expression id, value
    makeExprPage (dataLen, d) {
      const vp = []

      for (let i = 0; i < dataLen; i++) {
        let row = []
        for (let j = 0; j < this.tableSize.rank; j++) {
          row.push(
            this.translateDimEnumId(j, d[i].DimIds[j]) || d[i].DimIds[j]
          )
        }
        row.push(this.translateExprId(d[i].ExprId) || d[i].ExprId)
        row.push(!d[i].IsNull ? d[i].Value : void 0)
        vp.push(row)
      }
      return vp
    },

    // make accumulators data page, columns are: dimensions, accumulator id, sub-values
    // accumulator sub-values are separate rows in source rowset
    makeAccPage (dataLen, d) {
      const vp = []

      let nowKey = '?'  // non-existent start key to enforce append of the first row
      let n = 0
      const nSub0 = this.tableSize.rank + 1   // first sub-value position

      for (let i = 0; i < dataLen; i++) {
        let sk = [d[i].DimIds, d[i].AccId].toString() // current row key

        if (sk === nowKey) {  // same key: set sub-value at position of sub_id
          //
          if (!d[i].IsNull) vp[n][nSub0 + (d[i].SubId || 0)] = d[i].Value
        } else {
          // make new row
          let row = []
          for (let j = 0; j < this.tableSize.rank; j++) {
            row.push(
              this.translateDimEnumId(j, d[i].DimIds[j]) || d[i].DimIds[j]
            )
          }
          row.push(this.translateAccId(d[i].AccId) || d[i].AccId)
          for (let j = 0; j < this.subCount; j++) {   // append empty sub-values
            row.push(void 0)
          }

          // append new row to page data
          if (!d[i].IsNull) row[nSub0 + (d[i].SubId || 0)] = d[i].Value // set current sub-value
          n = vp.length
          vp.push(row)
          nowKey = sk
        }
      }
      return vp
    },

    // make all-accumulators view page, columns are: dimensions, accumulator id, sub-values
    // accumulator sub-values are separate rows in source rowset
    makeAllAccPage (dataLen, d) {
      const vp = []

      let dKey = '?'  // non-existent start key to enforce append of the first row
      let nf = 0
      const nSub0 = this.tableSize.rank + 1   // first sub-value position

      for (let i = 0; i < dataLen; i++) {
        let sk = [d[i].DimIds].toString() // current dimensions key

        if (sk === dKey) {  // same dimensions: for each accumulator set sub-value at position of sub_id
          //
          for (let k = 0; k < this.tableSize.allAccCount; k++) {
            if (!d[i].IsNull[k]) vp[nf + k][nSub0 + (d[i].SubId || 0)] = d[i].Value[k]
          }
        } else {
          // make new rows: one row per accumulator
          nf = vp.length
          dKey = sk

          for (let k = 0; k < this.tableSize.allAccCount; k++) {
            let row = []
            for (let j = 0; j < this.tableSize.rank; j++) {
              row.push(
                this.translateDimEnumId(j, d[i].DimIds[j]) || d[i].DimIds[j]
              )
            }
            row.push(this.translateAccId(k) || k)
            for (let j = 0; j < this.subCount; j++) {   // append empty sub-values
              row.push(void 0)
            }
            // append new row to page data
            if (!d[i].IsNull[k]) row[nSub0 + (d[i].SubId || 0)] = d[i].Value[k] // set current sub-value
            vp.push(row)
          }
        }
      }
      return vp
    },

    // translate dimension enum id to label, it does return enum code if label is empty
    // return total enum label if total enabled for dimension and id is total enum id
    translateDimEnumId (dimIdx, enumId) {
      if (this.dimProp[dimIdx].isTotal && enumId === this.dimProp[dimIdx].totalId) return this.totalEnumLabel
      return Mdf.enumDescrOrCodeById(this.dimProp[dimIdx].typeText, enumId)
    },

    // translate expression id to label or name if label is empty
    // it is expected to be expression id === expression index
    translateExprId (exprId) {
      return (exprId !== void 0 && exprId !== null && exprId >= 0 && exprId < this.exprProp.length)
        ? this.exprProp[exprId].label || this.exprProp[exprId].name
        : ''
    },

    // translate accumulator id to label or name if label is empty
    // it is expected to be accumulator id === accumulator index
    translateAccId (accId) {
      return (accId !== void 0 && accId !== null && accId >= 0 && accId < this.accProp.length)
        ? this.accProp[accId].label || this.accProp[accId].name
        : ''
    },

// get page of table data from current model run: expressions or accumulators
    async doRefreshDataPage () {
      this.loadDone = false
      this.loadWait = true
      this.msg = 'Loading...'

      // exit if model run: must be found (not found run is empty)
      if (!Mdf.isNotEmptyRunText(this.theRunText)) {
        this.msg = 'Model run is not completed'
        console.log('Model run is not completed (empty)')
        this.loadWait = false
        return
      }

      // make output table read layout and url
      let layout = this.makeSelectLayout()
      let u = this.omppServerUrl +
        '/api/model/' + (this.digest || '') + '/run/' + (this.theRunText.Digest || '') + '/table/value-id'

      // retrieve page from server and check if next page exist
      try {
        const response = await axios.post(u, layout)
        const d = response.data
        this.isNextPage = this.pageSize > 0 && ((!!d && (d.length || 0) > 0) ? d.length : 0) >= layout.Size
        this.setData(d)     // update table page
        this.loadDone = true
      } catch (e) {
        this.msg = 'Server offline or no models published'
        console.log('Server offline or no run completed')
      }
      this.loadWait = false
    },

    // return page layout to read table data
    makeSelectLayout () {
      //
      // if page size limited then make size to select at least page size +1
      // for accumulators each sub-value is a separate row
      let nSize = this.pageSize * ((this.kind === kind.ACC || this.kind === kind.ALL) ? this.subCount : 1)
      let layout = {
        Name: this.tableName,
        Offset: (this.pageSize > 0 ? this.pageStart : 0),
        Size: (this.pageSize > 0 ? 1 + nSize : 0),
        Filter: [],
        OrderBy: [],
        IsAccum: (this.kind === kind.ACC || this.kind === kind.ALL),
        IsAllAccum: this.kind === kind.ALL
      }

      // make deafult order by
      // expressions:      SELECT expr_id, dim0, dim1, value...        ORDER BY 2, 3, 1
      // accumulators:     SELECT acc_id, sub_id, dim0, dim1, value... ORDER BY 3, 4, 1, 2
      // all-accumulators: SELECT sub_id, dim0, dim1, acc0, acc1...    ORDER BY 2, 3, 1
      let n = this.kind === kind.ACC ? 3 : 2
      for (let k = 0; k < this.tableSize.rank; k++) {
        layout.OrderBy.push({IndexOne: k + n})
      }
      layout.OrderBy.push({IndexOne: 1})
      if (this.kind === kind.ACC) layout.OrderBy.push({IndexOne: 2})

      return layout
    }
  },

  mounted () {
    this.$emit('tab-mounted', 'table', this.tableName)

    // find table and table size, including run sub-values count
    this.tableText = Mdf.tableTextByName(this.theModel, this.tableName)
    this.tableSize = Mdf.tableSizeByName(this.theModel, this.tableName)
    this.subCount = this.theRunText.SubCount || 0
    this.totalEnumLabel = Mdf.wordByCode(this.wordList, Mdf.ALL_WORD_CODE)

    // find dimension type for each dimension
    this.dimProp = []
    for (let j = 0; j < this.tableSize.rank; j++) {
      if (this.tableText.TableDimsTxt[j].hasOwnProperty('Dim')) {
        let t = Mdf.typeTextById(this.theModel, (this.tableText.TableDimsTxt[j].Dim.TypeId || 0))
        this.dimProp.push({
          name: this.tableText.TableDimsTxt[j].Dim.Name || '',
          label: Mdf.descrOfDescrNote(this.tableText.TableDimsTxt[j]),
          isTotal: this.tableText.TableDimsTxt[j].Dim.IsTotal,
          totalId: t.Type.TotalEnumId || 0,
          typeText: t
        })
      } else {
        this.dimProp.push({
          name: '', label: '', isTotal: false, totalId: 0, typeText: Mdf.emptyTypeText() })
      }
    }

    // expression labels
    this.exprProp = []
    for (let j = 0; j < this.tableText.TableExprTxt.length; j++) {
      if (this.tableText.TableExprTxt[j].hasOwnProperty('Expr')) {
        this.exprProp.push({
          name: this.tableText.TableExprTxt[j].Expr.Name || '',
          label: Mdf.descrOfDescrNote(this.tableText.TableExprTxt[j])
        })
      } else {
        this.exprProp.push({ name: '', label: '' })
      }
    }

    // accumultor labels
    this.accProp = []
    for (let j = 0; j < this.tableText.TableAccTxt.length; j++) {
      if (this.tableText.TableAccTxt[j].hasOwnProperty('Acc')) {
        this.accProp.push({
          name: this.tableText.TableAccTxt[j].Acc.Name || '',
          label: Mdf.descrOfDescrNote(this.tableText.TableAccTxt[j])
        })
      } else {
        this.accProp.push({ name: '', label: '' })
      }
    }

    // columns layout: dimensions, expression or accumulator dimension, value or sub-values
    this.htSettings.colHeaders = []
    for (let j = 0; j < this.tableSize.rank; j++) {
      this.htSettings.colHeaders.push(this.dimProp[j].label || this.dimProp[j].name)
    }
    if (this.kind === kind.EXPR) {
      this.htSettings.colHeaders.push(this.tableText.ExprDescr || 'Measure')  // expression dimension
      this.htSettings.colHeaders.push('Value')          // expression value
    } else {
      this.htSettings.colHeaders.push(this.tableText.ExprDescr || 'Measure')  // accumulator dimension
      for (let j = 0; j < this.subCount; j++) {
        this.htSettings.colHeaders.push(j.toString())   // column for each sub-value
      }
    }

    // this.htRoot = 'ht-' + this.digest + '-' + this.tableName + '-' + (this.theRunText.Digest || '')
    this.doRefreshDataPage()
  },

  components: { OmMcwDialog, HotTable }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
  @import "handsontable/dist/handsontable.full.css";

  /* main container, header row and data table */
  .main-container {
    height: 100%;
    flex: 1 1 auto;
    display: flex; 
    flex-direction: column;
    overflow-y: hidden;
  }
  .hdr-row {
    flex: 0 0 auto;
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
    margin-top: 0.5rem;
    margin-bottom: 0.5rem;
  }
  .ht-container {
    flex: 1 1 auto;
    overflow: auto;
  }

  /* cell material icon: a link or empty (non-link) */
  .cell-icon {
    vertical-align: middle;
    margin: 0;
    padding-left: 0.5rem;
    padding-right: 0.5rem;
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
    @extend .mdc-theme--text-disabled-on-background;
  }

  /* note dialog, fix handsontable z-index: 101; */
  #note-dlg {
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
