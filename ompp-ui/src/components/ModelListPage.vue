<template>

<div id="model-list-page" class="main-container mdc-typography mdc-typography--body1">

  <div v-if="loadDone" class="main-container">
    <ul class="main-list mdc-list mdc-list--two-line">

      <li v-for="m in modelList" :key="m.Model.Digest" class="mdc-list-item">
        <template v-if="isModel(m)">
          <a href="#" 
            @click="showModelNote(m)" 
            class="material-icons mdc-list-item__start-detail note-link" 
            :title="m.Model.Name + ' notes'"
            :alt="m.Model.Name + ' notes'">event_note</a>
          <router-link
            :to="'/model/' + m.Model.Digest" 
            class="ahref-model" 
            :title="m.Model.Name + ' model'" 
            :alt="m.Model.Name + ' model'"
            >
            <span class="mdc-list-item__text">{{ m.Model.Name }}
              <span class="mdc-list-item__secondary-text">{{ descrOf(m) }}</span>
            </span>
          </router-link>
        </template>
      </li>

    </ul>
  </div>
  <div v-else>
    <span v-if="loadWait" class="material-icons om-mcw-spin">star</span><span class="mdc-typography--caption">{{msg}}</span>
  </div>

  <om-mcw-dialog ref="noteDlg" id="note-dlg" acceptText="OK">
    <span slot="header">{{titleNoteDlg}}</span>
    <div>{{textNoteDlg}}</div>
    <br/>
    <div class="mono">Name:&nbsp;&nbsp;&nbsp;&nbsp;{{nameNoteDlg}}</div>
    <div class="mono">Created:&nbsp;{{createdNoteDlg}}</div>
    <div class="mono">Digest:&nbsp;&nbsp;{{digestNoteDlg}}</div>
  </om-mcw-dialog>

</div>
  
</template>

<script>
import axios from 'axios'
import { mapGetters, mapActions } from 'vuex'
import { GET, SET } from '@/store'
import * as Mdf from '@/modelCommon'
import OmMcwDialog from './OmMcwDialog'

export default {
  props: {
    refreshTickle: false
  },

  data () {
    return {
      loadDone: false,
      loadWait: false,
      titleNoteDlg: '',
      textNoteDlg: '',
      nameNoteDlg: '',
      createdNoteDlg: '',
      digestNoteDlg: '',
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
    // return true if Model not empty
    isModel (md) { return Mdf.isModel(md) },

    // return description from DescrNote
    descrOf (md) {
      return Mdf.descrOfDescrNote(md)
    },

    // if notes not empty
    isModelNote (md) {
      return Mdf.isNoteOfDescrNote(md)
    },
    // then show model notes
    showModelNote (md) {
      this.titleNoteDlg = Mdf.descrOfDescrNote(md)
      this.textNoteDlg = Mdf.noteOfDescrNote(md)
      this.nameNoteDlg = Mdf.modelName(md)
      this.createdNoteDlg = Mdf.dtStr(md.Model.CreateDateTime)
      this.digestNoteDlg = Mdf.modelDigest(md)
      this.$refs.noteDlg.open()
    },

    // refersh model list
    async doRefresh () {
      let u = this.omppServerUrl + '/api/model-list/text' + (this.uiLang !== '' ? '/lang/' + this.uiLang : '')
      this.loadDone = false
      this.loadWait = true
      this.msg = 'Loading...'
      try {
        const response = await axios.get(u)
        this.setModelList(response.data)   // update model list in store
        this.loadDone = true
      } catch (e) {
        this.msg = 'Server offline or no models published'
        console.log('Server offline or no models published')
      }
      this.loadWait = false
    },

    ...mapActions({
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

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";

  /* model list: containers and list itself */
  .main-container {
    display: block; 
    overflow-y: auto;
  }
  .main-list {
    padding-left: 0;
  }

  /* model list: containers and list itself */
  /*
  .main-container {
    height: 100%;
    flex: 1 1 auto;
    display: flex; 
    flex-direction: column;
    overflow-y: auto;
  }
  .main-list {
    padding-left: 0;
  }
  */

  /* model a link */
  .ahref-model {
    display: block;
    width: 100%;
    height: 100%;
    text-decoration: none;
    @extend .mdc-theme--text-primary-on-background;
    &:hover {
      background: rgba(0, 0, 0, 0.1);
    }
  }

  /* notes: a link or empty (not a link) */
  .note-item {
    display: inline-block;
    vertical-align: top;
    height: 100%;
    margin: 0;
    padding-left: 0.5rem;
    padding-right: 0.5rem;
  }
  .note-link {
    @extend .note-item;
    text-decoration: none;
    outline: none;
    @extend .mdc-theme--text-secondary-on-background;
  }
  a.note-link:hover {
    background: rgba(0, 0, 0, 0.1);
  }
  .note-empty {
    @extend .note-item;
    cursor: default;
    @extend .mdc-theme--text-disabled-on-background;
  }
</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/list/mdc-list";
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
