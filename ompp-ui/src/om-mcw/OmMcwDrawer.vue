<!-- Material Components Web simple wrapper for MDCDrawer -->
<template>

<aside class="mdc-drawer">
  <div class="mdc-drawer__content">

    <div v-if="$slots['header']" class="mdc-drawer__header">
      <slot name="header"></slot>
    </div>

    <div class="mdc-drawer__content">
        <slot></slot>
    </div>

  </div>
</aside>

</template>

<script>
import { MDCDrawer } from '@material/drawer'

export default {
  props: {
  },
  data () {
    return {
      mdcDrawer: void 0
    }
  },

  methods: {
    toggle () {
      this.mdcDrawer.open = !this.mdcDrawer.open
    }
  },

  mounted () {
    this.mdcDrawer = MDCDrawer.attachTo(this.$el)
  },
  beforeDestroy () {
    if (this.mdcDrawer !== void 0) this.mdcDrawer.destroy()
  }
}
</script>

<!-- local scope css: this component only -->
<style scoped lang="scss">
</style>

<!-- import MDC styles -->
<style lang="scss">
  @import "@material/drawer/mdc-drawer";
</style>
