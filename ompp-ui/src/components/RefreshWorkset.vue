<!-- reload workset-text by model digest and workset name -->
<template>

<span id="refresh-workset" v-show="!loadDone" class="mdc-typography--caption">
  <span v-show="loadWait" class="material-icons om-mcw-spin">star</span><span>{{msgLoad}}</span>
</span>
  
</template>

<script>
import axios from 'axios'
import { mapGetters, mapActions } from 'vuex'
import { GET, SET } from '@/store'

export default {
  props: {
    modelDigest: '',
    worksetName: '',
    refreshTickle: false,
    refreshWsTickle: false
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
    // refresh handlers
    refreshTickle () { this.doRefreshWsText() },
    refreshWsTickle () { this.doRefreshWsText() },
    worksetName () { this.doRefreshWsText() }
  },

  methods: {
    // refersh workset text
    async doRefreshWsText () {
      this.loadDone = false
      this.loadWait = true
      this.msgLoad = 'Loading workset...'
      this.$emit('wait')

      let u = this.omppServerUrl + '/api/model/' + (this.modelDigest || '') + '/workset/' + (this.worksetName || '') + '/text' + (this.uiLang !== '' ? '/lang/' + this.uiLang : '')
      try {
        const response = await axios.get(u)
        this.setWorksetText(response.data)   // update workset text in store
        this.loadDone = true
      } catch (e) {
        this.msgLoad = '<Server offline or no model input set not found>'
        console.log('Server offline or no model input set not found')
      }
      this.loadWait = false
      this.$emit('done', this.loadDone)
    },

    ...mapActions({
      setWorksetText: SET.THE_WORKSET_TEXT
    })
  },

  mounted () {
    this.doRefreshWsText() // reload workset
  }
}
</script>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
