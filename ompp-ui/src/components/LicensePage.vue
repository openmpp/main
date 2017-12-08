<template>
  <div id="license-page" class="mdc-typography">
    <div v-if="loadDone" class="lic-text mdc-typography--body1">{{ msg }}</div>
    <div v-else class="mdc-typography--body1">
      <span v-if="loadWait" class="material-icons om-mcw-spin">star</span><span>{{msg}}</span>
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
      loadWait: false,
      msg: ''
    }
  },

  computed: {
  },

  methods: {
    // refersh page content
    async doRefresh () {
      this.loadDone = false
      this.loadWait = true
      this.msg = 'Loading...'
      try {
        const response = await axios.get('/static/LICENSE.txt')
        this.msg = response.data
        this.loadDone = true
      } catch (e) {
        this.msg = 'LICENSE.txt not found'
      }
      this.loadWait = false
    }
  },

  mounted () {
    this.doRefresh()
  }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>

  /* license text style */
  .lic-text {
    font-family: "Roboto Mono", monospace;
    white-space: pre;
  }
</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
