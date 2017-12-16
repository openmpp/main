<template>
  <div id="license-page" class="mdc-typography mdc-typography--body1">
   <span v-if="loadWait" class="material-icons om-mcw-spin">star</span>{{ msg }}
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
  #license-page {
    display: block;
    height: 100%;
    width: 100%;
    overflow: auto;
    white-space: pre;
    font-family: "Roboto Mono", monospace;
  }
  /*
  #license-page {
    flex: 1 0 auto; 
    height: 100%;
    width: 100%;
    overflow: auto;
    white-space: pre;
    font-family: "Roboto Mono", monospace;
  }
  */
</style>

<!-- MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>
