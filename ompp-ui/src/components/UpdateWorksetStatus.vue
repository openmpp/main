<!-- update workset readonly status by model digest and workset name -->
<template>

<span id="update-workset-status" v-show="!loadDone" class="mdc-typography--caption">
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
    worksetName: '',
    enableEdit: false,
    saveWsStatusTickle: false
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
    // start status update handler
    saveWsStatusTickle () { this.doUpdateWsStatus() }
  },

  methods: {
    // update workset readonly status
    async doUpdateWsStatus () {
      let u = this.omppServerUrl +
        '/api/model/' + (this.modelDigest || '') +
        '/workset/' + (this.worksetName || '') +
        '/readonly/' + (!this.enableEdit).toString()
      this.loadDone = false
      this.loadWait = true
      this.msgLoad = 'Updating workset...'
      this.$emit('wait')
      try {
        const response = await axios.post(u)
        const rsp = response.data
        if ((rsp || '') !== '') console.log('Server reply:', rsp)
        this.loadDone = true
      } catch (e) {
        this.msgLoad = '<Server offline or no model input set not found>'
        console.log('Server offline or no model input set not found')
      }
      this.loadWait = false
      this.$emit('done', this.loadDone)
    }
  },

  mounted () {
  }
}
</script>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
