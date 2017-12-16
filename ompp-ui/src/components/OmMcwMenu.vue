<!-- originally based on stasson/vue-mdc-adapter, MIT license -->
<template>

<div ref="root" :class="classes" tabindex="-1">
  <nav ref="items" class="mdc-simple-menu__items mdc-list" role="menu" aria-hidden="true">
    <slot></slot>
  </nav>
</div>

</template>

<script>
import MDCSimpleMenuFoundation from '@material/menu/simple/foundation'
import { getTransformPropertyName } from '@material/menu/util'

export default {
  props: {
    openFromTopLeft: Boolean,
    openFromTopRight: Boolean,
    openFromBottomLeft: Boolean,
    openFromBottomRight: Boolean
  },
  data () {
    return {
      classes: {
        'mdc-simple-menu': true,
        'mdc-simple-menu--open-from-top-left': this.openFromTopLeft,
        'mdc-simple-menu--open-from-top-right': this.openFromTopRight,
        'mdc-simple-menu--open-from-bottom-left': this.openFromBottomLeft,
        'mdc-simple-menu--open-from-bottom-right': this.openFromBottomRight
      },
      foundation: null,
      prevFocus: null
    }
  },

  methods: {
    toggle (options) {
      this.foundation.isOpen() ? this.foundation.close() : this.foundation.open(options)
    },
    show (options) {
      this.foundation.open(options)
    },
    hide () {
      this.foundation.close()
    },
    isOpen () {
      return this.foundation.isOpen()
    }
  },

  mounted () {
    let vm = this
    const itemsContainer = vm.$refs.items
    const items = [].slice.call(vm.$refs.items.querySelectorAll('.mdc-list-item[role]'))

    this.foundation = new MDCSimpleMenuFoundation({
      addClass: (className) => vm.$set(vm.classes, className, true),
      removeClass: (className) => vm.$delete(vm.classes, className),
      hasClass: (className) => vm.$el && vm.$el.classList.contains(className),
      hasNecessaryDom: () => Boolean(itemsContainer),
      getAttributeForEventTarget: (target, attributeName) => target.getAttribute(attributeName),
      eventTargetHasClass: (target, className) => target.classList.contains(className),
      getInnerDimensions: () => {
        return {
          width: itemsContainer.offsetWidth,
          height: itemsContainer.offsetHeight}
      },
      hasAnchor: () => vm.$el.parentElement && vm.$el.parentElement.classList.contains('mdc-menu-anchor'),
      getAnchorDimensions: () => vm.$el.parentElement.getBoundingClientRect(),
      getWindowDimensions: () => {
        return {
          width: window.innerWidth,
          height: window.innerHeight}
      },
      setScale: (x, y) => {
        vm.$el.style[getTransformPropertyName(window)] = `scale(${x}, ${y})`
      },
      setInnerScale: (x, y) => {
        itemsContainer.style[getTransformPropertyName(window)] = `scale(${x}, ${y})`
      },
      getNumberOfItems: () => items.length,
      registerInteractionHandler: (type, handler) => vm.$el.addEventListener(type, handler),
      deregisterInteractionHandler: (type, handler) => vm.$el.removeEventListener(type, handler),
      registerBodyClickHandler: (handler) => document.body.addEventListener('click', handler),
      deregisterBodyClickHandler: (handler) => document.body.removeEventListener('click', handler),
      getYParamsForItemAtIndex: (index) => {
        const {offsetTop: top, offsetHeight: height} = items[index]
        return {top, height}
      },
      setTransitionDelayForItemAtIndex: (index, value) =>
        items[index].style.setProperty('transition-delay', value),
      getIndexForEventTarget: (target) => items.indexOf(target),
      notifySelected: (evtData) => vm.$emit('selected', {
        index: evtData.index,
        item: items[evtData.index]
      }),
      notifyCancel: () => vm.$emit('cancel'),
      saveFocus: () => {
        vm.prevFocus = document.activeElement
      },
      restoreFocus: () => {
        if (vm.prevFocus) vm.prevFocus.focus()
      },
      isFocused: () => document.activeElement === vm.$el,
      focus: () => vm.$el.focus(),
      getFocusedItemIndex: () => items.indexOf(document.activeElement),
      focusItemAtIndex: (index) => items[index].focus(),
      isRtl: () => getComputedStyle(vm.$el).getPropertyValue('direction') === 'rtl',
      setTransformOrigin: (origin) => {
        vm.$el.style[`${getTransformPropertyName(window)}-origin`] = origin
      },
      setPosition: (position) => {
        vm.$el.style.left = 'left' in position ? position.left : null
        vm.$el.style.right = 'right' in position ? position.right : null
        vm.$el.style.top = 'top' in position ? position.top : null
        vm.$el.style.bottom = 'bottom' in position ? position.bottom : null
      },
      getAccurateTime: () => window.performance.now()
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
  @import "@material/list/mdc-list";
  @import "@material/menu/mdc-menu";
</style>
