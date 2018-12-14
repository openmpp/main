<template>

<div id="app" class="mdc-typography">

  <header class="topbar-container mdc-top-app-bar mdc-top-app-bar--fixed" >
    <div class="mdc-top-app-bar__row">

      <section class="mdc-top-app-bar__section mdc-top-app-bar__section--align-start">

        <button
          @click="toggleDrawer"
          class="material-icons mdc-top-app-bar__navigation-icon" title="Menu" alt="Menu">menu</button>

        <router-link
          v-if="isModel"
          :to="'/model/' + modelDigest"
          :title="modelName"
          :alt="modelName"
          class="mdc-top-app-bar__title topbar-title-link">
          {{ mainTitle }}
        </router-link>
        <span v-else class="mdc-top-app-bar__title">{{ mainTitle }}</span>

      </section>

      <section class="topbar-last-section mdc-top-app-bar__section mdc-top-app-bar__section--align-end mdc-menu-surface--anchor">

        <button
          v-if="isModel"
          @click="showModelNote"
          class="material-icons mdc-top-app-bar__action-item"
          :alt="modelName + ' notes'"
          :title="modelName + ' notes'">event_note</button>
        <button
          @click="doRefresh"
          class="material-icons mdc-top-app-bar__action-item"
          title="Refresh"
          alt="Refresh">refresh</button>
        <button
          @click="toggleMore"
          class="material-icons mdc-top-app-bar__action-item"
          title="More"
          alt="More">more_vert</button>

        <om-mcw-menu ref="more" @selected="moreSelected">
          <a href="#" class="mdc-list-item" alt="English" role="menuitem" tabindex="0">
            <span class="mdc-list-item__text">EN<span hidden="true">_lang_change_</span>&nbsp;English</span>
          </a>
          <a href="#" class="mdc-list-item" alt="Français" role="menuitem" tabindex="0">
            <span class="mdc-list-item__text">FR<span hidden="true">_lang_change_</span>&nbsp;Français</span>
          </a>
          <hr class="mdc-list-divider" role="separator" />
          <router-link to="/settings" class="mdc-list-item" alt="Settings" role="menuitem" tabindex="0">
            <i class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">settings</i>
            <span class="mdc-list-item__text">Settings</span>
          </router-link>
          <router-link to="/login" class="mdc-list-item" title="Login" alt="Login" role="menuitem">
            <i class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">account_circle</i>
            <span class="mdc-list-item__text">Login</span>
          </router-link>
          <router-link to="/license" class="mdc-list-item" alt="OpenM++ licence" role="menuitem" tabindex="0">
            <i class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">info_outline</i>
            <span class="mdc-list-item__text">Licence</span>
          </router-link>
          <hr class="mdc-list-divider" role="separator" />
          <a href="//www.openmpp.org/" target="_blank" class="mdc-list-item" alt="OpenM++ website" role="menuitem" tabindex="0">
            <i class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">link</i>
            <span class="mdc-list-item__text">OpenM++ website</span>
          </a>
          <a href="//www.openmpp.org/wiki/" target="_blank" class="mdc-list-item" alt="OpenM++ wiki" role="menuitem" tabindex="0">
            <i class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">link</i>
            <span class="mdc-list-item__text">OpenM++ wiki</span>
          </a>
        </om-mcw-menu>

      </section>
    </div>

  </header>

  <om-mcw-drawer ref="drawer" class="mdc-drawer--dismissible mdc-top-app-bar--fixed-adjust">
    <div class="mdc-list-group" role="menu">

      <nav class="mdc-list">
        <router-link to="/" class="mdc-list-item" alt="Models list" role="menuitem">
          <i class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">home</i>
          <span class="mdc-list-item__text">Models</span>
          <span v-if="modelCount > 0" class="menu-count mdc-list-item__meta">{{modelCount}}</span>
        </router-link>
      </nav>
      <hr class="mdc-list-divider" role="separator" />

      <nav class="mdc-list" :class="{'disable-model-menu': !isModel}">
        <router-link
          :to="'/model/' + modelDigest + '/run-list'"
          :class="{'disable-item': !isModel}" class="mdc-list-item"
          alt="Model runs"
          role="menuitem">
          <i :class="{'disable-icon': !isModel}" class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">folder</i>
          <span class="mdc-list-item__text">Model runs</span>
          <span v-if="isModel" class="menu-count mdc-list-item__meta">{{runTextCount}}</span>
        </router-link>
        <router-link
          :to="'/model/' + modelDigest + '/run/parameter-list'"
          :class="{'disable-item': !isModel}" class="mdc-list-item"
          alt="Model parameters"
          role="menuitem">
          <i :class="{'disable-icon': !isModel}" class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">input</i>
          <span class="mdc-list-item__text">Parameters</span>
          <span v-if="isModel" class="menu-count mdc-list-item__meta">{{paramCount}}</span>
        </router-link>
        <router-link
          :to="'/model/' + modelDigest + '/run/table-list'"
          :class="{'disable-item': !isModel}" class="mdc-list-item"
          alt="Output tables"
          role="menuitem">
          <i :class="{'disable-icon': !isModel}" class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">web</i>
          <span class="mdc-list-item__text">Output tables</span>
          <span v-if="isModel" class="menu-count mdc-list-item__meta">{{outTableCount}}</span>
        </router-link>
        <hr class="mdc-list-divider menu-divider-inset" role="separator" />
        <router-link
          :to="'/model/' + modelDigest + '/workset-list'"
          :class="{'disable-item': !isModel}" class="mdc-list-item"
          alt="Input sets"
          role="menuitem">
          <i :class="{'disable-icon': !isModel}" class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">folder</i>
          <span class="mdc-list-item__text">Input sets</span>
          <span v-if="isModel" class="menu-count mdc-list-item__meta">{{worksetTextCount}}</span>
        </router-link>
        <router-link
          :to="'/model/' + modelDigest + '/set/parameter-list'"
          :class="{'disable-item': !isModel}" class="mdc-list-item"
          alt="Model parameters"
          role="menuitem">
          <i :class="{'disable-icon': !isModel}" class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">mode_edit</i>
          <span class="mdc-list-item__text">Edit parameters</span>
          <span v-if="isModel" class="menu-count mdc-list-item__meta">{{paramCount}}</span>
        </router-link>
        <hr class="mdc-list-divider menu-divider-inset" role="separator" />
        <router-link
          to="/run"
          :class="{'disable-item': !isModel}" class="mdc-list-item"
          alt="Run the model"
          role="menuitem">
          <i :class="{'disable-icon': !isModel}" class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">directions_run</i>
          <span class="mdc-list-item__text">Run the model</span>
        </router-link>
      </nav>
      <hr class="mdc-list-divider" role="separator" />

      <nav class="mdc-list">
        <router-link to="/service" class="mdc-list-item" alt="Service status and model(s) run queue" role="menuitem">
          <i class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">queue_play_next</i>
          <span class="mdc-list-item__text">Service status</span>
        </router-link>
        <router-link to="/run-history" class="mdc-list-item" alt="Models run history" role="menuitem">
          <i class="menu-start mdc-list-item__graphic material-icons" aria-hidden="true">history</i>
          <span class="mdc-list-item__text">Run history</span>
        </router-link>
      </nav>

    </div>
  </om-mcw-drawer>

  <div class="app-content mdc-drawer-app-content mdc-top-app-bar--fixed-adjust">
    <main class="main-content">

      <router-view :refresh-tickle="refreshTickle"></router-view>

      <om-mcw-dialog ref="theModelInfoDlg" id="the-model-info-dlg" :scrollable="true" acceptText="OK">
        <span slot="header">{{titleNoteDlg}}</span>
        <div>{{textNoteDlg}}</div>
        <br/>
        <div class="mono">Name:&nbsp;&nbsp;&nbsp;&nbsp;{{nameNoteDlg}}</div>
        <div class="mono">Created:&nbsp;{{createdNoteDlg}}</div>
        <div class="mono">Digest:&nbsp;&nbsp;{{digestNoteDlg}}</div>
      </om-mcw-dialog>

    </main>
  </div>

