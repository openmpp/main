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
          :to="'/model/' + digest + '/run/' + (nameDigest || '') + '/table/' + t.Table.Name" 
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

  <table-info-dialog ref="noteDlg" id="table-note-dlg"></table-info-dialog>

</div>
  
</template>

<script>
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import * as Mdf from '@/modelCommon'
import TableInfoDialog from './TableInfoDialog'

export default {
  components: { TableInfoDialog },

  props: {
    digest: '',
    nameDigest: '',
    refreshTickle: false
  },

  data () {
    return {
    }
  },

  computed: {
    isTableList () { return Mdf.isTableTextList(this.theModel) },

    ...mapGetters({
      theModel: GET.THE_MODEL
    })
  },

  watch: {
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
      this.$refs.noteDlg.showTableInfo(t.Table.Name, 0)
    }
  },

  mounted () {
    this.$emit('tab-mounted', 'table-list', '')
  }
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
    padding-left: 0.5rem;
    padding-right: 0.5rem;
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
