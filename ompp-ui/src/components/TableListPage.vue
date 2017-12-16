<template>

<div id="table-list-page" class="mdc-typography mdc-typography--body1">

  <div v-if="isTableList">
    <ul class="main-list mdc-list mdc-list--two-line">

      <li v-for="t in TableList()" :key="'tt-' + t.Table.TableId" class="mdc-list-item">

        <span
          @click="showTableInfo(t)" 
          class="material-icons mdc-list-item__start-detail note-link" 
          :title="t.Table.Name + ' notes'"
          :alt="t.Table.Name + ' notes'">event_note</span>
        <router-link
          :to="'/model/' + digest + '/table/' + t.Table.Name" 
          class="ahref-next" 
          :title="t.Table.Name"
          :alt="t.Table.Name" 
          >
          <span class="mdc-list-item__text">{{ t.Table.Name }}
            <span class="mdc-list-item__secondary-text">{{ t.TableDescr }}</span>
          </span>
        </router-link>

      </li>

    </ul>
  </div>

  <om-mcw-dialog ref="noteDlg" id="note-dlg" acceptText="OK">
    <span slot="header">{{titleNoteDlg}}</span>
    <div>{{textNoteDlg}}</div>
    <div v-if="((exprDescrDlg || '') !== '' || (exprNoteDlg || '') !== '')">
      <br v-if="((textNoteDlg || '') !== '')"/>
      <div>{{exprDescrDlg}}</div>
      <div>{{exprNoteDlg}}</div>
    </div>
    <br/>
    <div class="mono">Name:&nbsp;&nbsp;&nbsp;{{nameNoteDlg}}</div>
    <div class="mono">Rank:&nbsp;&nbsp;&nbsp;{{rankNoteDlg}}</div>
    <div class="mono">Digest:&nbsp;{{digestNoteDlg}}</div>
  </om-mcw-dialog>

</div>
  
</template>

<script>
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import { default as Mdf } from '@/modelCommon'
import OmMcwDialog from './OmMcwDialog'

export default {
  props: {
    digest: '',
    refreshTickle: false
  },

  data () {
    return {
      titleNoteDlg: '',
      textNoteDlg: '',
      nameNoteDlg: '',
      exprDescrDlg: '',
      exprNoteDlg: '',
      rankNoteDlg: 0,
      digestNoteDlg: ''
    }
  },

  computed: {
    isTableList () { return Mdf.isTableTextList(this.theModel) },

    ...mapGetters({
      theModel: GET.THE_MODEL
    })
  },

  watch: {
    // refresh button handler
  },

  methods: {
    // array of TableTxt if model has table list
    TableList () {
      return Mdf.isTableTextList(this.theModel) ? this.theModel.TableTxt : []
    },

    // return description from DescrNote
    descrOf (t) {
      return Mdf.descrOfDescrNote(t)
    },

    // show table info
    showTableInfo (t) {
      this.titleNoteDlg = t.TableDescr || ''
      this.textNoteDlg = t.TableNote || ''
      this.exprDescrDlg = t.ExprDescr || ''
      this.exprNoteDlg = t.ExprNote || ''
      this.nameNoteDlg = t.Table.Name
      this.digestNoteDlg = t.Table.Digest
      this.rankNoteDlg = t.Table.Rank || 0
      this.$refs.noteDlg.open()
    }
  },

  mounted () {
    this.$emit('tab-mounted', 'table-list', '')
  },

  components: { OmMcwDialog }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";

  /* main body list */
  .main-list {
    padding-left: 0;
  }

  /* a link to next page */
  .ahref-next {
    display: block;
    width: 100%;
    height: 100%;
    text-decoration: none;
    @extend .mdc-theme--text-primary-on-background;
    &:hover {
      background: rgba(0, 0, 0, 0.1);
    }
  }

  /* notes: a link or empty (not a link) */
  .note-item {
    display: inline-block;
    vertical-align: top;
    height: 100%;
    margin: 0;
    padding-left: 0.5em;
    padding-right: 0.5em;
  }
  .note-link {
    @extend .note-item;
    &:hover {
      cursor: pointer;
      background: rgba(0, 0, 0, 0.1);
    }
    @extend .mdc-theme--text-secondary-on-background;
  }
  .note-empty {
    @extend .note-item;
    cursor: default;
    @extend .mdc-theme--text-disabled-on-background;
  }
</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
