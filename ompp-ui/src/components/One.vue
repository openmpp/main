<!-- Test page, not for production -->
<template>
  <div id="one" class="mdc-typography">
    <slot></slot>
    <p class="mdc-typography--body1">For internal use only.</p>
    <p class="mdc-typography--body1">modelTitle ={{ modelTitle }}=</p>
    <p class="mdc-typography--body1">uiLang ={{ uiLang }}=</p>
    <p class="mdc-typography--body1">msg ={{ msg }}=</p>

    <om-mcw-button :raised="true" @click="doClick('ok')">ok</om-mcw-button>
    <om-mcw-button :raised="true" @click="doShow()">show</om-mcw-button>

    <om-mcw-dialog 
      ref="dlg" 
      id="one-dlg" 
      cancelText="No"
      acceptText="Yes"
      @accept="doAccept" 
      @cancel="doCancel">
      <span slot="header">Title</span>
      <div>Dialog text</div>
    </om-mcw-dialog>

  </div>
</template>

<script>
import { mapGetters, mapMutations } from 'vuex'
import { GET, SET } from '@/store'
import OmMcwButton from './OmMcwButton'
import OmMcwDialog from './OmMcwDialog'
import { default as Mdf } from '@/modelCommon'

export default {
  props: {
  },

  data () {
    return {
      opened: true,
      msg: 'not initialized'
    }
  },

  computed: {
    modelTitle () { return Mdf.modelTitle(this.theModel) },

    ...mapGetters({
      uiLang: GET.UI_LANG,
      theModel: GET.THE_MODEL,
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  methods: {
    doClick (m) { this.msg = m + ':' + process.env.NODE_ENV + ':' + this.omppServerUrl + ':' },

    doShow () {
      this.msg = 'do show'
      this.$refs.dlg.open()
    },

    doAccept () { this.msg = 'do accept' },
    doCancel () { this.msg = 'do cancel' },

    ...mapMutations({
      setUiLang: SET.UI_LANG,
      setTheModel: SET.THE_MODEL
    })
  },

  components: { OmMcwButton, OmMcwDialog }
}

</script>

<!-- this component only css -->
<style scoped>

.button-m-icon {
  display: inline;
  vertical-align: middle;
}

</style>
