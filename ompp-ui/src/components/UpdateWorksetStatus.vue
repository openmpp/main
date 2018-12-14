<!-- update workset readonly status by model digest and workset name -->
<template>

<span id="update-workset-status" v-show="!loadDone" class="mdc-typography--caption">
  <span v-show="loadWait" class="material-icons om-mcw-spin">hourglass_empty</span><span>{{msgLoad}}</span>
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
      this.loadDone = false
      this.loadWait = true
      this.msgLoad = 'Updating workset...'
      this.$emit('wait')

      let u = this.omppServerUrl +
        '/api/model/' + (this.modelDigest || '') +
        '/workset/' + (this.worksetName || '') +
        '/readonly/' + (!this.enableEdit).toString()
      try {
        const response = await axios.post(u, { }, { responseType: 'text' })
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
