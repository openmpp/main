<!-- based on petejohanson/vue-mdc, MIT license -->
<!--                                            -->
<template>

<aside :class="classes">
  <nav ref="drawer" class="mdc-persistent-drawer__drawer om-mcw-drawer-bottom">

    <header class="mdc-persistent-drawer__header" v-if="$slots['header'] || drawerHeader">
      <slot name="header"></slot>
    </header>
    <div class="mdc-persistent-drawer__toolbar-spacer" v-else></div>

    <div class="mdc-list-group" role="menu">
      <slot></slot>
    </div>

  </nav>
</aside>

</template>

<script>
import { MDCPersistentDrawerFoundation } from '@material/drawer'
import * as util from '@material/drawer/util'

export default {
  props: {
    drawerHeader: Boolean
  },
  data () {
    return {
      classes: {
        'mdc-persistent-drawer': true
      },
      foundation: null
    }
  },

  methods: {
    toggle () {
      this.foundation.isOpen() ? this.foundation.close() : this.foundation.open()
    }
  },

  mounted () {
    const {FOCUSABLE_ELEMENTS} = MDCPersistentDrawerFoundation.strings

    let vm = this
    this.foundation = new MDCPersistentDrawerFoundation({
      addClass (className) {
        vm.$set(vm.classes, className, true)
      },
      removeClass (className) {
        vm.$delete(vm.classes, className)
      },
      hasClass (className) {
        return Boolean(vm.classes[className]) || (vm.$el && vm.$el.classList.contains(className))
      },
      hasNecessaryDom () {
        return Boolean(vm.$refs.drawer)
      },
      registerInteractionHandler (evt, handler) {
        vm.$el.addEventListener(evt, handler)
      },
      deregisterInteractionHandler (evt, handler) {
        vm.$el.removeEventListener(evt, handler)
      },
      registerDrawerInteractionHandler (evt, handler) {
        vm.$refs.drawer.addEventListener(evt, handler)
      },
      deregisterDrawerInteractionHandler (evt, handler) {
        vm.$refs.drawer.removeEventListener(evt, handler)
      },
      registerTransitionEndHandler (handler) {
        vm.$refs.drawer.addEventListener('transitionend', handler)
      },
      deregisterTransitionEndHandler (handler) {
        vm.$refs.drawer.removeEventListener('transitionend', handler)
      },
      registerDocumentKeydownHandler (handler) {
        document.addEventListener('keydown', handler)
      },
      deregisterDocumentKeydownHandler (handler) {
        document.removeEventListener('keydown', handler)
      },
      getDrawerWidth () {
        return vm.$refs.drawer.offsetWidth
      },
      setTranslateX (value) {
        vm.$refs.drawer.style.setProperty(
          util.getTransformPropertyName(),
          value === null ? null : `translateX(${value}px)`
        )
      },
      getFocusableElements () {
        return vm.$refs.drawer.querySelectorAll(FOCUSABLE_ELEMENTS)
      },
      saveElementTabState (el) {
        util.saveElementTabState(el)
      },
      restoreElementTabState (el) {
        util.restoreElementTabState(el)
      },
      makeElementUntabbable (el) {
        el.setAttribute('tabindex', -1)
      },
      notifyOpen () {
        vm.$emit('opened')
      },
      notifyClose  () {
        vm.$emit('closed')
      },
      isRtl () {
        /* global getComputedStyle */
        return getComputedStyle(vm.$el).getPropertyValue('direction') === 'rtl'
      },
      isDrawer (el) {
        el === vm.$refs.drawer
      }
    })

    this.foundation.init()
  },

  beforeDestroy () {
    this.foundation.destroy()
  }
}

</script>

<!-- this component only css -->
<style scoped>
.om-mcw-drawer-bottom {
  border-bottom: 1px solid rgba(0, 0, 0, .12);
}
</style>

<!-- import MDC styles -->
<style lang="scss">
  @import "@material/drawer/mdc-drawer";
  @import "@material/elevation/mdc-elevation";
  @import "@material/list/mdc-list";
</style>
