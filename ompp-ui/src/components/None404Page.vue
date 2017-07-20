<template>
  <div id="none-404-page" class="mdc-typography">
    <i class="material-icons om-mcw-stop-icon" aria-hidden="true">do_not_disturb_on</i>
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

<!-- this component only css -->
<style scoped>

#none-404-page {
  text-align: center;
}

.om-mcw-stop-icon {
  display: inline;
  vertical-align: middle;
}

</style>

<!-- import MDC styles -->
<style lang="scss">
  @import "@material/typography/mdc-typography";
</style>
