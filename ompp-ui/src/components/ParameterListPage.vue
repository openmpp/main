<template>

<div id="parameter-list-page" class="mdc-typography mdc-typography--body1">

  <div v-if="isParamList()">
    <ul class="main-list mdc-list mdc-list--two-line">

      <li v-for="p in paramList()" :key="'pt-' + p.Param.ParamId" class="mdc-list-item">

        <span
          @click="showParamInfo(p)"
          class="material-icons mdc-list-item__graphic note-link"
          :title="p.Param.Name + ' info'"
          :alt="p.Param.Name + ' info'">event_note</span>
        <router-link v-if="pathRunSet !== '/'"
          :to="'/model/' + digest + '/' + pathRunSet + '/parameter/' + p.Param.Name"
          class="ahref-next"
          :title="p.Param.Name"
          :alt="p.Param.Name"
          >
          <span class="mdc-list-item__text">
            <span class="mdc-list-item__primary-text">{{ p.Param.Name }}</span>
            <span class="mdc-list-item__secondary-text">{{ descrOf(p) }}</span>
          </span>
        </router-link>
        <span v-else class="ahref-next">
          <span class="mdc-list-item__text">
            <span class="mdc-list-item__primary-text">{{ p.Param.Name }}</span>
            <span class="mdc-list-item__secondary-text">{{ descrOf(p) }}</span>
          </span>
        </span>

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
    nameDigest: ''
  },

  data () {
    return {
    }
  },

  computed: {
    pathRunSet () {
      let rs = this.runOrSet || ''
      let nd = this.nameDigest || ''
      if (nd === '') {
        if (rs === Mdf.SET_OF_RUNSET) {
          nd = this.theWorksetText.Name
        } else {
          nd = this.theRunText.Digest
          if (nd === '') nd = this.theRunText.Name
          if (rs === '' && nd !== '') rs = Mdf.RUN_OF_RUNSET
        }
      }
      return rs + '/' + nd
    },
    ...mapGetters({
      theModel: GET.THE_MODEL,
      theRunText: GET.THE_RUN_TEXT,
      theWorksetText: GET.THE_WORKSET_TEXT
    })
  },

  watch: {
  },

  methods: {
    // array of ParamTxt if model has parameter list
    isParamList () { return Mdf.isParamTextList(this.theModel) },
    paramList () { return Mdf.isParamTextList(this.theModel) ? this.theModel.ParamTxt : [] },

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

  /* link to next page */
  .item-link-next {
    display: block;
    width: 100%;
    height: 100%;
    text-decoration: none;
    @extend .mdc-theme--text-primary-on-background;
  }
  .link-next {
    @extend .item-link-next;
    &:hover {
      cursor: pointer;
      background: rgba(0, 0, 0, 0.1);
    }
  }
  .ahref-next {
    @extend .item-link-next;
    &:hover {
      background: rgba(0, 0, 0, 0.1);
    }
  }

  /* notes: a link or empty (not a link) */
  .note-item {
    display: inline-block;
    height: 100%;
    margin: 0;
    padding-top: 2rem;
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
