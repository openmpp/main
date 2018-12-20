<!-- Material Components Web simple wrapper for MDCIconButton with toggle state ripple effect -->
<template>

<button
  class="mdc-icon-toggle-button"
  :class="{ 'mdc-icon-button--on': isNowOn }"
  :disabled="disabled"
  :aria-pressed="{isNowOn}"
  aria-hidden="true"
  @MDCIconButtonToggle:change="onToggle">
  <i class="material-icons mdc-icon-button__icon mdc-icon-button__icon--on">{{iconOn}}</i>
  <i class="material-icons mdc-icon-button__icon">{{iconOff}}</i>
</button>

</template>

<script>
import { MDCIconButtonToggle } from '@material/icon-button'
import { MDCRipple } from '@material/ripple'

export default {
  props: {
    iconOn: { type: String, required: true },
    iconOff: { type: String, required: true },
    isNowOn: Boolean,
    disabled: Boolean
  },
  data () {
    return {
      mdcIconButton: void 0,
      mdcRipple: void 0
    }
  },

  methods: {
    onToggle (evt) {
      console.log(evt.detail)
      this.$emit('toggle', evt.detail.isOn)
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
