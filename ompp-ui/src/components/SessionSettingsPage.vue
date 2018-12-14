<template>
  <div id="settings-page" class="mdc-typography mdc-typography--body1">

    <div class="set-table">

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button @click="doUiLangClear()" unelevated class="clear-button" alt="Clear language" title="Clear language">
            <i class="material-icons mdc-button__icon">clear</i>
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Language:</span>
        <span class="set-table-cell medium-wt">{{ uiLangTitle }}</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button @click="doModelListClear()" unelevated class="clear-button" alt="Clear model list" title="Clear model list">
            <i class="material-icons mdc-button__icon">clear</i>
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Models list:</span>
        <span class="set-table-cell medium-wt">{{ modelCount }} model(s)</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button @click="doModelClear()" unelevated class="clear-button" alt="Clear model" title="Clear model">
            <i class="material-icons mdc-button__icon">clear</i>
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Current model:</span>
        <span class="set-table-cell medium-wt">{{ modelTitle }}</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button @click="doRunClear()" unelevated class="clear-button" alt="Clear model run list" title="Clear model run list">
            <i class="material-icons mdc-button__icon">clear</i>
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Model run results:</span>
        <span class="set-table-cell medium-wt">{{ runCount }}</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button @click="doWsClear()" unelevated class="clear-button" alt="Clear model workset list" title="Clear model workset list">
            <i class="material-icons mdc-button__icon">clear</i>
          </om-mcw-button>
        </span>
        <span class="set-table-cell">Sets of input parameters:</span>
        <span class="set-table-cell medium-wt">{{ worksetCount }}</span>
      </div>

      <div v-if="isNotEmptyRun" class="set-table-row">
        <span class="set-table-cell"></span>
        <span class="set-table-cell">Current model run:</span>
        <span class="set-table-cell">
          <span class="mono">{{lastTimeOfRun}}&nbsp;</span><span class="medium-wt">{{nameOfRun}}</span>
          <span>{{ descrOfRun }}</span>
        </span>
      </div>

      <div v-if="isNotEmptyWorkset" class="set-table-row">
        <span class="set-table-cell"></span>
        <span class="set-table-cell">Current input set:</span>
        <span class="set-table-cell">
          <span class="mono">{{lastTimeOfWorkset}}&nbsp;</span><span class="medium-wt">{{nameOfWorkset}}</span>
          <span>{{ descrOfWorkset }}</span>
        </span>
      </div>

    </div>
  </div>
</template>

<script>
import { mapGetters, mapActions } from 'vuex'
import { GET, SET } from '@/store'
import * as Mdf from '@/modelCommon'
import OmMcwButton from '@/om-mcw/OmMcwButton'

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
    isNotEmptyRun () { return Mdf.isNotEmptyRunText(this.theRunText) },
    lastTimeOfRun () { return Mdf.dtStr(this.theRunText.UpdateDateTime) },
    nameOfRun () { return this.theRunText.Name || '' },
    descrOfRun () { return Mdf.descrOfTxt(this.theRunText) },
    isNotEmptyWorkset () { return Mdf.isNotEmptyWorksetText(this.theWorksetText) },
    lastTimeOfWorkset () { return Mdf.dtStr(this.theWorksetText.UpdateDateTime) },
    nameOfWorkset () { return this.theWorksetText.Name || '' },
    descrOfWorkset () { return Mdf.descrOfTxt(this.theWorksetText) },

    ...mapGetters({
      uiLang: GET.UI_LANG,
      theModel: GET.THE_MODEL,
      modelListCount: GET.MODEL_LIST_COUNT,
      runTextList: GET.RUN_TEXT_LIST,
      worksetTextList: GET.WORKSET_TEXT_LIST,
      theRunText: GET.THE_RUN_TEXT,
      theWorksetText: GET.THE_WORKSET_TEXT,
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

  .clear-button {
    border: 0;
    padding: 0 0 0 .5rem;
    min-width: 1rem;
  }
  :not(:disabled).clear-button {
    background-color: var(--mdc-theme-primary);
  }

  .set-table {
    display: table;
    padding-top: 0.5rem;
  }

  .set-table-row {
    display: table-row;
  }

  .set-table-cell {
    display: table-cell;
    padding-right: 0.5rem;
    padding-top: 0.5rem;
  }
</style>
