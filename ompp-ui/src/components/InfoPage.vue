<template>
  <div id="info-page" class="mdc-typography">
    <div v-if="loadDone" class="om-mcw-lic mdc-typography--body1">{{ msg }}</div>
    <div v-else>
      <span class="material-icons om-mcw-spin">refresh</span><span>{{msg}}</span>
    </div>
  </div>
</template>

<script>
import axios from 'axios'

export default {
  props: {
  },

  data () {
    return {
      loadDone: false,
      msg: 'Loading...'
    }
  },

  computed: {
  },

  methods: {
    // refersh page content
    async doRefresh () {
      this.loadDone = false
      try {
        const response = await axios.get('/static/LICENSE.txt')
        this.msg = response.data
      } catch (e) {
        this.msg = 'LICENSE.txt not found'
      }
      this.loadDone = true
    }
  },

  mounted () {
    this.doRefresh()
  }
}

</script>

<!-- this component only css -->
<style scoped lang="scss">

/* license text style */
.om-mcw-lic {
  font-family: "Roboto Mono", monospace;
  white-space: pre;
}

</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/typography/mdc-typography";
  @import "@material/theme/mdc-theme";
</style>
