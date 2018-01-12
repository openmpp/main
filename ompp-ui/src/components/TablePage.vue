<template>

<div id="table-page" class="main-container mdc-typography  mdc-typography--body1">

  <div v-if="loadDone" class="hdr-row mdc-typography--body1">
    
    <span
      @click="showTableInfo()" 
      class="cell-icon-link material-icons" :alt="tableName + ' info'" :title="tableName + ' info'">event_note</span>

    <span v-if="tv.isPrev">
      <span
        @click="doFirstPage()"
        class="cell-icon-link material-icons" title="First page" alt="First page">first_page</span>
      <span
        @click="doPrevPage()"
        class="cell-icon-link material-icons" title="Previous page" alt="Previous page">navigate_before</span>
    </span>
    <span v-else>
      <span
        class="cell-icon-empty material-icons" title="First page" alt="First page">first_page</span>
      <span
        class="cell-icon-empty material-icons" title="Previous page" alt="Previous page">navigate_before</span>
    </span>

    <span v-if="tv.isNext">
      <span
        @click="doNextPage()"
        class="cell-icon-link material-icons" title="Next page" alt="Next page">navigate_next</span>
      <span
        @click="doLastPage()"
        class="cell-icon-link material-icons" title="Last page" alt="Last page">last_page</span>
    </span>
    </span>
    <span v-else>
      <span
        class="cell-icon-empty material-icons" title="Next page" alt="Next page">navigate_next</span>
      <span
        class="cell-icon-empty material-icons" title="Last page" alt="Last page">last_page</span>
    </span>

    <span
      @click="toggleMoreControls()"
      class="cell-icon-link material-icons" :title="moreControlsLabel" :alt="moreControlsLabel">more_horiz</span>

    <span class="mdc-typography--body2">{{ tableName }}: </span>
    <span>{{ tableDescr() }}</span>

  </div>
  <div v-else class="hdr-row mdc-typography--body2">
    <span class="cell-icon-link material-icons" aria-hidden="true">refresh</span>
    <span v-if="loadWait" class="material-icons om-mcw-spin">star</span>
    <span class="mdc-typography--body2">{{msg}}</span>
  </div>

  <div v-if="isShowMoreControls" class="hdr-row mdc-typography--body2">

    <span
      @click="doResetView()"
      class="cell-icon-link material-icons" title="Reset table view to default" alt="Reset table view to default">settings_backup_restore</span>

    <span v-if="tv.isInc">
      <span
        @click="doIncreasePage()"
        class="cell-icon-link material-icons" title="Increase page size" alt="Increase page size">format_line_spacing</span>
    </span>
    <span v-else>
      <span
        class="cell-icon-empty material-icons" title="Increase page size" alt="Increase page size">format_line_spacing</span>
    </span>

    <span v-if="tv.isDec">
      <span
        @click="doDecreasePage()"
        class="cell-icon-link material-icons" title="Decrease page size" alt="Decrease page size">vertical_align_center</span>
    </span>
    <span v-else>
      <span
        class="cell-icon-empty material-icons" title="Decrease page size" alt="Decrease page size">vertical_align_center</span>
    </span>

    <span
      @click="doUnlimitedPage()"
      class="cell-icon-link material-icons" title="Unlimited page size, show all data" alt="Unlimited page size, show all data">all_inclusive</span>
    <span
      @click="doExpressionPage()"
      class="cell-icon-link material-icons" title="View table expressions"alt="View table expressions">filter_none</span>
    <span
      @click="doAccumulatorPage()"
      class="cell-icon-link material-icons" title="View accumulators and sub-values"alt="View accumulators and sub-values">filter_8</span>
    <span
      @click="doAllAccumulatorPage()"
      class="cell-icon-link material-icons" title="View all accumulators and sub-values"alt="View all accumulators and sub-values">library_add</span>
    <span
      @click="doRefresh()"
      class="cell-icon-link material-icons" title="Refresh"alt="Refresh">refresh</span>
  </div>

  <div class="ht-container">
    <HotTable ref="ht" :root="htRoot" :settings="htSettings"></HotTable>
  </div>

  <table-info-dialog ref="noteDlg" id="table-note-dlg"></table-info-dialog>

