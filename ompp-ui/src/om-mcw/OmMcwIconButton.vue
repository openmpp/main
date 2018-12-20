<!-- Material Components Web simple wrapper for MDCIconButton with ripple effect -->
<template>

<button class="mdc-icon-button" :disabled="disabled" @click="doClick">
  <slot></slot>
</button>

</template>

<script>
import { MDCIconButtonToggle } from '@material/icon-button'
import { MDCRipple } from '@material/ripple'

export default {
  props: {
    disabled: Boolean
  },
  data () {
    return {
      mdcIconButton: void 0,
      mdcRipple: void 0
    }
  },

  methods: {
    doClick (event) {
      this.$emit(event.type)
    }
  },

  mounted () {
    this.mdcIconButton = MDCIconButtonToggle.attachTo(this.$el)
    this.mdcRipple = MDCRipple.attachTo(this.$el)
    this.mdcRipple.unbounded = true
  },
  beforeDestroy () {
    if (this.mdcRipple !== void 0) this.mdcRipple.destroy()
    if (this.mdcIconButton !== void 0) this.mdcIconButton.destroy()
  }
}
</script>

<!-- import MDC styles -->
<style lang="scss">
  @import '@material/icon-button/mdc-icon-button';
</style>