</div>

</template>

<script>
import { mapGetters, mapActions } from 'vuex'
import { GET, SET } from './store'
import * as Mdf from './modelCommon'
import OmMcwDrawer from '@/om-mcw/OmMcwDrawer'
import OmMcwMenu from '@/om-mcw/OmMcwMenu'
import OmMcwDialog from '@/om-mcw/OmMcwDialog'

export default {
  name: 'app',
  data () {
    return {
      refreshTickle: false,
      titleNoteDlg: '',
      textNoteDlg: '',
      nameNoteDlg: '',
      createdNoteDlg: '',
      digestNoteDlg: ''
    }
  },

  computed: {
    mainTitle () {
      let t = Mdf.modelTitle(this.theModel)
      return (t !== '') ? t : 'OpenM++'
    },
    isModel () { return Mdf.isModel(this.theModel) },
    modelName () { return Mdf.modelName(this.theModel) },
    modelDigest () { return Mdf.modelDigest(this.theModel) },
    isModelNote () { return Mdf.isNoteOfDescrNote(this.theModel) },
    modelCount () { return this.modelListCount },
    paramCount () { return Mdf.paramCount(this.theModel) },
    outTableCount () { return Mdf.outTableCount(this.theModel) },
    runTextCount () { return Mdf.runTextCount(this.runTextList) },
    worksetTextCount () { return Mdf.worksetTextCount(this.worksetTextList) },

    ...mapGetters({
      theModel: GET.THE_MODEL,
      modelListCount: GET.MODEL_LIST_COUNT,
      runTextList: GET.RUN_TEXT_LIST,
      worksetTextList: GET.WORKSET_TEXT_LIST
    })
  },

  methods: {
    toggleDrawer () {
      this.$refs.drawer.toggle()
    },
    toggleMore () {
      this.$refs.more.toggle()
    },
    doRefresh () {
      this.refreshTickle = !this.refreshTickle
    },

    // more menu: view and update settings, change language, refresh data
    moreSelected (evt) {
      // if this is language change
      let txt = evt.item.textContent || ''
      let lc = txt.substring(txt, txt.indexOf('_lang_change_')).trim()
      if (lc) {
        this.setUiLang(lc)
        return
      }
      // if this is action button click
      let bt = txt.substring(txt, txt.indexOf('_action_button_')).trim()
      if (bt) {
        if (bt === 'refresh') {
          this.doRefresh()
        } else {
          console.log('moreSelected: unknown action button', bt)
        }
      }
    },

    // show model notes
    showModelNote () {
      const md = this.theModel
      this.titleNoteDlg = Mdf.descrOfDescrNote(md)
      this.textNoteDlg = Mdf.noteOfDescrNote(md)
      this.nameNoteDlg = Mdf.modelName(md)
      this.createdNoteDlg = Mdf.dtStr(md.Model.CreateDateTime)
      this.digestNoteDlg = Mdf.modelDigest(md)
      this.$refs.theModelInfoDlg.open()
    },

    ...mapActions({
      setUiLang: SET.UI_LANG
    })
  },

  components: { OmMcwDrawer, OmMcwMenu, OmMcwDialog }
}
</script>

