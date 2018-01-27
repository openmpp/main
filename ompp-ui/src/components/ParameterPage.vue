<template>

<div id="parameter-page" class="main-container mdc-typography mdc-typography--body1">

  <div v-if="loadDone && saveDone" class="hdr-row mdc-typography--body1">
    
    <span
      @click="showParamInfo()" 
      class="cell-icon-link material-icons" :alt="paramName + ' info'" :title="paramName + ' info'">event_note</span>

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

    <span v-if="isEdit">
      <span v-if="isEditUpdated"
        @click="doSave()" 
        class="cell-icon-link material-icons" :alt="'Save ' + paramName" :title="'Save ' + paramName">save</span>
      <span v-else
        class="cell-icon-empty material-icons" title="Save" alt="Save">save</span>
    </span>

    <span
      @click="toggleMoreControls()"
      class="cell-icon-link material-icons" :title="moreControlsLabel" :alt="moreControlsLabel">more_horiz</span>

    <span v-if="isShowMoreControls">

      <span
        @click="doResetView()"
        class="cell-icon-link material-icons" title="Reset parameter view to default" alt="Reset parameter view to default">settings_backup_restore</span>

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
        @click="doRefresh()"
        class="cell-icon-link material-icons" title="Refresh"alt="Refresh">refresh</span>

    </span>

    <span class="mdc-typography--body2">{{ paramName }}: </span>
    <span>{{ paramDescr() }}</span>

  </div>
  <div v-else class="hdr-row mdc-typography--body2">
    <span class="cell-icon-empty material-icons" aria-hidden="true">refresh</span>
    <span v-if="loadWait || saveWait" class="material-icons om-mcw-spin">star</span>
    <span class="mdc-typography--caption">{{msg}}</span>
  </div>

  <div class="ht-container">
    <HotTable ref="ht" :root="htRoot" :settings="htSettings"></HotTable>
  </div>

  <param-info-dialog ref="noteDlg" id="param-note-dlg"></param-info-dialog>

</div>
  
</template>

<script>
import axios from 'axios'
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import * as Mdf from '@/modelCommon'
import { default as ParamInfoDialog } from './ParameterInfoDialog'
import HotTable from '@/vue-handsontable-official/src/HotTable'

const MAX_PAGE_SIZE = 65536
const MAX_PAGE_OFFSET = (MAX_PAGE_SIZE * MAX_PAGE_SIZE)
const SHOW_MORE_LABEL = 'Show more controls'
const HIDE_MORE_LABEL = 'Hide extra controls'

