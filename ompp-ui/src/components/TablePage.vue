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
      tableSize: Mdf.emptyTableSize(),
      subCount: 0,
      isAcc: false,
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

      // set dimension(s) items and expression(s) or accumulator(s) values
      // put accumulator sub-values as extra columns
      // accumulator sub-values not adjacent, default order by: acc_id, sub_id, dim0, dim1
      const vp = []
      if (!this.isAcc) {    // dimension(s) items and expression(s) values
        //
        for (let i = 0; i < len; i++) {
          let row = []
          for (let j = 0; j < this.tableSize.rank; j++) {
            row.push(d[i].Dims[j])
          }
          row.push(d[i].ExprId)
          row.push(!d[i].IsNull ? d[i].Value : void 0)
          vp.push(row)
        }
      } else {  // dimension(s) items and accumulator(s) values: sub-values as columns
        //
        let ak = []   // array of row keys: each key is string of [dimensions, acc_id]
        let subOffset = this.tableSize.rank + 1

        for (let i = 0; i < len; i++) {
          // make row key and search it
          let sk = [d[i].Dims, d[i].AccId].toString()
          let n = ak.indexOf(sk)

          if (n >= 0) {   // key found: set sub-value at position of sub_id
            vp[n][subOffset + (d[i].SubId || 0)] = (!d[i].IsNull ? d[i].Value : void 0)
          } else {
            // append new row
            let row = []
            for (let j = 0; j < this.tableSize.rank; j++) {
              row.push(d[i].Dims[j])
            }
            row.push(d[i].AccId)
            row[subOffset + (d[i].SubId || 0)] = (!d[i].IsNull ? d[i].Value : void 0)
            vp.push(row)
            ak.push(sk)
          }
        }
      }
      this.htSettings.data = vp
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

      // make url: table name, expression or accumulators, table start row and rows count
      let nVal = !this.isAcc ? this.tableSize.exprCount : this.tableSize.accCount * this.subCount
      let u = this.omppServerUrl +
        '/api/model/' + (this.digest || '') +
        '/run/' + (this.theRunText.Digest || '') +
        '/table/' + this.tableName +
        (!this.isAcc ? '/expr' : '/acc')
      if ((this.pageSize || 0) > 0) {
        u +=
          '/start/' + (this.pageStart || 0).toString() +
          '/count/' + (1 + this.pageSize * nVal).toString()
      }

      // retrieve page from server and check if next page exist
      try {
        const response = await axios.get(u)
        const d = response.data
        this.isNextPage = this.pageSize > 0 && this.pageSize * nVal < ((!!d && (d.length || 0) > 0) ? d.length : 0)
        this.setData(d)     // update table page
        this.loadDone = true
      } catch (e) {
        this.msg = 'Server offline or no models published'
        console.log('Server offline or no run completed')
      }
      this.loadWait = false
    }
  },

  mounted () {
    this.$emit('tab-mounted', 'table', this.tableName)

    // find table and table size, including run sub-values count
    this.tableText = Mdf.tableTextByName(this.theModel, this.tableName)
    this.tableSize = Mdf.tableSizeByName(this.theModel, this.tableName)
    this.subCount = this.theRunText.SubCount || 0

    // set columns layout
    this.htSettings.colHeaders = []
    for (let j = 0; j < this.tableSize.rank; j++) {
      let name = this.tableText.TableDimsTxt[j].hasOwnProperty('Dim') ? (this.tableText.TableDimsTxt[j].Dim.Name || '') : ''
      this.htSettings.colHeaders.push(name)
    }
    if (!this.isAcc) {
      this.htSettings.colHeaders.push('ExprId', 'Value')  // expression id and value
    } else {
      this.htSettings.colHeaders.push('AccId')
      for (let j = 0; j < this.subCount; j++) {
        this.htSettings.colHeaders.push(j.toString())     // column for each sub-value
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
