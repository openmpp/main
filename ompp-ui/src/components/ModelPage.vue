<template>

<div id="model-page" class="mdc-typography">

  <div v-if="loadDone" class="mdc-typography--body1">

    <a href="#" 
      v-if="isModelNote()" 
      @click="showModelNote()" 
      class="material-icons" 
      title="Notes" 
      aria-hidden="true">info_outline</a>

      <span>{{ modelTitle() }}</span>

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
    digest: ''
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

.ahref-model {
  display: block;
  width: 100%;
  height: 100%;
  text-decoration: none;
  color: var(--mdc-theme-text-primary-on-background, rgba(0, 0, 0, 0.87));
}

a.ahref-model:hover {
  background: rgba(0, 0, 0, 0.1);
}

.mdc-list-item__start-detail, .mdc-list-item__end-detail {
  display: inline;
  vertical-align: middle;
  margin: 0;
  text-decoration: none;
}

.mdc-list-item__start-detail {
  padding-right: 0.5em;
}

.mdc-list-item__end-detail {
  padding: 1em;
}

a.mdc-list-item__end-detail:hover {
  background: rgba(0, 0, 0, 0.1);
}

.mdc-list-item__start-detail, .mdc-list-item__end-detail {
  color: var(--mdc-theme-text-secondary-on-background, rgba(0, 0, 0, 0.54))
}

/* fix for ms edge 12+ */
@supports (-ms-ime-align:auto) {
  .mdc-list-item__start-detail, .mdc-list-item__end-detail {
    color: rgba(0, 0, 0, 0.54);
  }
  .ahref-model {
    color: rgba(0, 0, 0, 0.87);
  }
}

/* fix for ie 10+ */
@media all and (-ms-high-contrast: none), (-ms-high-contrast: active) {
  .mdc-list-item__start-detail, .mdc-list-item__end-detail {
    color: rgba(0, 0, 0, 0.54);
  }
  .ahref-model {
    color: rgba(0, 0, 0, 0.87);
  }
}

</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/list/mdc-list";
  @import "@material/typography/mdc-typography";
  @import "@material/theme/mdc-theme";
</style>