export default {
  components: { ParamInfoDialog, HotTable },

  props: {
    digest: '',
    paramName: '',
    runOrSet: '',
    nameDigest: ''
  },

  data () {
    return {
      loadDone: false,
      loadWait: false,
      saveDone: false,
      saveWait: false,
      paramText: Mdf.emptyParamText(),
      paramSize: Mdf.emptyParamSize(),
      paramType: Mdf.emptyTypeText(),
      paramRunSet: Mdf.emptyParamRunSet(),
      subCount: 0,
      isWsView: false,    // if true then page view is a workset else model run
      htRoot: '',
      htSettings: {
        manualColumnMove: true,
        manualColumnResize: true,
        manualRowResize: true,
        autoColumnSize: {useHeaders: false},
        columnSorting: true,
        sortIndicator: true,
        preventOverflow: 'horizontal',
        renderAllRows: true,
        stretchH: 'all',
        fillHandle: false,
        readOnly: true,
        afterChange: this.doAfterChange,
        rowHeaders: true,
        rowHeaderWidth: 72,
        columns: [],
        data: []
      },
      dimProp: [],
      dimKeys: [],
      tv: {
        start: 0,
        size: 20,
        isNext: false,
        isPrev: false,
        isInc: false,
        isDec: false
      },
      isShowMoreControls: false,
      moreControlsLabel: SHOW_MORE_LABEL,
      isEdit: false,
      isEditUpdated: false,
      editCount: 0,
      editRowCol: [],
      msg: ''
    }
  },

  computed: {
    routeKey () {
      return [this.digest, this.paramName, this.runOrSet, this.nameDigest].toString()
    },
    ...mapGetters({
      theModel: GET.THE_MODEL,
      theRunText: GET.THE_RUN_TEXT,
      theWorksetText: GET.THE_WORKSET_TEXT,
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  watch: {
    routeKey () { this.refreshView() },
    saveDone () {
      let isDone = this.saveDone
      if (isDone) {
        this.isEditUpdated = false
        this.editCount = 0
        this.editRowCol = []
      }
    }
  },

  methods: {
    paramDescr () { return Mdf.descrOfDescrNote(this.paramText) },

    // show parameter info
    showParamInfo () {
      this.$refs.noteDlg.showParamInfo(this.paramName, this.subCount)
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
      // set table data and row headers
      this.setNewTableData(len, d)
      this.htSettings.rowHeaders = this.makeRowHeaders(len)
    },

    // make parameter data page, columns are: dimensions, sub-values
    // each sub-value is a separate row in source rowset
    setNewTableData (len, d) {
      const vp = []
      const dp = []

      let nowKey = '?'  // non-existent start key to enforce append of the first row
      let n = 0
      const nSub0 = this.paramSize.rank // first sub-value position

      for (let i = 0; i < len; i++) {
        let sk = [d[i].DimIds].toString() // current row key

        if (sk === nowKey) {  // same key: set sub-value at position of sub_id
          vp[n][nSub0 + (d[i].SubId || 0)] = this.translateValue(d[i].Value)  // convert sub-value from enum id to label if required
        } else {              // make new row
          let row = []
          for (let j = 0; j < this.paramSize.rank; j++) {
            row.push(
              this.translateDimEnumId(j, d[i].DimIds[j]) || d[i].DimIds[j]
            )
          }
          for (let j = 0; j < this.subCount; j++) {   // append empty sub-values
            row.push(void 0)
          }

          // append new row to page data, set first row value, store dimension ids
          row[nSub0 + (d[i].SubId || 0)] = this.translateValue(d[i].Value)
          n = vp.length
          vp.push(row)
          nowKey = sk
          if (this.isEdit) dp.push(d[i].DimIds)
        }
      }

      // set page data and stroe dimension keys for page rows
      this.htSettings.data = vp
      this.dimKeys = dp
    },

    // make table row headers, each data row can contain multiple table rows.
    makeRowHeaders (len) {
      if ((!!len || 0) <= 0) return ['none']
      const rh = []
      let n = this.tv.start
      if (n < 0) n = 0
      for (let k = 0; k < len; k++) {
        rh.push(n + k + 1)
      }
      return rh
    },

    // translate dimension enum id to label, it does return enum code if label is empty
    translateDimEnumId (dimIdx, enumId) {
      return Mdf.enumDescrOrCodeById(this.dimProp[dimIdx].typeText, enumId)
    },

    // translate column value if parameter value is enum-based
    translateValue (val) {
      if (!Mdf.isBuiltIn(this.paramType.Type)) return Mdf.enumDescrOrCodeById(this.paramType, val)
      return val
    },

    // save if data editied
    doSave () {
      this.doSaveDataPage()
    },

    // cell(s) edit completed event: load data, copy-paste, undo-redo
    // handsontable issue: even data not changed handsontable emit 'eidt' event
    doAfterChange (changes, source) {
      if (!this.isEdit) return      // edit not allowed

      if (source === 'loadData') {  // new data loaded
        this.editCount = 0
        this.isEditUpdated = false
        this.editRowCol = []
        return
      }

      // undo changes
      if (source === 'UndoRedo.undo') {
        if (this.editCount > 0) this.editCount--
        if (this.editCount <= 0) {
          this.isEditUpdated = false
          this.editRowCol = []
        }
        return
      }
      this.editCount++

      // compare old and new value: it may be no changes
      let len = Mdf.lengthOf(changes)
      if (len <= 0) return      // changes undefined
      try {
        for (let k = 0; k < len; k++) {
          let oldVal = changes[k][2]
          let newVal = changes[k][3]
          let iRow = changes[k][0]
          let iCol = changes[k][1]
          let isUpd = false

          // handsontable issue: compare old and new value, it may be identical
          // handsontable issue: numeric value converted to string
          // check new value type, convert back to number, store numeric value in the data
          if (newVal !== oldVal) {
            if (!Mdf.isInt(this.paramType.Type) && !Mdf.isFloat(this.paramType.Type)) {
              isUpd = true
            } else {
              if (typeof newVal !== 'number') {
                let nv = parseFloat(newVal)
                isUpd = nv !== oldVal                 // cell updated only if numeric values different
                this.htSettings.data[iRow][iCol] = nv // restore numeric value
              }
            }
          }

          // value updated, store updated cell row column
          if (isUpd) {
            this.isEditUpdated = true
            let isFound = false
            for (let j = 0; !isFound && j < this.editRowCol.length; j++) {
              isFound = this.editRowCol[j][0] === iRow && this.editRowCol[j][1] === iCol
            }
            if (!isFound) this.editRowCol.push([iRow, iCol])
          }
        }
      } catch (e) {
        console.log('after change comparison failed')
      }
    },

    // refresh current page view on mounted or tab switch
    refreshView () {
      // find parameter, parameter type and size, including run sub-values count
      this.isWsView = ((this.runOrSet || '') === Mdf.SET_OF_RUNSET)
      this.paramText = Mdf.paramTextByName(this.theModel, this.paramName)
      this.paramSize = Mdf.paramSizeByName(this.theModel, this.paramName)
      this.paramType = Mdf.typeTextById(this.theModel, (this.paramText.Param.TypeId || 0))
      this.paramRunSet = Mdf.paramRunSetByName(
        this.isWsView ? this.theWorksetText : this.theRunText,
        this.paramName)
      this.subCount = this.paramRunSet.SubCount || 0
      this.isEdit = this.isWsView && !this.theWorksetText.IsReadonly

      // column sorting only if parameter non-editable
      this.htSettings.columnSorting = !this.isEdit
      this.htSettings.sortIndicator = !this.isEdit

      // find dimension type for each dimension
      this.dimProp = []
      for (let j = 0; j < this.paramSize.rank; j++) {
        if (this.paramText.ParamDimsTxt[j].hasOwnProperty('Dim')) {
          let t = Mdf.typeTextById(this.theModel, (this.paramText.ParamDimsTxt[j].Dim.TypeId || 0))
          this.dimProp.push({
            name: this.paramText.ParamDimsTxt[j].Dim.Name || '',
            label: Mdf.descrOfDescrNote(this.paramText.ParamDimsTxt[j]),
            typeText: t
          })
        } else {
          this.dimProp.push({
            name: '', label: '', typeText: Mdf.emptyTypeText() })
        }
      }

      // set column: header, type and validator
      this.htSettings.columns = []
      for (let j = 0; j < this.paramSize.rank; j++) {
        this.htSettings.columns.push({
          readOnly: true,
          title: (this.dimProp[j].label || this.dimProp[j].name)})
      }
      // single value column or multiple sub-values
      if (this.subCount <= 1) {
        this.htSettings.columns.push(this.valueColumnDef('Value'))
      } else {
        for (let j = 0; j < this.subCount; j++) {
          this.htSettings.columns.push(this.valueColumnDef(j.toString()))
        }
      }

      // set columns layout and refresh the data
      this.htRoot = 'ht-p-' + this.digest + '-' + this.runOrSet + '-' + this.nameDigest + '-' + this.paramName
      this.setDefaultPageView()
      this.doRefreshDataPage()
    },

    // value column(s) settings, editor and validator
    valueColumnDef (title) {
      // readonly value columns: editing disabled
      if (!this.isEdit) {
        let col = {readOnly: true, title: title}

        if (Mdf.isInt(this.paramType.Type) || Mdf.isFloat(this.paramType.Type)) {
          col.className = 'htRight'
        }
        if (Mdf.isBool(this.paramType.Type)) {
          col.type = 'checkbox'
        }
        return col
      }

      // editing enabled
      let col = {readOnly: false, allowInvalid: false, allowEmpty: false, title: title}

      if (Mdf.isInt(this.paramType.Type)) {
        col.validator = /^[-+]?[0-9]+$/
        col.className = 'htRight'
      }
      if (Mdf.isFloat(this.paramType.Type)) {
        col.validator = /^[-+]?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?$/
        col.className = 'htRight'
      }
      if (Mdf.isBool(this.paramType.Type)) {
        col.type = 'checkbox'
      }
      if (!Mdf.isBuiltIn(this.paramType.Type)) {  // enum type
        col.editor = 'select'
        col.selectOptions = Mdf.enumDescrOrCodeArray(this.paramType)
        // col.type = 'dropdown'
        // col.source = Mdf.enumDescrOrCodeArray(this.paramType)
      }
      return col
    },

    // set default page view parameters
    setDefaultPageView () {
      this.tv.start = 0
      this.tv.size = Math.min(20, this.paramSize.dimTotal)
    },

    // get page of parameter data from current model run or workset
    async doRefreshDataPage () {
      this.loadDone = false
      this.loadWait = true
      this.msg = 'Loading...'
      this.tv.isNext = false
      this.tv.isPrev = false
      this.tv.isInc = false
      this.tv.isDec = false

      // exit if parameter not found in model run or workset
      if (!Mdf.isParamRunSet(this.paramRunSet)) {
        let m = 'Parameter not found in ' + this.nameDigest
        this.msg = m
        console.log(m)
        this.loadWait = false
        return
      }

      // make parameter read layout and url
      let layout = this.makeSelectLayout()
      let u = this.omppServerUrl +
        '/api/model/' + (this.digest || '') +
        (this.isWsView ? '/workset/' : '/run/') + (this.nameDigest || '') +
        '/parameter/value-id'

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
        this.tv.start = Math.floor(lt.Offset / this.subCount)
        this.tv.isNext = !lt.IsLastPage
        this.tv.isPrev = lt.Offset > 0
        this.tv.isInc = this.tv.isNext && this.tv.size < MAX_PAGE_SIZE
        this.tv.isDec = this.tv.size > 1 || (this.tv.size === 0 && lt.Size > 2 * this.subCount)

        // update table page
        this.setData(d)
        this.loadDone = true
      } catch (e) {
        this.msg = 'Server offline or parameter data not found'
        console.log('Server offline or parameter data not found')
      }
      this.loadWait = false
    },

    // return page layout to read parameter data
    makeSelectLayout () {
      //
      // each sub-value is a separate row
      let nStart = this.tv.start * this.subCount
      let nSize = this.tv.size * this.subCount
      let layout = {
        Name: this.paramName,
        Offset: (this.tv.size > 0 ? nStart : 0),
        Size: (this.tv.size > 0 ? nSize : 0),
        IsLastPage: true,
        Filter: [],
        OrderBy: []
      }

      // make deafult order by:
      //   SELECT sub_id, dim0, dim1, value... ORDER BY 2, 3, 1
      for (let k = 0; k < this.paramSize.rank; k++) {
        layout.OrderBy.push({IndexOne: k + 2})
      }
      layout.OrderBy.push({IndexOne: 1})

      return layout
    },

    // save page of parameter data into current workset
    async doSaveDataPage () {
      this.saveDone = false
      this.saveWait = true
      this.msg = 'Saving...'

      // exit if parameter not found in model run or workset
      if (!Mdf.isParamRunSet(this.paramRunSet)) {
        let m = 'Parameter not found in ' + this.nameDigest
        this.msg = m
        console.log(m)
        this.saveWait = false
        return
      }

      // prepare parameter data for save, exit with error if no changes found
      let pv = this.makePageForSave()
      if (!Mdf.lengthOf(pv)) {
        this.msg = 'No parameter changes, nothing to save'
        console.log('No parameter changes, nothing to save')
        this.saveWait = false
        return
      }

      // url to update parameter data
      let u = this.omppServerUrl +
        '/api/model/' + (this.digest || '') +
        '/workset/' + (this.nameDigest || '') +
        '/parameter/' + (this.paramName || '') + '/new/value-id'

      // send data page to the server, response body expected to be empty
      try {
        const response = await axios.post(u, pv)
        const rsp = response.data
        if ((rsp || '') !== '') console.log('Server reply:', rsp)
        this.saveDone = true
      } catch (e) {
        this.msg = 'Server offline or parameter save failed'
        console.log('Server offline or parameter save failed')
      }
      this.saveWait = false
    },

    // prepare page of parameter data for save
    makePageForSave () {
      if (!this.isEdit || this.editRowCol.length <= 0) return []   // no changes or edit disabled

      const nSub0 = this.paramSize.rank // first sub-value position
      const pv = []

      for (let k = 0; k < this.editRowCol.length; k++) {
        let iRow = this.editRowCol[k][0]
        let iCol = this.editRowCol[k][1]

        // if parameter enum-based then convert label to enum id
        let v = (Mdf.isBuiltIn(this.paramType.Type)
          ? this.htSettings.data[iRow][iCol]
          : Mdf.enumIdByDescrOrCode(this.paramType, this.htSettings.data[iRow][iCol]))

        pv.push({
          DimIds: this.dimKeys[iRow],
          Value: v,
          SubId: iCol - nSub0})
      }
      return pv
    }
  },

  mounted () {
    this.saveDone = true
    this.refreshView()
    this.$emit('tab-mounted', 'parameter', this.paramName)
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
