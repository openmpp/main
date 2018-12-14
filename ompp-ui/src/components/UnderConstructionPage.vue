<template>
  <div id="under-construction-page" class="mdc-typography mdc-typography--body1">
    <i class="material-icons stop-icon" aria-hidden="true">remove_circle_outline</i>
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

<!-- local scope css: this component only -->
<style scoped>

  #under-construction-page {
    text-align: center;
  }

  .stop-icon {
    display: inline;
    vertical-align: middle;
  }
</style>
