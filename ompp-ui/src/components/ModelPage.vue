<template>

<div id="model-page" class="mdc-typography">

  <div v-if="loadDone" class="mdc-typography--body1">

    <a href="#" 
      v-if="isModelNote()" 
      @click="showModelNote()" 
      class="model-note material-icons" 
      title="Notes" 
      aria-hidden="true">info_outline</a>

      <span class="mdc-typography--body1">{{ modelTitle() }}</span>

  </div>

  <div v-else>
    <span class="material-icons om-mcw-spin">refresh</span><span>{{msg}}</span>
  </div>

  <om-mcw-dialog ref="noteDlg" id="note-dlg" acceptText="OK">
    <span slot="header">{{titleNoteDlg}}</span>
    <div>{{textNoteDlg}}</div>
  </om-mcw-dialog>

</div>
  
</template>

<script>
import axios from 'axios'
import { mapGetters, mapMutations } from 'vuex'
import { GET, SET } from '@/store'
import OmMcwButton from './OmMcwButton'
import OmMcwDialog from './OmMcwDialog'
import { default as Mdf } from '@/modelCommon'

export default {
  props: {
    digest: '',
    refreshTickle: false
  },

  data () {
    return {
      loadDone: false,
      titleNoteDlg: '',
      textNoteDlg: '',
      msg: ''
    }
  },

  computed: {
    ...mapGetters({
      uiLang: GET.UI_LANG,
      theModel: GET.THE_MODEL,
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  watch: {
    // refresh button handler
    refreshTickle () {
      this.doRefresh()
    }
  },

  methods: {
    modelTitle () { return Mdf.modelTitle(this.theModel) },

    // if model notes not empty
    isModelNote () {
      return Mdf.isModelNote(this.theModel)
    },
    // then show model notes
    showModelNote () {
      this.titleNoteDlg = Mdf.modelTitle(this.theModel)
      this.textNoteDlg = Mdf.modelNote(this.theModel)
      this.$refs.noteDlg.open()
    },

    // refersh current model
    async doRefresh () {
      let u = this.omppServerUrl + '/api/model-text/' + (this.digest || '') + (this.uiLang !== '' ? '/' + this.uiLang : '')
      this.loadDone = false
      this.msg = 'Loading...'
      try {
        const response = await axios.get(u)
        this.setTheModel(response.data)   // update current model in store
      } catch (e) {
        this.msg = 'Server offline or model not found'
      }
      this.loadDone = true
    },

    ...mapMutations({
      setTheModel: SET.THE_MODEL
    })
  },

  mounted () {
    // if model already loaded then exit
    if (Mdf.modelDigest(this.theModel) === this.digest) {
      this.loadDone = true
      return
    }
    this.doRefresh() // load new model
  },

  components: { OmMcwButton, OmMcwDialog }
}

</script>

<!-- this component only css -->
<style scoped lang="scss">

.model-note {
  display: inline-block;
  vertical-align: top;
  height: 100%;
  margin: 0;
  text-decoration: none;
  padding-left: 0.5em;
  padding-right: 0.5em;
}

a.model-note {
  outline: none;
}

a.model-note:hover {
  background: rgba(0, 0, 0, 0.1);
}

.model-note {
  color: rgba(0, 0, 0, 0.54);
  color: var(--mdc-theme-text-secondary-on-background, rgba(0, 0, 0, 0.54));
}

</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/list/mdc-list";
  @import "@material/typography/mdc-typography";
  @import "@material/theme/mdc-theme";
</style>
