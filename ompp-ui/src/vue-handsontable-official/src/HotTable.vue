<template>
  <div :id="this.root"></div>
</template>

<script>
  import {
    hotInit,
    hotDestroy,
    propFactory,
    propWatchFactory,
    updateHotSettings,
    updateBulkHotSettings
  } from './helpers'

  export default {
    name: 'HotTable',
    props: propFactory(),
    watch: propWatchFactory(updateHotSettings, updateBulkHotSettings),
    mounted: function () { return hotInit(this) },
    beforeDestroy: function () { return hotDestroy(this) }
  }
</script>

<style>
  @import "handsontable/dist/handsontable.full.css";
</style>