<!-- local scope css: this component only -->
<style lang="scss" scoped>
  @import "@material/theme/mdc-theme";
  @import "@material/list/mdc-list";

  /* app body: drawer on left side and content */
  #app {
    height: 100%;
    width: 100%;
    display: flex;
    padding: 0;
    margin: 0;
    box-sizing: border-box;
    overflow-y: hidden;
  }

  /* content: topbar and main page body */
  .app-content {
    flex: auto;
    overflow: hidden;
    position: relative;
  }
  .main-content {
    overflow: auto;
    height: 100%;
    padding-left: 1rem;
  }
  .topbar-container {
    flex: 0 0 auto;
    z-index: 7;
  }

  /* disable drawer model menu and links if no current model selected */
  .disable-model-menu {
    @extend .mdc-theme--text-disabled-on-background;
  }
  a.disable-item, div.disable-item {
    pointer-events: none;
    cursor: default;
    @extend .mdc-list-item--disabled;
  }
  .disable-icon {
    @extend .mdc-theme--text-disabled-on-background;
  }

  /* adjust drawer model menu margins */
  .menu-start {
    margin-right: .5rem;
  }
  .menu-count {
    width: 3rem;
    text-align: right;
    padding-right: 1px;
    @extend .mdc-theme--on-primary;
    @extend .mdc-theme--primary-bg;
  }
  hr.menu-divider-inset {
    margin-left: 3rem;
    /* @extend .mdc-list-divider--inset; */
  }

  /* main title: model a link */
  .topbar-title-link {
    @extend .mdc-theme--on-primary;
  }

  /* topbar links */
  .mdc-top-app-bar a {
    text-decoration: none;
  }

  /* topbar last section: shrink to fit */
  .topbar-last-section {
    flex: none;
  }
</style>

<!-- import refresh spin animation shared style -->
<style lang="scss">
  @import "./refresh-spin.scss";
  @import "./om-mcw.scss";
</style>

<!-- import MDC styles -->
<style lang="scss">
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
  @import "@material/button/mdc-button";
  @import "@material/dialog/mdc-dialog";
  @import "@material/top-app-bar/mdc-top-app-bar";
  @import "@material/drawer/mdc-drawer";
  @import "@material/list/mdc-list";
  @import "@material/menu/mdc-menu";
</style>
