<template>
  <div id="under-construction-page" class="mdc-typography">
    <i class="material-icons om-mcw-stop-icon" aria-hidden="true">do_not_disturb_on</i>
    <span class="mdc-typography--body1">Work in progress, please try again later.</span>
  </div>
</template>

<script>
export default {
  data () {
    return {
    }
  }
}
</script>

<!-- this component only css -->
<style scoped>

#under-construction-page {
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
