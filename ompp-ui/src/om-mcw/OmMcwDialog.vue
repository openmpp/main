<!-- Material Components Web simple wrapper for MDCDialog -->
<template>

<div
  :id="id"
  class="mdc-dialog above-ht-table"
  :class="classes"
  @MDCDialog:closed="onClosed"
  role="alertdialog"
  aria-hidden="true"
  aria-modal="true"
  :aria-labelledby="labelId"
  :aria-describedby="descrId">

  <div class="mdc-dialog__container">
    <div class="mdc-dialog__surface">

      <!-- Title cannot contain leading whitespace due to mdc-typography-baseline-top() -->
      <header v-if="$slots['header']">
        <h2 :id="labelId" class="mdc-dialog__title"><!-- --><slot name="header"></slot><!-- --></h2>
      </header>

      <section :id="descrId" class="mdc-dialog__content" :class="bodyScrollable">
        <slot></slot>
      </section>

      <footer class="mdc-dialog__actions" v-if="cancelText || acceptText">
        <button
          v-if="cancelText"
          type="button" class="mdc-button mdc-dialog__button" data-mdc-dialog-action="cancel">{{cancelText}}</button>
        <button
          v-if="acceptText"
          type="button" class="mdc-button mdc-dialog__button" data-mdc-dialog-action="accept" mdc-dialog__button--default>{{acceptText}}</button>
      </footer>

    </div>
  </div>

  <div v-if="!opened" class="mdc-dialog__scrim"></div>

</div>

</template>

<script>
import { MDCDialog } from '@material/dialog'

export default {
  props: {
    id: {
      type: String,
      required: true
    },
    opened: Boolean,
    scrollable: Boolean,
    acceptText: String,
    cancelText: String
  },

  data () {
    return {
      classes: {
        'mdc-dialog--open': this.opened
      },
      bodyScrollable: this.scrollable ? 'mdc-dialog__body--scrollable' : '',
      mdcDialog: void 0
    }
  },

  computed: {
    labelId () { return (this.id || '') + '__label' },
    descrId () { return (this.id || '') + '__descr' }
  },

  methods: {
    onClosed (evtData) {
      this.$emit('closed', evtData.detail)
    },
    open () {
      this.mdcDialog.open()
    },
    cancel () {
      this.mdcDialog.close('cancel')
    },
    isOpen () {
      return this.mdcDialog.isOpen
    }
  },

  mounted () {
    this.mdcDialog = MDCDialog.attachTo(this.$el)
  },
  beforeDestroy () {
    this.mdcDialog.destroy()
  }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  /* fix handsontable z-index: 101; */
  .above-ht-table {
    z-index: 204;
  }
</style>

<!-- import MDC styles -->
<style lang="scss">
  @import "@material/button/mdc-button";
  @import "@material/dialog/mdc-dialog";
</style>
