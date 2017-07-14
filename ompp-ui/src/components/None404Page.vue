<template>
  <div id="none-404-page" class="mdc-typography mdc-typography--display">
    <p class="mdc-typography--body2">{{ msg }}</p>
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

<!-- this component only css -->
<style scoped>
</style>

<!-- import MDC styles -->
<style lang="scss">
  @import "@material/typography/mdc-typography";
</style>
