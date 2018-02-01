<!-- run the model and monitor progress -->
<template>

<span id="new-run-model" v-show="!loadDone" class="mdc-typography--caption">
  <span v-show="loadWait" class="material-icons om-mcw-spin">star</span><span>{{msgLoad}}</span>
</span>
  
</template>

<script>
import axios from 'axios'
import { mapGetters } from 'vuex'
import { GET } from '@/store'

export default {
  props: {
    modelDigest: '',
    newRunName: '',
    worksetName: ''
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
      omppServerUrl: GET.OMPP_SRV_URL
    })
  },

  watch: {
    // refresh handlers
  },

  methods: {
    // refersh run text
    async doNewRunInit () {
      if ((this.modelDigest || '') === '') return   // exit: model digest unknown

      // set new run parameters and model run url
      let rv = {
        RunName: (this.newRunName || ''),
        SetName: (this.worksetName || '')
      }
      let u = this.omppServerUrl + '/api/model/' + (this.modelDigest || '') + '/new-run'

      // empty result
      let runState = { RunKey: '', StartDateTime: '', UpdateDateTime: '' }

      this.loadDone = false
      this.loadWait = true
      this.msgLoad = 'Loading model run...'
      this.$emit('wait')
      try {
        // send data page to the server, response body expected to be empty
        const response = await axios.post(u, rv)
        runState = response.data
        this.loadDone = true
      } catch (e) {
        this.msgLoad = '<Server offline or run start failed>'
        console.log('Server offline or run start failed')
      }
      this.loadWait = false
      this.$emit('done', this.loadDone, runState)
    }
  },

  mounted () {
    this.$emit('mounted')
    this.doNewRunInit()
  },
  beforeDestroy () {
  }
}
</script>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
