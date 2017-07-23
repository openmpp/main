<template>
  <div id="settings-page" class="mdc-typography">
    <slot></slot>
    
    <div class="mdc-typography--body1 set-table">

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button :raised="true" @click="doUiLangClear()">Clear</om-mcw-button>
        </span>
        <span class="set-table-cell">Language:</span>
        <span class="set-table-cell mdc-typography--body2">{{ uiLangTitle }}</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button :raised="true" @click="doModelListClear()">Clear</om-mcw-button>
        </span>
        <span class="set-table-cell">Models List:</span>
        <span class="set-table-cell mdc-typography--body2">{{ modelListCount() }} model(s)</span>
      </div>

      <div class="set-table-row">
        <span class="set-table-cell">
          <om-mcw-button :raised="true" @click="doModelClear()">Clear</om-mcw-button>
        </span>
        <span class="set-table-cell">Current Model:</span>
        <span class="set-table-cell mdc-typography--body2">{{ modelTitle }}</span>
      </div>

    </div>
  </div>
</template>

<script>
import { mapGetters, mapMutations } from 'vuex'
import { GET, SET } from '@/store'
import OmMcwButton from './OmMcwButton'
import { default as Mdf } from '@/modelCommon'

export default {
  props: {
  },

  data () {
    return {
    }
  },

  computed: {
    modelTitle () {
      if (!Mdf.isModel(this.theModel)) return 'Undefined model'
      if (Mdf.isModelEmpty(this.theModel)) return 'Not selected'
      return Mdf.modelTitle(this.theModel)
    },

    uiLangTitle () { return this.uiLang !== '' ? this.uiLang : 'Default' },

    ...mapGetters({
      uiLang: GET.UI_LANG,
      theModel: GET.THE_MODEL,
      modelList: GET.MODEL_LIST,
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  methods: {
    doUiLangClear () { this.setUiLang('') },
    doModelClear () { this.setEmptyModel() },
    doModelListClear () { this.setEmptyModelList() },
    modelListCount () { return Mdf.isModelList(this.modelList) ? this.modelList.length : 0 },

    ...mapMutations({
      setUiLang: SET.UI_LANG,
      setTheModel: SET.THE_MODEL,
      setEmptyModel: SET.EMPTY_MODEL,
      setEmptyModelList: SET.EMPTY_MODEL_LIST
    })
  },

  components: { OmMcwButton }
}

</script>

<!-- this component only css -->
<style scoped lang="scss">
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
  @import "@material/typography/mdc-typography";
  @import "@material/theme/mdc-theme";
</style>
