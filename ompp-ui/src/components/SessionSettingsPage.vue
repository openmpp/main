<template>
  <div id="settings-page" class="mdc-typography mdc-typography--body1">
    
    <div class="set-table">

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button :raised="true" @click="doUiLangClear()" alt="Clear language">
            <i class="material-icons mdc-button__icon">clear</i>Clear
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Language:</span>
        <span class="set-table-cell mdc-typography--body2">{{ uiLangTitle }}</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button :raised="true" @click="doModelListClear()" alt="Clear model list">
            <i class="material-icons mdc-button__icon">clear</i>Clear
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Models list:</span>
        <span class="set-table-cell mdc-typography--body2">{{ modelCount }} model(s)</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button :raised="true" @click="doModelClear()" alt="Clear model">
            <i class="material-icons mdc-button__icon">clear</i>Clear
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Current model:</span>
        <span class="set-table-cell mdc-typography--body2">{{ modelTitle }}</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button :raised="true" @click="doRunClear()" alt="Clear model run list">
            <i class="material-icons mdc-button__icon">clear</i>Clear
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Model run results:</span>
        <span class="set-table-cell mdc-typography--body2">{{ runCount }}</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button :raised="true" @click="doWsClear()" alt="Clear model workset list">
            <i class="material-icons mdc-button__icon">clear</i>Clear
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Sets of input parameters:</span>
        <span class="set-table-cell mdc-typography--body2">{{ worksetCount }}</span>
      </div>

    </div>
  </div>
</template>

<script>
import { mapGetters, mapActions } from 'vuex'
import { GET, SET } from '@/store'
import { default as Mdf } from '@/modelCommon'
import OmMcwButton from './OmMcwButton'

export default {
  props: {
  },

  data () {
    return {
    }
  },

  computed: {
    uiLangTitle () { return this.uiLang !== '' ? this.uiLang : 'Default' },
    modelTitle () {
      if (!Mdf.isModel(this.theModel)) return 'Not selected'
      return Mdf.modelTitle(this.theModel)
    },
    modelCount () { return this.modelListCount },
    runCount () { return Mdf.runTextCount(this.runTextList) },
    worksetCount () { return Mdf.worksetTextCount(this.worksetTextList) },

    ...mapGetters({
      uiLang: GET.UI_LANG,
      theModel: GET.THE_MODEL,
      modelListCount: GET.MODEL_LIST_COUNT,
      runTextList: GET.RUN_TEXT_LIST,
      worksetTextList: GET.WORKSET_TEXT_LIST,
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  methods: {
    doUiLangClear () { this.setUiLang('') },
    doModelClear () { this.setEmptyModel() },
    doModelListClear () { this.setEmptyModelList() },
    doRunClear () { this.setEmptyRunTextList() },
    doWsClear () { this.setEmptyWorksetTextList() },

    ...mapActions({
      setUiLang: SET.UI_LANG,
      setTheModel: SET.THE_MODEL,
      setEmptyModel: SET.EMPTY_MODEL,
      setEmptyModelList: SET.EMPTY_MODEL_LIST,
      setEmptyRunTextList: SET.EMPTY_RUN_TEXT_LIST,
      setEmptyWorksetTextList: SET.EMPTY_WORKSET_TEXT_LIST
    })
  },

  components: { OmMcwButton }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>

  .set-table {
    display: table;
    padding-top: 0.5em;
  }

  .set-table-row {
    display: table-row;
  }

  .set-table-cell {
    display: table-cell;
    padding-right: 0.5em;
    padding-top: 0.5em;
  }
</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
