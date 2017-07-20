<template>

<div id="home-page" class="mdc-typography">

  <div v-if="loadDone" class="mdc-typography--body1">
    <ul class="mdc-list mdc-list--two-line">

      <li v-for="m in modelLst" :key="m.Model.Digest" class="mdc-list-item">

        <a href="#" 
          v-if="isModelNote(m)" 
          @click="showModelNote(m)" 
          class="material-icons mdc-list-item__start-detail" 
          title="Notes" 
          aria-hidden="true">info_outline</a>
        <a href="#" 
          v-else 
          @click="showModelNote(m)" 
          class="material-icons mdc-list-item__start-detail info-disabled" 
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
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import OmMcwDialog from './OmMcwDialog'
import { default as mC } from '@/modelCommon'

export default {
  data () {
    return {
      loadDone: false,
      modelLst: [],
      titleNoteDlg: '',
      textNoteDlg: '',
      msg: ''
    }
  },

  computed: {
    ...mapGetters({
      uiLang: GET.UI_LANG,
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  methods: {
    // refersh model list
    async doRefresh () {
      let u = this.omppServerUrl + '/api/model-list-text/' + (this.uiLang !== '' ? this.uiLang : '')
      this.loadDone = false
      this.msg = 'Loading...'
      try {
        const response = await axios.get(u)
        this.modelLst = response.data
      } catch (e) {
        this.msg = 'Server offline or no models published'
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
    }
  },

  mounted () {
    this.doRefresh()
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
  color: var(--mdc-theme-text-primary-on-background, rgba(0, 0, 0, 0.87));
}

a.ahref-model:hover {
  background: rgba(0, 0, 0, 0.1);
}

.mdc-list-item__start-detail {
  display: inline;
  vertical-align: middle;
  margin: 0;
  text-decoration: none;
}

.mdc-list-item__end-detail {
  display: inline-block;
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

a.info-disabled {
  pointer-events: none;
  cursor: default;
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
