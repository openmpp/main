<!-- reload run-text-list by digest -->
<template>

<span id="refresh-run-list" v-show="!loadDone" class="mdc-typography--caption">
  <span v-show="loadWait" class="material-icons om-mcw-spin">star</span><span>{{msgLoad}}</span>
</span>
  
</template>

<script>
import axios from 'axios'
import { mapGetters, mapActions } from 'vuex'
import { GET, SET } from '@/store'

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
      this.doRefreshRunTextList() // reload run list
    },
    digest () {
      this.doRefreshRunTextList() // reload run list
    }
  },

  methods: {
    // refersh run list
    async doRefreshRunTextList () {
      let u = this.omppServerUrl + '/api/model/' + (this.digest || '') + '/run-list/text' + (this.uiLang !== '' ? '/lang/' + this.uiLang : '')
      this.loadDone = false
      this.loadWait = true
      this.msgLoad = 'Loading run list...'
      this.$emit('wait')
      try {
        const response = await axios.get(u)
        this.setRunTextList(response.data)   // update run list in store
        this.loadDone = true
      } catch (e) {
        this.msgLoad = '<Server offline or no model runs published>'
        console.log('Server offline or no model runs published')
      }
      this.loadWait = false
      this.$emit('done', this.loadDone)
    },
    ...mapActions({
      setRunTextList: SET.RUN_TEXT_LIST
    })
  },

  mounted () {
    this.doRefreshRunTextList() // reload run list
  }
}
</script>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
