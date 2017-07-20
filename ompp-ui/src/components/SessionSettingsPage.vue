<template>
  <div id="settings-page" class="mdc-typography">
    <slot></slot>
    
    <div class="mdc-typography--body1 set-table">

      <div class="set-table-row">
          <span class="set-table-cell">
            <om-mcw-button :raised="true" @click="doModelClear()">Clear</om-mcw-button>
          </span>
          <span class="set-table-cell">Model:  </span>
          <span class="set-table-cell mdc-typography--body2">{{ modelTitle }}</span>
      </div>

      <div class="set-table-row">
          <span class="set-table-cell">
            <om-mcw-button :raised="true" @click="doUiLangClear()">Clear</om-mcw-button>
          </span>
          <span class="set-table-cell">Language:</span>
          <span class="set-table-cell mdc-typography--body2">{{ uiLangTitle }}  </span>
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
      return !Mdf.isModel(this.theModel)
        ? 'Undefined model'
        : !Mdf.isEmptyModel(this.theModel) ? Mdf.modelTitle(this.theModel) : 'No model selected'
    },

    uiLangTitle () { return this.uiLang !== '' ? this.uiLang : 'Default' },

    ...mapGetters({
      uiLang: GET.UI_LANG,
      theModel: GET.THE_MODEL,
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  methods: {
    doModelClear () { this.setEmptyModel() },
    doUiLangClear () { this.setUiLang('') },

    ...mapMutations({
      setUiLang: SET.UI_LANG,
      setTheModel: SET.THE_MODEL,
      setEmptyModel: SET.EMPTY_MODEL
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
