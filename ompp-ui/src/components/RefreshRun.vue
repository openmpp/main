<!-- reload run-text by model digest and run digest -->
<template>

<span id="refresh-run" v-show="!loadDone" class="mdc-typography--caption">
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
    runDigest: '',
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
      this.doRefreshRunText() // reload run
    },
    runDigest () {
      this.doRefreshRunText() // reload run
    }
  },

  methods: {
    // refersh run text
    async doRefreshRunText () {
      let u = this.omppServerUrl + '/api/model/' + (this.modelDigest || '') + '/run/' + (this.runDigest || '') + '/text' + (this.uiLang !== '' ? '/lang/' + this.uiLang : '')
      this.loadDone = false
      this.loadWait = true
      this.msgLoad = 'Loading model run...'
      this.$emit('wait')
      try {
        const response = await axios.get(u)
        this.setRunText(response.data)   // update run text in store
        this.loadDone = true
      } catch (e) {
        this.msgLoad = '<Server offline or model run not found>'
        console.log('Server offline or model run not found')
      }
      this.loadWait = false
      this.$emit('done', this.loadDone)
    },
    ...mapActions({
      setRunText: SET.THE_RUN_TEXT
    })
  },

  mounted () {
    this.doRefreshRunText() // reload run
  }
}
</script>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
