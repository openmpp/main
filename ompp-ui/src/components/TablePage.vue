<template>

<div id="table-page" class="main-container mdc-typography  mdc-typography--body1">

  <div class="hdr-row mdc-typography--body1">
    
    <span
      @click="showTableInfo()" 
      class="cell-icon-link material-icons" 
      :alt="tableName + ' info'"
      :title="tableName + ' info'">event_note</span>
    <span class="mdc-typography--body2">{{ tableName }}: </span>
    <span>{{ tableDescr() }}</span>

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
    <div class="mono">Name:&nbsp;&nbsp;&nbsp;{{tableText.Table.Name}}</div>
    <div class="mono">Rank:&nbsp;&nbsp;&nbsp;{{tableText.Table.Rank || 0}}</div>
    <div class="mono">Digest:&nbsp;{{tableText.Table.Digest}}</div>
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
      titleNoteDlg: '',
      textNoteDlg: '',
      tableText: Mdf.emptyTableText(),
      htRoot: '',
      htSettings: {
        manualColumnMove: true,
        manualColumnResize: true,
        manualRowResize: true,
        rowHeaders: true,
        fillHandle: false,
        stretchH: 'last',
        data: [],
        colHeaders: [],
        columns: []
      },
      dataPage: []
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
  },

  methods: {
    tableDescr () { return Mdf.descrOfDescrNote(this.tableText) },

    // show table info
    showTableInfo () {
      this.titleNoteDlg = this.tableText.TableDescr || ''
      this.textNoteDlg = this.tableText.TableNote || ''
      this.$refs.noteDlg.open()
    },

    //
    async doDataPage () {
      //
      let r = this.theRunText
      if (!Mdf.isNotEmptyRunText(r)) {
        console.log('No run completed')
        return
      }
      //
      let u = this.omppServerUrl + '/api/model/' + (this.digest || '') + '/run/' + (r.Digest || '') + '/table/' + this.tableName + '/expr'
      try {
        const response = await axios.get(u)
        this.dataPage = response.data
      } catch (e) {
        console.log('Server offline or no run completed')
      }
      //
      const result = []
      for (let i = 0; i < this.dataPage.length; i++) {
        let row = []
        for (let j = 0; j < this.dataPage[i].Dims.length; j++) {
          row.push(this.dataPage[i].Dims[j])
        }
        row.push(this.dataPage[i].ExprId, this.dataPage[i].IsNull, this.dataPage[i].Value)
        result.push(row)
      }
      this.htSettings.data = result
    }
  },

  mounted () {
    this.$emit('tab-mounted', 'table', this.tableName)
    this.tableText = Mdf.tableTextByName(this.theModel, this.tableName)

    // let n = Mdf.tableSizeByName(this.theModel, this.tableName)
    // console.log('tableSizeByName', n)

    this.htSettings.columns = []
    this.htSettings.colHeaders = []
    for (let j = 0; j < this.tableText.TableDimsTxt.length; j++) {
      this.htSettings.columns.push({readOnly: true})
      this.htSettings.colHeaders.push(this.tableText.TableDimsTxt[j].Dim.Name)
    }
    this.htSettings.columns.push({readOnly: true}, {readOnly: true}, {readOnly: true})
    this.htSettings.colHeaders.push('ExprId', 'IsNull', 'Value')

    // this.htRoot = 'ht-' + this.digest + '-' + this.tableName + '-' + (this.theRunText.Digest || '')
    this.doDataPage()
  },

  components: { OmMcwDialog, HotTable }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
  @import "handsontable/dist/handsontable.full.css";

  /* main container and header row */
  .main-container {
    height: 100%;
    flex: 1 1 auto;
    display: flex; 
    flex-direction: column;
    overflow-y: hidden;
  }
  .hdr-row {
    margin-top: 0.5em;
    margin-bottom: 0.5em;
    flex: 0 0 auto;
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
  }

  /* table container */
  .ht-container {
    flex: 1 1 auto;
    position: relative;
    overflow: auto;
  }

  /* to fix handsontable z-index: 101; */
  #note-dlg {
    z-index: 200;
  }

  /* cell material icon: a link or empty (non-link) */
  .cell-icon {
    vertical-align: middle;
    margin: 0;
    padding-left: 0.5em;
    padding-right: 0.5em;
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
</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