</div>
  
</template>

<script>
import axios from 'axios'
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import { default as Mdf } from '@/modelCommon'
import TableInfoDialog from './TableInfoDialog'
import HotTable from '@/vue-handsontable-official/src/HotTable'

const kind = {
  EXPR: 0,  // output table expression(s)
  ACC: 1,   // output table accumulator(s)
  ALL: 2    // output table all-accumultors view
}
const MAX_PAGE_SIZE = 65536
const MAX_PAGE_OFFSET = (MAX_PAGE_SIZE * MAX_PAGE_SIZE)
const SHOW_MORE_LABEL = 'Show more controls'
const HIDE_MORE_LABEL = 'Hide extra controls'

export default {
  components: { TableInfoDialog, HotTable },

  props: {
    digest: '',
    tableName: '',
    refreshTickle: false
  },

  data () {
    return {
      loadDone: false,
      loadWait: false,
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
      tv: {
        kind: kind.EXPR,
        start: 0,
        size: 20,
        isNext: false,
        isPrev: false,
        isInc: false,
        isDec: false
      },
      isShowMoreControls: false,
      moreControlsLabel: SHOW_MORE_LABEL,
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
    // parent refresh button handler
    refreshTickle () {
      this.doRefreshDataPage()
    }
  },

  methods: {
    tableDescr () { return this.tableText.TableDescr || '' },

    // show table info
    showTableInfo () {
      this.$refs.noteDlg.showTableInfo(this.tableName, this.subCount)
    },

    // local refresh button handler, table content only
    doRefresh () {
      this.doRefreshDataPage()
    },

    // show or hide extra controls
    toggleMoreControls () {
      this.isShowMoreControls = !this.isShowMoreControls
      this.moreControlsLabel = this.isShowMoreControls ? HIDE_MORE_LABEL : SHOW_MORE_LABEL
    },

    // move to previous page
    doPrevPage () {
      if (this.tv.start === 0) return
      if (this.tv.start > 0) this.tv.start -= this.tv.size
      if ((this.tv.start || 0) <= 0) this.tv.start = 0
      this.doRefreshDataPage()
    },
    // move to next page
    doNextPage () {
      this.tv.start = this.tv.start + this.tv.size
      this.doRefreshDataPage()
    },
    // move to first page
    doFirstPage () {
      this.tv.start = 0
      this.doRefreshDataPage()
    },
    // move to last page
    doLastPage () {
      this.tv.start = MAX_PAGE_OFFSET
      this.doRefreshDataPage()
    },
    // increase page size
    doIncreasePage () {
      this.tv.size = this.tv.size >= 10 ? this.tv.size *= 2 : this.tv.size + 1
      if (this.tv.size > MAX_PAGE_SIZE) this.tv.size = MAX_PAGE_SIZE
      this.doRefreshDataPage()
    },
    // decrease page size
    doDecreasePage () {
      if (this.tv.size > 0) {
        this.tv.size = this.tv.size > 10 ? Math.floor(this.tv.size / 2) : this.tv.size - 1
      } else {
        let len = (this.htSettings.rowHeaders.length || 0) > 0 ? this.htSettings.rowHeaders.length : 0
        if (len <= 1) {
          this.tv.isDec = false
          return
        }
        // page size unlimited and last row > 0
        this.tv.size = len > 10 ? Math.floor(len / 2) : len - 1
      }
      if (this.tv.size < 1) this.tv.size = 1
      this.doRefreshDataPage()
    },
    // unlimited page size, show all data
    doUnlimitedPage () {
      this.tv.start = 0
      this.tv.size = 0
      this.doRefreshDataPage()
    },
    // show output table expressions
    doExpressionPage () {
      this.tv.kind = kind.EXPR
      this.setExprColHeaders()
      this.doRefreshDataPage()
    },
    // show output table accumulators
    doAccumulatorPage () {
      this.tv.kind = kind.ACC
      this.setAccColHeaders()
      this.doRefreshDataPage()
    },
    // show all-accumulators view
    doAllAccumulatorPage () {
      this.tv.kind = kind.ALL
      this.setAccColHeaders()
      this.doRefreshDataPage()
    },
    // reset table view to default
    doResetView () {
      this.setDefaultPageView()
      this.doRefreshDataPage()
    },

    // update table data from response data page
    setData (d) {
      // if response is empty or invalid: clean table
      let len = (!!d && (d.length || 0) > 0) ? d.length : 0
      if (!d || len <= 0) {
        this.htSettings.data = ['none']
        return
      }

      // set page data
      switch (this.tv.kind) {
        case kind.EXPR:
          this.htSettings.data = this.makeExprPage(len, d)
          this.htSettings.rowHeaders = this.makeRowHeaders(len, 1)
          break
        case kind.ACC:
          this.htSettings.data = this.makeAccPage(len, d)
          this.htSettings.rowHeaders = this.makeRowHeaders(len, 1)
          break
        case kind.ALL:
          this.htSettings.data = this.makeAllAccPage(len, d)
          this.htSettings.rowHeaders = this.makeRowHeaders(len, this.tableSize.allAccCount)
          break
        default:
          this.htSettings.data = []
          this.htSettings.rowHeaders = true
          console.log('Invalid kind of table view: must be or of: EXPR, ACC, ALL')
      }
    },

    // make expression data page, columns are: dimensions, expression id, value
    makeExprPage (len, d) {
      const vp = []

      for (let i = 0; i < len; i++) {
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
    // each sub-value is a separate row in source rowset
    makeAccPage (len, d) {
      const vp = []

      let nowKey = '?'  // non-existent start key to enforce append of the first row
      let n = 0
      const nSub0 = this.tableSize.rank + 1   // first sub-value position

      for (let i = 0; i < len; i++) {
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

    // make all-accumulators view page, columns are: dimensions, sub-values
    // each sub-value is a separate row in source rowset
    // each row has all accumulators
    makeAllAccPage (len, d) {
      const vp = []

      let dKey = '?'  // non-existent start key to enforce append of the first row
      let nf = 0
      const nSub0 = this.tableSize.rank + 1   // first sub-value position

      for (let i = 0; i < len; i++) {
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

    // make table row headers, each data row can contain multiple table rows.
    // in case of all-accumulators view each data row produce one table row for each accumulator
    makeRowHeaders (len, rowSize) {
      if ((!!len || 0) <= 0) return ['none']
      const rh = []
      let n = this.tv.start * rowSize
      if (n < 0) n = 0
      for (let k = 0; k < len * rowSize; k++) {
        rh.push(n + k + 1)
      }
      return rh
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
      this.tv.isNext = false
      this.tv.isPrev = false
      this.tv.isInc = false
      this.tv.isDec = false

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

      // retrieve page from server, it must be: {Layout: {...}, Page: [...]}
      try {
        const response = await axios.post(u, layout)
        const rsp = response.data
        let d = []
        if (!!rsp && rsp.hasOwnProperty('Page')) {
          if ((rsp.Page.length || 0) > 0) d = rsp.Page
        }
        let lt = {Offset: 0, Size: 0, IsLastPage: true}
        if (!!rsp && rsp.hasOwnProperty('Layout')) {
          if ((rsp.Layout.Offset || 0) > 0) lt.Offset = rsp.Layout.Offset || 0
          if ((rsp.Layout.Size || 0) > 0) lt.Size = rsp.Layout.Size || 0
          if (rsp.Layout.hasOwnProperty('IsLastPage')) lt.IsLastPage = !!rsp.Layout.IsLastPage
        }

        // set page view state: is next page exist
        this.tv.start = lt.Offset
        if (this.tv.kind !== kind.EXPR) this.tv.start = Math.floor(lt.Offset / this.subCount)
        this.tv.isNext = !lt.IsLastPage
        this.tv.isPrev = lt.Offset > 0
        this.tv.isInc = this.tv.isNext && this.tv.size < MAX_PAGE_SIZE
        this.tv.isDec = this.tv.size > 1
        if (this.tv.size === 0 && lt.Size > 1) {
          this.tv.isDec = this.tv.kind === kind.EXPR || lt.Size > 2 * this.subCount
        }

        // update table page
        this.setData(d)
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
      // for accumulators each sub-value is a separate row
      let nStart = this.tv.start
      let nSize = this.tv.size
      if (this.tv.kind !== kind.EXPR) {
        nStart *= this.subCount
        nSize *= this.subCount
      }
      let layout = {
        Name: this.tableName,
        Offset: (this.tv.size > 0 ? nStart : 0),
        Size: (this.tv.size > 0 ? nSize : 0),
        IsLastPage: true,
        Filter: [],
        OrderBy: [],
        IsAccum: (this.tv.kind !== kind.EXPR),
        IsAllAccum: this.tv.kind === kind.ALL
      }

      // make deafult order by
      // expressions:      SELECT expr_id, dim0, dim1, value...        ORDER BY 2, 3, 1
      // accumulators:     SELECT acc_id, sub_id, dim0, dim1, value... ORDER BY 3, 4, 1, 2
      // all-accumulators: SELECT sub_id, dim0, dim1, acc0, acc1...    ORDER BY 2, 3, 1
      let n = this.tv.kind === kind.ACC ? 3 : 2
      for (let k = 0; k < this.tableSize.rank; k++) {
        layout.OrderBy.push({IndexOne: k + n})
      }
      layout.OrderBy.push({IndexOne: 1})
      if (this.tv.kind === kind.ACC) layout.OrderBy.push({IndexOne: 2})

      return layout
    },

    // set default page view parameters
    setDefaultPageView () {
      this.tv.kind = kind.EXPR
      this.tv.start = 0
      this.tv.size = Math.min(20, this.tableSize.dimTotal * this.tableSize.exprCount)
      this.setExprColHeaders()
    },
    // column headers for output table expressions
    setExprColHeaders () {
      this.htSettings.colHeaders = []
      for (let j = 0; j < this.tableSize.rank; j++) {
        this.htSettings.colHeaders.push(this.dimProp[j].label || this.dimProp[j].name)
      }
      this.htSettings.colHeaders.push(this.tableText.ExprDescr || 'Measure')  // expression dimension
      this.htSettings.colHeaders.push('Value')          // expression value
    },
    // column headers for accumulators or all accumulators veiw
    setAccColHeaders () {
      this.htSettings.colHeaders = []
      for (let j = 0; j < this.tableSize.rank; j++) {
        this.htSettings.colHeaders.push(this.dimProp[j].label || this.dimProp[j].name)
      }
      this.htSettings.colHeaders.push(this.tableText.ExprDescr || 'Measure')  // accumulator dimension
      for (let j = 0; j < this.subCount; j++) {
        this.htSettings.colHeaders.push(j.toString())   // column for each sub-value
      }
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

    // set columns layout and refresh the data
    this.setDefaultPageView()
    this.doRefreshDataPage()
  }
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
  }
  .ht-container {
    flex: 1 1 auto;
    overflow: auto;
    margin-top: 0.5rem;
  }

  /* cell material icon: a link or empty (non-link) */
  .cell-icon {
    vertical-align: middle;
    margin: 0;
    padding-left: 0;
    padding-right: 0;
    -webkit-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
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
    @extend .mdc-theme--primary-light-bg;
    @extend .mdc-theme--text-primary-on-primary;
    /* @extend .mdc-theme--text-disabled-on-background; */
  }
</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
