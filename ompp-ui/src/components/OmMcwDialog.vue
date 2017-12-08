<!-- MIT license -->
<template>

<aside 
  :id="id" 
  :class="classes"
  role="alertdialog" 
  aria-hidden="true" 
  :aria-labelledby="labelId" 
  :aria-describedby="descrId">

  <div ref="surface" class="mdc-dialog__surface">

    <header class="mdc-dialog__header">
      <h2 :id="labelId" class="mdc-dialog__header__title">
        <slot name="header"></slot>
      </h2>
    </header>

    <section :id="descrId" class="mdc-dialog__body" :class="bodyScrollable">
      <slot></slot>
    </section>

    <footer class="mdc-dialog__footer">
      <button
        v-if="cancelText"
        ref="cancel"
        type="button"
        class="mdc-button mdc-dialog__footer__button mdc-dialog__footer__button--cancel">
        {{cancelText}}
      </button>
      <button 
        ref="accept"
        type="button"
        class="mdc-button mdc-dialog__footer__button mdc-dialog__footer__button--accept">
        {{acceptText}}
      </button>
    </footer>

  </div>
  <div v-if="!opened" class="mdc-dialog__backdrop"></div>

</aside>

</template>

<script>
import MDCDialogFoundation from '@material/dialog/foundation'
import { createFocusTrapInstance } from '@material/dialog/util'
import OmMcwButton from './OmMcwButton'

export default {
  components: { OmMcwButton },

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
        'mdc-dialog': true,
        'mdc-dialog--open': this.opened
      },
      bodyScrollable: this.scrollable ? 'mdc-dialog__body--scrollable' : '',
      foundation: null,
      focusTrap: null
    }
  },

  computed: {
    labelId () { return (this.id || '') + '__label' },
    descrId () { return (this.id || '') + '__descr' }
  },

  methods: {
    open () {
      this.foundation.open()
    },
    close () {
      this.foundation.close()
    },
    isOpen () {
      return this.foundation.isOpen()
    }
  },

  mounted () {
    let vm = this

    this.focusTrap = createFocusTrapInstance(this.$refs.surface, this.$refs.accept)

    this.foundation = new MDCDialogFoundation({
      addClass: (className) => vm.$set(vm.classes, className, true),
      removeClass: (className) => vm.$delete(vm.classes, className),
      setStyle: (prop, val) => vm.$el.style.setProperty(prop, val),
      addBodyClass: (className) => document.body.classList.add(className),
      removeBodyClass: (className) => document.body.classList.remove(className),
      eventTargetHasClass: (target, className) => target.classList.contains(className),
      registerInteractionHandler: (evt, handler) => vm.$el.addEventListener(evt, handler),
      deregisterInteractionHandler: (evt, handler) => vm.$el.removeEventListener(evt, handler),
      registerSurfaceInteractionHandler: (evt, handler) => vm.$refs.surface.addEventListener(evt, handler),
      deregisterSurfaceInteractionHandler: (evt, handler) => vm.$refs.surface.removeEventListener(evt, handler),
      registerDocumentKeydownHandler: (handler) => document.addEventListener('keydown', handler),
      deregisterDocumentKeydownHandler: (handler) => document.removeEventListener('keydown', handler),
      registerTransitionEndHandler: (handler) => vm.$refs.surface.addEventListener('transitionend', handler),
      deregisterTransitionEndHandler: (handler) => vm.$refs.surface.removeEventListener('transitionend', handler),
      notifyAccept: () => vm.$emit('accept'),
      notifyCancel: () => vm.$emit('cancel'),
      isDialog: (el) => el === vm.$refs.surface,
      trapFocusOnSurface: () => vm.focusTrap.activate(),
      untrapFocusOnSurface: () => vm.focusTrap.deactivate(),
      layoutFooterRipples: () => {
        if (vm.$refs.accept) {
          if (vm.$refs.accept.hasOwnProperty('ripple')) vm.$refs.accept.ripple.layout()
        }
        if (vm.cancelText && vm.$refs.cancel) {
          if (vm.$refs.cancel.hasOwnProperty('ripple')) vm.$refs.cancel.ripple.layout()
        }
      }
    })

    this.foundation.init()
  },

  beforeDestroy () {
    this.foundation.destroy()
  }
}
</script>

<!-- import MDC styles -->
<style lang="scss">
  @import "@material/button/mdc-button";
  @import "@material/dialog/mdc-dialog";
</style>
