<template>

<div id="home-page" class="mdc-typography">

  <div v-if="loadDone" class="mdc-typography--body1">
    <ul class="mdc-list mdc-list--two-line">

      <li v-for="m in modelList" :key="m.Model.Digest" class="mdc-list-item">

        <a href="#" 
          v-if="isModelNote(m)" 
          @click="showModelNote(m)" 
          class="material-icons mdc-list-item__start-detail model-note" 
          title="Notes" 
          aria-hidden="true">info_outline</a>
        <a href="#" 
          v-else 
          @click="showModelNote(m)" 
          class="material-icons mdc-list-item__start-detail model-note-disabled" 
          title="Notes" 
          aria-hidden="true">info</a>

        <router-link :to="'/model/' + m.Model.Digest" class="ahref-model">
          <span class="mdc-list-item__text">{{ m.Model.Name }}
            <span class="mdc-list-item__text__secondary">{{ m.DescrNote.Descr }}</span>
          </span>
        </router-link>

      </li>

    </ul>
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
import OmMcwDialog from './OmMcwDialog'
import { default as Mdf } from '@/modelCommon'

export default {
  props: {
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
      modelList: GET.MODEL_LIST,
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
    // if model notes not empty
    isModelNote (md) {
      return Mdf.isModelNote(md)
    },
    // then show model notes
    showModelNote (md) {
      this.titleNoteDlg = Mdf.modelTitle(md)
      this.textNoteDlg = Mdf.modelNote(md)
      this.$refs.noteDlg.open()
    },

    // refersh model list
    async doRefresh () {
      let u = this.omppServerUrl + '/api/model-list-text' + (this.uiLang !== '' ? '/' + this.uiLang : '')
      this.loadDone = false
      this.msg = 'Loading...'
      try {
        const response = await axios.get(u)
        this.setModelList(response.data)   // update model list in store
      } catch (e) {
        this.msg = 'Server offline or no models published'
      }
      this.loadDone = true
    },

    ...mapMutations({
      setModelList: SET.MODEL_LIST
    })
  },

  mounted () {
    // if model list already loaded then exit
    if (Mdf.isModelList(this.modelList) && this.modelList.length > 0) {
      this.loadDone = true
      return
    }
    this.doRefresh() // load new model list
  },

  components: { OmMcwDialog }
}

</script>

<!-- this component only css -->
<style scoped lang="scss">

.ahref-model {
  display: block;
  width: 100%;
  height: 100%;
  text-decoration: none;
  color: rgba(0, 0, 0, 0.87);
  color: var(--mdc-theme-text-primary-on-background, rgba(0, 0, 0, 0.87));
}

a.ahref-model:hover {
  background: rgba(0, 0, 0, 0.1);
}

.model-note, .model-note-disabled {
  display: inline-block;
  vertical-align: top;
  height: 100%;
  margin: 0;
  text-decoration: none;
  padding-left: 0.5em;
  padding-right: 0.5em;
}

a.model-note, a.model-note-disabled {
  outline: none;
}

a.model-note:hover {
  background: rgba(0, 0, 0, 0.1);
}

a.model-note-disabled {
  pointer-events: none;
  cursor: default;
}

.model-note, .model-note-disabled {
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
