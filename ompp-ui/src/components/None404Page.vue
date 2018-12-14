<template>
  <div id="none-404-page" class="mdc-typography">
    <i class="material-icons stop-icon" aria-hidden="true">remove_circle_outline</i>
    <span class="mdc-typography--body1">{{ msg }}</span>
  </div>
</template>

<script>
export default {
  data () {
    return {
      msg: '404 - not found: ' +
        (((window.location.pathname || '').length > 100)
          ? (window.location.pathname || '').substr(0, 100) + '...'
          : (window.location.pathname || ''))
    }
  }
}
</script>

<!-- local scope css: this component only -->
<style scoped>

  #none-404-page {
    text-align: center;
  }

  .stop-icon {
    display: inline;
    vertical-align: middle;
  }
</style>
