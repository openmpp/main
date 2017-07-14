<template>

<div id="model-page" class="mdc-typography">

  <div v-if="loadDone" class="mdc-typography--body1">

    <p>{{ modelDef.Name }} : {{ modelDef.Digest }} </p>

    <a href="#" 
      v-if="isModelNote(modelDef)" 
      @click="showModelNote(modelDef)" 
      class="material-icons mdc-list-item__end-detail" 
      title="Notes" 
      aria-hidden="true">info_outline</a>

  </div>

  <div v-else>
    <span class="fa fa-refresh fa-spin fa-fw fa-2x"></span><span>{{msg}}</span>
  </div>

  <div>
    <span>Model:</span><span>{{modelDef}}</span>
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

export default {
  props: {
    digest: ''
  },

  data () {
    return {
      loadDone: false,
      modelDef: {},
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
    // refersh current model
    async doRefresh () {
      let u = this.omppServerUrl + '/api/model-text/' + (this.digest || '') + (this.uiLang !== '' ? '/' + this.uiLang : '')
      this.loadDone = false
      this.msg = 'Loading...'
      try {
        const response = await axios.get(u)
        this.modelDef = response.data
        this.setTheModel(this.modelDef)   // update current model in store
      } catch (e) {
        this.msg = 'Server offline or model not found'
      }
      this.loadDone = true
    },

    // is model notes not empty
    isModelNote (md) {
      if (!md) return false
      if (!md.hasOwnProperty('TxtRow')) return false
      if (!md.TxtRow.hasOwnProperty('Note')) return false
      return (md.TxtRow.Note || '') !== ''
    },

    showModelNote (md) {
      if (!md) return false
      this.titleNoteDlg = md.ModelRow.Name
      this.textNoteDlg = md.TxtRow.Note
      this.$refs.noteDlg.open()
    },

    ...mapMutations({
      setTheModel: SET.THE_MODEL
    })
  },

  mounted () {
    this.doRefresh()
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

<!-- MDC styles expected to be imported by App -->
<!-- 
<style lang="scss">
  @import "@material/list/mdc-list";
  @import "@material/typography/mdc-typography";
</style>
-->
