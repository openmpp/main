<template>

<div id="parameter-list-page" class="mdc-typography mdc-typography--body1">

  <div v-if="isParamList">
    <ul class="main-list mdc-list mdc-list--two-line">

      <li v-for="p in paramList()" :key="'pt-' + p.Param.ParamId" class="mdc-list-item">

        <span
          @click="showParamInfo(p)" 
          class="material-icons mdc-list-item__start-detail note-link" 
          :title="p.Param.Name + ' info'"
          :alt="p.Param.Name + ' info'">event_note</span>
        <router-link
          :to="'/model/' + digest + '/' + (runOrSet || '') + '/' + (nameDigest || '') + '/parameter/' + p.Param.Name" 
          class="ahref-next" 
          :title="p.Param.Name"
          :alt="p.Param.Name" 
          >
          <span class="mdc-list-item__text">{{ p.Param.Name }}
            <span class="mdc-list-item__secondary-text">{{ descrOf(p) }}</span>
          </span>
        </router-link>

      </li>

    </ul>
  </div>

  <param-info-dialog ref="noteDlg" id="param-note-dlg"></param-info-dialog>

</div>
  
</template>

<script>
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import * as Mdf from '@/modelCommon'
import { default as ParamInfoDialog } from './ParameterInfoDialog'

export default {
  components: { ParamInfoDialog },

  props: {
    digest: '',
    runOrSet: '',
    nameDigest: '',
    refreshTickle: false
  },

  data () {
    return {
    }
  },

  computed: {
    isParamList () { return Mdf.isParamTextList(this.theModel) },

    ...mapGetters({
      theModel: GET.THE_MODEL
    })
  },

  watch: {
  },

  methods: {
    // array of ParamTxt if model has parameter list
    paramList () {
      return Mdf.isParamTextList(this.theModel) ? this.theModel.ParamTxt : []
    },

    // return description from DescrNote
    descrOf (p) {
      return Mdf.descrOfDescrNote(p)
    },

    // show parameter info
    showParamInfo (p) {
      this.$refs.noteDlg.showParamInfo(p.Param.Name, 0)
    }
  },

  mounted () {
    this.$emit('tab-mounted', 'parameter-list', '')
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
