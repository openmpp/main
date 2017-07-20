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
    dense: Boolean,
    raised: Boolean,
    compact: Boolean,
    primary: Boolean,
    accent: Boolean,
    materialIcon: Boolean
  },
  data () {
    return {
      classes: {
        'mdc-button': true,
        'mdc-button--dense': this.dense,
        'mdc-button--raised': this.raised,
        'mdc-button--compact': this.compact,
        'mdc-button--primary': this.primary,
        'mdc-button--accent': this.accent,
        'button-m-icon': this.materialIcon
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

<!-- this component only css -->
<style scoped>

.button-m-icon {
  display: inline;
  vertical-align: middle;
}

</style>

<!-- import MDC styles -->
<style lang="scss">
  @import '@material/button/mdc-button';
</style>
