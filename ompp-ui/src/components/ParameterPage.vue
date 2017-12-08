<template>

<div id="parameter-page" class="mdc-typography  mdc-typography--body1">

  <div class="hdr-row mdc-typography--body1">
    
    <span>
      <a href="#"
        @click="showParamInfo()" 
        class="cell-icon-link material-icons" 
        :alt="paramName + ' notes'"
        :title="paramName + ' notes'">event_note</a>
    </span>
    <span class="mdc-typography--body2">{{ paramName }}: </span>
    <span>{{ paramDescr() }}</span>

  </div>

  <div v-for="(r, i) in dataPage" :key="i" :id="'r-' + i.toString()">
    {{r}}
  </div>

  <om-mcw-dialog ref="noteDlg" id="note-dlg" acceptText="OK">
    <span slot="header">{{titleNoteDlg}}</span>
    <div>{{textNoteDlg}}</div>
    <br/>
    <div class="mono">Name:&nbsp;&nbsp;&nbsp;{{nameNoteDlg}}</div>
    <div class="mono">Rank:&nbsp;&nbsp;&nbsp;{{rankNoteDlg}}</div>
    <div class="mono">Digest:&nbsp;{{digestNoteDlg}}</div>
  </om-mcw-dialog>

</div>
  
</template>

<script>
import axios from 'axios'
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import { default as Mdf } from '@/modelCommon'
import OmMcwDialog from './OmMcwDialog'

export default {
  props: {
    digest: '',
    paramName: '',
    refreshTickle: false
  },

  data () {
    return {
      titleNoteDlg: '',
      textNoteDlg: '',
      nameNoteDlg: '',
      rankNoteDlg: 0,
      digestNoteDlg: '',
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
    paramText () {
      return Mdf.paramTextByName(this.theModel, this.paramName)
    },
    paramDescr () { return Mdf.descrOfDescrNote(this.paramText()) },

    // show parameter info
    showParamInfo () {
      let pt = this.paramText()
      this.titleNoteDlg = Mdf.descrOfDescrNote(pt)
      this.textNoteDlg = Mdf.noteOfDescrNote(pt)
      this.nameNoteDlg = this.paramName
      this.digestNoteDlg = pt.Param.Digest || ''
      this.rankNoteDlg = pt.Param.Rank || 0
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
      let u = this.omppServerUrl + '/api/model/' + (this.digest || '') + '/run/' + (r.Digest || '') + '/parameter/' + this.paramName + '/value'
      try {
        const response = await axios.get(u)
        this.dataPage = response.data
      } catch (e) {
        console.log('Server offline or no run completed')
      }
    }
  },

  mounted () {
    this.$emit('tab-mounted', 'parameter', this.paramName)
    this.doDataPage()
  },

  components: { OmMcwDialog }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";

  /* header row of model, run, workset properties */
  .hdr-row {
    margin-top: 0.5em;
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
  }

  /* cell material icon: a link or empty (not a link) */
  .cell-icon {
    display: inline-block;
    vertical-align: middle;
    height: 100%;
    margin: 0;
    padding-left: 0.5em;
    padding-right: 0.5em;
  }
  .cell-icon-link {
    @extend .cell-icon;
    text-decoration: none;
    outline: none;
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
