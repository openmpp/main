<!-- based on stasson/vue-mdc-adapter, MIT license -->
<!--                                               -->
<template>

<button :class="classes" :disabled="disabled" @click="dispatchEvent">
  <slot>
  </slot>
</button>

</template>

<script>

export default {
  props: {
    disabled: Boolean,
    compact: Boolean,
    dense: Boolean,
    raised: Boolean,
    unelevated: Boolean,
    stroked: Boolean
  },
  data () {
    return {
      classes: {
        'mdc-button': true,
        'mdc-button--compact': this.compact,
        'mdc-button--dense': this.dense,
        'mdc-button--raised': this.raised,
        'mdc-button--unelevated': this.unelevated,
        'mdc-button--stroked': this.stroked
      }
    }
  },
  methods: {
    dispatchEvent (event) {
      this.$emit(event.type)
    }
  }
}
</script>

<!-- import MDC styles -->
<style lang="scss">
  @import '@material/button/mdc-button';
</style>
