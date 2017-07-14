<template>
  <div id="one" class="mdc-typography--display">
    <slot></slot>
    <p class="mdc-typography--body1">One modelTitle ={{ modelTitle }}=</p>
    <p class="mdc-typography--body1">One storeTxt ={{ storeTxt }}=</p>
    <p class="mdc-typography--body1">One uiLang ={{ uiLang }}=</p>
    <p class="mdc-typography--body1">One msg ={{ msg }}=</p>

    <p class="mdc-typography--body1">Fa =<i class="fa fa-subscript fa-2x" role="presentation"aria-hidden="true"></i>=</p>

    <om-mcw-button :raised="true" @click="doClick('ok')">ok</om-mcw-button>
    <om-mcw-button :raised="true" @click="doShow()">show</om-mcw-button>
    <om-mcw-button :raised="true" @click="doSetModel('IDMM', 'f5024ac32c4e8abfc696a0f925141c95')">Set Model</om-mcw-button>

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

export default {
  props: {
    digest: ''
  },

  data () {
    return {
      opened: true,
      msg: 'not initialized'
    }
  },

  computed: {
    modelTitle () {
      return this.theModel.Name + ': ' + this.theModel.Digest + '=' + this.digest
    },
    ...mapGetters({
      uiLang: GET.UI_LANG,
      theModel: GET.THE_MODEL,
      storeTxt: GET.TXT
    })
  },

  methods: {
    doClick (m) { this.msg = m },

    doShow () {
      this.msg = 'do show'
      this.$refs.dlg.open()
    },

    doAccept () { this.msg = 'do accept' },
    doCancel () { this.msg = 'do cancel' },

    doSetModel (name, digest) {
      this.setTheModel({Name: name, Digest: digest})
    },

    ...mapMutations({
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
