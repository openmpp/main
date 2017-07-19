<template>

<div id="app" class="om-mcw-app mdc-typography">

  <om-mcw-drawer ref="drawer">
    
    <!--
    <div slot="header" class="mdc-persistent-drawer__header-content">Login</div>
    -->
    
    <nav class="mdc-list">
      <router-link to="/" class="mdc-list-item" role="menuitem">
        <i class="mdc-list-item__start-detail material-icons" aria-hidden="true">home</i>Models
      </router-link>
    </nav>
    <hr class="mdc-list-divider" role="separator">
    <nav class="mdc-list">
      <router-link to="/run" class="mdc-list-item" role="menuitem">
        <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">launch</i>Run the model
      </router-link>
      <router-link to="/service" class="mdc-list-item" role="menuitem">
        <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">queue_play_next</i>Service status
      </router-link>
      <router-link to="/history" class="mdc-list-item" role="menuitem">
        <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">history</i>Run history
      </router-link>
    </nav>
    
  </om-mcw-drawer>

  <div class="om-mcw-content">
  
    <header class="mdc-toolbar" >
      <div class="mdc-toolbar__row">
      
        <section class="mdc-toolbar__section mdc-toolbar__section--align-start">
          <button @click="toggleDrawer" class="material-icons mdc-toolbar__icon--menu" title="Menu" alt="Menu">menu</button>
          <span id="om-mcw-title" class="mdc-toolbar__title">{{mainTitle}}</span>
        </section>
         
        <section class="mdc-toolbar__section mdc-toolbar__section--align-end mdc-toolbar__section--shrink-to-fit">
          <router-link to="/login" class="material-icons mdc-toolbar__icon" title="Login" alt="Login">account_circle</router-link>
        </section>

        <section class="mdc-toolbar__section mdc-toolbar__section--align-end mdc-toolbar__section--shrink-to-fit mdc-menu-anchor">
             
          <a href="#" @click="toggleMore" class="material-icons mdc-toolbar__icon" title="More" alt="More">more_vert</a>

          <om-mcw-menu ref="more" @selected="moreSelected">
            <a href="#" class="mdc-list-item" role="menuitem" tabindex="0">
              <i class="mdc-list-item__start-detail" aria-hidden="true">EN</i>
              <span hidden="true">_lang_change_</span>English
            </a>
            <a href="#" class="mdc-list-item" role="menuitem" tabindex="0">
              <i class="mdc-list-item__start-detail" aria-hidden="true">FR</i>
              <span hidden="true">_lang_change_</span>Français
            </a>
            <hr class="mdc-list-divider" role="separator"></hr>
            <router-link to="/settings" class="mdc-list-item" role="menuitem" tabindex="0">
              <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">settings</i>Settings
            </router-link>
            <router-link to="/info" class="mdc-list-item" role="menuitem" tabindex="0">
              <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">info_outline</i>Licence
            </router-link>
            <a href="//www.openmpp.org/" target="_blank" class="mdc-list-item" role="menuitem" tabindex="0">
              <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">link</i>OpenM++ website
            </a>
            <a href="//www.openmpp.org/wiki/" target="_blank" class="mdc-list-item" role="menuitem" tabindex="0">
              <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">link</i>OpenM++ wiki
            </a>
            <hr class="mdc-list-divider" role="separator"></hr>
            <router-link to="/one" class="mdc-list-item" role="menuitem" tabindex="0">
              <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">settings</i>Test
            </router-link>
          </om-mcw-menu>
          
        </section>
      </div>
      
    </header>
    
    <main class="om-mcw-main">

      <router-view></router-view>

    </main>
    
  </div>
  
</div>
  
</template>

<script>
import OmMcwDrawer from './components/OmMcwDrawer'
import OmMcwMenu from './components/OmMcwMenu'
import { mapGetters, mapMutations } from 'vuex'
import { GET, SET } from './store'
import { default as mC } from './modelCommon'

export default {
  // name: 'app',
  data () {
    return {
      // msg: ''
    }
  },
  components: {
    OmMcwDrawer, OmMcwMenu
  },

  computed: {
    mainTitle () {
      let t = mC.modelTitle(this.theModel)
      return (t !== '') ? t : 'OpenM++'
    },
    ...mapGetters({
      uiLang: GET.UI_LANG,
      theModel: GET.THE_MODEL
    })
  },

  methods: {
    toggleDrawer () {
      this.$refs.drawer.toggle()
    },
    toggleMore () {
      this.$refs.more.toggle()
    },
    // more menu: view settings and language change
    moreSelected (evt) {
      let lc = evt.item.textContent || ''
      lc = lc.substring(lc, lc.indexOf('_lang_change_')).trim()
      if (lc) {
        this.setUiLang(lc)
      }
    },
    ...mapMutations({
      setUiLang: SET.UI_LANG
    })
  }
}

</script>

<!-- this component only css -->
<style lang="scss" scoped>

/* app body: drawer on left side and content */
.om-mcw-app {
  display: flex;
  flex-direction: row;
  padding: 0;
  margin: 0;
  box-sizing: border-box;
}

/* content: toolbar and main page body */
.om-mcw-content {
  display: inline-flex;
  flex-direction: column;
  flex-grow: 1;
  box-sizing: border-box;
}

/* main page body */
.om-mcw-main {
  padding-left: 1em;
}

/* toolbar */
.mdc-toolbar a {
  text-decoration: none;
}

/* fix for ie 10+ */
@media all and (-ms-high-contrast: none), (-ms-high-contrast: active) {
  button.mdc-toolbar__icon--menu {
    background-color: rgb(63, 81, 181);
    background-color: var(--mdc-theme-primary, rgb(63, 81, 181));
  }
}

</style>

<!-- import font-awesome, shared by multiple pages -->
<style lang="scss">
  @import "./font-awesome/scss/font-awesome.scss"
</style>

<!-- import MDC styles -->
<style lang="scss">
  @import "@material/button/mdc-button";
  @import "@material/dialog/mdc-dialog";
  @import "@material/drawer/mdc-drawer";
  @import "@material/elevation/mdc-elevation";
  @import "@material/list/mdc-list";
  @import "@material/menu/mdc-menu";
  @import "@material/theme/mdc-theme";
  @import "@material/toolbar/mdc-toolbar";
  @import "@material/typography/mdc-typography";
</style>

