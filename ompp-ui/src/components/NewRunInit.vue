<!-- initiate (start) new model run: send request to the server -->
<template>

<span id="new-run-init" v-show="!loadDone" class="mdc-typography--caption">
  <span v-show="loadWait" class="material-icons om-mcw-spin">hourglass_empty</span><span>{{msgLoad}}</span>
</span>

</template>

<script>
import axios from 'axios'
import { mapGetters } from 'vuex'
import { GET } from '@/store'
import * as Mdf from '@/modelCommon'

export default {
  props: {
    modelDigest: '',
    newRunName: '',
    worksetName: '',
    subCount: 0
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
    // initiate new model run: send request to the server
    async doNewRunInit () {
      if ((this.modelDigest || '') === '') return // exit: model digest unknown

      this.loadDone = false
      this.loadWait = true
      this.msgLoad = 'Starting model run...'
      this.$emit('wait')

      // set new run parameters
      let rv = {
        ModelDigest: (this.modelDigest || ''),
        Opts: {
          'OpenM.SubValues': (this.subCount || 1).toString(),
          'OpenM.RunName': (this.newRunName || ''),
          'OpenM.SetName': (this.worksetName || '')
        }
      }
      let rst = Mdf.emptyRunState()
      let u = this.omppServerUrl + '/api/run'

      try {
        // send data page to the server, response body expected to be empty
        const response = await axios.post(u, rv)
        rst = response.data
        this.loadDone = true
      } catch (e) {
        this.msgLoad = '<Server offline or run start failed>'
        console.log('Server offline or run start failed')
      }
      this.loadWait = false

      // return run status
      if (!Mdf.isRunState(rst)) rst = Mdf.emptyRunState()
      this.$emit('done', this.loadDone, rst)
    }
  },

  mounted () {
    this.doNewRunInit()
  }
}
</script>
