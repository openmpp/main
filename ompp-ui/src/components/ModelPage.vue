<template>

<div id="model-page" class="mdc-typography">

  <div v-if="loadDone" class="mdc-typography--body1">

      <i class="fa fa-subscript fa-lg" role="presentation" aria-hidden="true"></i>
      <span>{{ modelName }}
        <span>{{ modelDescr }}</span>
      </span>

    <a href="#" 
      v-if="isModelNote(modelDef)" 
      @click="showModelNote(modelDef)" 
      class="material-icons" 
      title="Notes" 
      aria-hidden="true">info_outline</a>

  </div>

  <div v-else>
    <span class="fa fa-refresh fa-spin fa-fw fa-2x"></span><span>{{msg}}</span>
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
import { default as mC } from '@/modelCommon'

export default {
  props: {
    digest: ''
  },

  data () {
    return {
      loadDone: false,
      modelDef: {
        Model: {
          Name: '',
          Digest: ''
        }
      },
      titleNoteDlg: '',
      textNoteDlg: '',
      msg: ''
    }
  },

  computed: {
    modelName () { return mC.modelName(this.modelDef) },
    modelDescr () { return mC.modelDescr(this.modelDef) },

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
        this.setTheModel(response.data)   // update current model in store
        this.modelDef = this.theModel
      } catch (e) {
        this.msg = 'Server offline or model not found'
      }
      this.loadDone = true
    },

    // if model notes not empty
    isModelNote (md) {
      return mC.isModelNote(md)
    },
    // then show model notes
    showModelNote (md) {
      let d = mC.modelDescr(md)
      this.titleNoteDlg = mC.modelName(md) + (d !== '' ? ': ' + d : '')
      this.textNoteDlg = mC.modelNote(md)
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
