<!-- reload current model by digest -->
<template>

<span id="refresh-model" v-show="!loadDone" class="mdc-typography--body1">
  <span v-show="loadWait" class="material-icons om-mcw-spin">star</span><span>{{msgLoad}} </span>
  <span class="mono">{{digest}}</span>
</span>

</template>

<script>
import axios from 'axios'
import { mapGetters, mapActions } from 'vuex'
import { GET, SET } from '@/store'
import { default as Mdf } from '@/modelCommon'

export default {
  props: {
    digest: '',
    refreshTickle: false
  },

  data () {
    return {
      loadDone: false,
      loadWait: false,
      msgLoad: ''
    }
  },

  computed: {
    ...mapGetters({
      uiLang: GET.UI_LANG,
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  watch: {
    // refresh button handler
    refreshTickle () {
      this.doRefreshModel() // reload current model
    }
  },

  methods: {
    // refersh current model
    async doRefreshModel () {
      //
      // refresh model text rows
      let u = this.omppServerUrl + '/api/model/' + (this.digest || '') + '/text' + (this.uiLang !== '' ? '/lang/' + this.uiLang : '')
      this.loadDone = false
      this.msgLoad = 'Loading model...'
      this.loadWait = true
      this.$emit('wait')
      try {
        const response = await axios.get(u)
        this.setTheModel(response.data)   // update current model in store
        this.loadDone = true
        this.$emit('done')
      } catch (e) {
        this.msgLoad = 'Server offline or model not found'
        console.log('Server offline or no models published')
      }
      this.loadWait = false

      // refresh model "words" language-specific strings
      let uw = this.omppServerUrl + '/api/model/' + (this.digest || '') + '/word-list' + (this.uiLang !== '' ? '/lang/' + this.uiLang : '')
      try {
        const response = await axios.get(uw)
        this.setWordList(response.data)  // update model words list in store
      } catch (e) {
        console.log('Model words refresh failed')
      }
    },

    ...mapActions({
      setTheModel: SET.THE_MODEL,
      setWordList: SET.WORD_LIST
    })
  },

  mounted () {
    // if model already loaded then exit
    if (Mdf.modelDigest(this.theModel) === this.digest) {
      this.loadDone = true
      this.$emit('done')
      return
    }
    this.doRefreshModel() // reload current model
  }
}
</script>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
