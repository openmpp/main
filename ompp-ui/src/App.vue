<template>

<div id="app" class="app-container mdc-typography">

  <om-mcw-drawer ref="drawer">
    
    <div slot="header" class="menu-header mdc-list">
      <router-link to="/login" class="mdc-list-item" title="Login" alt="Login">
        <i class="menu-start mdc-list-item__start-detail material-icons" aria-hidden="true">account_circle</i>
        Login
      </router-link>
    </div>
    
    <nav class="mdc-list">
      <router-link to="/" class="mdc-list-item" alt="Models list" role="menuitem">
        <i class="menu-start mdc-list-item__start-detail material-icons" aria-hidden="true">home</i>
        Models
        <span v-if="modelCount > 0" class="menu-count mdc-list-item__end-detail">{{modelCount}}</span>
      </router-link>
    </nav>
    <hr class="mdc-list-divider" role="separator" />

    <nav class="mdc-list" :class="{'disable-model-menu': !isModel}">
      <router-link 
        :to="'/model/' + modelDigest + '/run-list'" 
        :class="{'disable-item': !isModel}" class="mdc-list-item" 
        alt="Model runs"
        role="menuitem">
        <i :class="{'disable-icon': !isModel}" 
          class="menu-start material-icons mdc-list-item__start-detail" aria-hidden="true">folder</i>
        Model runs
        <span v-if="isModel" class="menu-count mdc-list-item__end-detail">{{runTextCount}}</span>
      </router-link>
      <router-link 
        :to="'/model/' + modelDigest + '/parameter-list'" 
        :class="{'disable-item': !isModel}" class="mdc-list-item" 
        alt="Model parameters"
        role="menuitem">
        <i :class="{'disable-icon': !isModel}"
          class="menu-start material-icons mdc-list-item__start-detail" aria-hidden="true">input</i>
        Parameters
        <span v-if="isModel" class="menu-count mdc-list-item__end-detail">{{paramCount}}</span>
      </router-link>
      <router-link 
        :to="'/model/' + modelDigest + '/table-list'" 
        :class="{'disable-item': !isModel}" class="mdc-list-item" 
        alt="Output tables"
        role="menuitem">
        <i :class="{'disable-icon': !isModel}"
          class="menu-start material-icons mdc-list-item__start-detail" aria-hidden="true">web</i>
        Output tables
        <span v-if="isModel" class="menu-count mdc-list-item__end-detail">{{outTableCount}}</span>
      </router-link>
      <hr class="mdc-list-divider mdc-list-divider--inset" role="separator" />
      <router-link 
        :to="'/model/' + modelDigest + '/workset-list'" 
        :class="{'disable-item': !isModel}" class="mdc-list-item" 
        alt="Input sets"
        role="menuitem">
        <i :class="{'disable-icon': !isModel}"
          class="menu-start material-icons mdc-list-item__start-detail" aria-hidden="true">folder</i>
        Input sets
        <span v-if="isModel" class="menu-count mdc-list-item__end-detail">{{worksetTextCount}}</span>
      </router-link>
      <router-link 
        to="/parameters" 
        :class="{'disable-item': !isModel}" class="mdc-list-item" 
        alt="Model parameters"
        role="menuitem">
        <i :class="{'disable-icon': !isModel}"
          class="menu-start material-icons mdc-list-item__start-detail" aria-hidden="true">mode_edit</i>
        Edit parameters
        <span v-if="isModel" class="menu-count mdc-list-item__end-detail">{{paramCount}}</span>
      </router-link>
      <hr class="mdc-list-divider mdc-list-divider--inset" role="separator" />
      <router-link 
        to="/run" 
        :class="{'disable-item': !isModel}" class="mdc-list-item" 
        alt="Run the model"
        role="menuitem">
        <i :class="{'disable-icon': !isModel}"
          class="menu-start material-icons mdc-list-item__start-detail" aria-hidden="true">directions_run</i>
        Run the model
      </router-link>
    </nav>
    <hr class="mdc-list-divider" role="separator" />

    <nav class="mdc-list">
      <router-link to="/service" class="mdc-list-item" alt="Service status and model(s) run queue" role="menuitem">
        <i class="menu-start material-icons mdc-list-item__start-detail" aria-hidden="true">queue_play_next</i>Service status
      </router-link>
      <router-link to="/run-history" class="mdc-list-item" alt="Models run history" role="menuitem">
        <i class="menu-start material-icons mdc-list-item__start-detail" aria-hidden="true">history</i>Run history
      </router-link>
    </nav>
    
  </om-mcw-drawer>

  <div class="content-container">
  
    <header class="mdc-toolbar" >
      <div class="mdc-toolbar__row">
      
        <section class="mdc-toolbar__section mdc-toolbar__section--align-start">

          <button @click="toggleDrawer" class="toolbar-button material-icons mdc-toolbar__menu-icon" title="Menu" alt="Menu">menu</button>

          <router-link 
            v-if="isModel" 
            :to="'/model/' + modelDigest + '/parameter-list'" 
            :title="modelName"
            :alt="modelName"
            class="mdc-toolbar__title toolbar-title-link">
            {{ mainTitle }}
          </router-link>
          <span v-else class="mdc-toolbar__title">{{ mainTitle }}</span>
        </section>

        <section class="mdc-toolbar__section mdc-toolbar__section--align-end mdc-toolbar__section--shrink-to-fit">
          
          <button
            v-if="isModel" 
            @click="showModelNote"
            class="toolbar-button material-icons mdc-toolbar__icon"
            :alt="modelName + ' notes'"
            :title="modelName + ' notes'">event_note</button>
          <button
            @click="doRefresh"
            class="toolbar-button material-icons mdc-toolbar__icon"
            title="Refresh"
            alt="Refresh">refresh</button>
          <button
            @click="toggleMore"
            class="toolbar-button material-icons mdc-toolbar__icon"
            title="More"
            alt="More">more_vert</button>

          <om-mcw-menu ref="more" class="more-menu" @selected="moreSelected">
            <a href="#" class="mdc-list-item" alt="English" role="menuitem" tabindex="0">
              <i class="mdc-list-item__start-detail" aria-hidden="true">EN</i>
              <span hidden="true">_lang_change_</span>English
            </a>
            <a href="#" class="mdc-list-item" alt="Français" role="menuitem" tabindex="0">
              <i class="mdc-list-item__start-detail" aria-hidden="true">FR</i>
              <span hidden="true">_lang_change_</span>Français
            </a>
            <hr class="mdc-list-divider" role="separator"></hr>
            <router-link to="/settings" class="mdc-list-item" alt="Settings" role="menuitem" tabindex="0">
              <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">settings</i>Settings
            </router-link>
            <router-link to="/license" class="mdc-list-item" alt="OpenM++ licence" role="menuitem" tabindex="0">
              <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">info_outline</i>Licence
            </router-link>
            <hr class="mdc-list-divider" role="separator"></hr>
            <a href="//www.openmpp.org/" target="_blank" class="mdc-list-item" alt="OpenM++ website" role="menuitem" tabindex="0">
              <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">link</i>OpenM++ website
            </a>
            <a href="//www.openmpp.org/wiki/" target="_blank" class="mdc-list-item" alt="OpenM++ wiki" role="menuitem" tabindex="0">
              <i class="material-icons mdc-list-item__start-detail" aria-hidden="true">link</i>OpenM++ wiki
            </a>
          </om-mcw-menu>

        </section>
      </div>

    </header>
    
    <main class="main-container">
      <router-view :refresh-tickle="refreshTickle"></router-view>

      <om-mcw-dialog ref="noteDlg" id="note-dlg" acceptText="OK">
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
import { default as Mdf } from './modelCommon'
import OmMcwDrawer from './components/OmMcwDrawer'
import OmMcwMenu from './components/OmMcwMenu'
import OmMcwDialog from './components/OmMcwDialog'

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
      this.$refs.noteDlg.open()
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

  /* app body: drawer on left side and content */
  .app-container {
    display: flex;
    flex-direction: row;
    padding: 0;
    margin: 0;
    box-sizing: border-box;
  }

  /* content: toolbar and main page body */
  .content-container {
    display: inline-flex; 
    flex-direction: column;
    flex-grow: 1;
    box-sizing: border-box;
    overflow: hidden;
  }

  /* main page body */
  .main-container {
    padding-left: 1em;
  }

  /* disable model menu and links if no current model selected */
  .disable-model-menu {
    @extend .mdc-theme--text-disabled-on-background;
  }
  a.disable-item, div.disable-item {
    pointer-events: none;
    cursor: default;
  }
  .disable-icon {
    @extend .mdc-theme--text-disabled-on-background;
  }

  /* adjust model menu margins */
  .menu-header {
    width: 100%;
  }
  .menu-start {
    margin-right: .5em;
  }
  .menu-count {
    width: 3em;
    text-align: right;
    padding-right: 1px;
    @extend .mdc-theme--text-primary-on-primary;
    @extend .mdc-theme--primary-bg;
  }

  /* main title: model a link */
  .toolbar-title-link {
    @extend .mdc-theme--text-primary-on-primary;
  }

  /* toolbar links */
  .mdc-toolbar a {
    text-decoration: none;
  }

  /* toolbar icon buttons */
  .toolbar-button {
    outline: none;
    &:hover {
      cursor: pointer;
    }
    @extend .mdc-theme--primary-bg;
  }
  .toolbar-pad-button {
    padding-right: 0;
    &:last-of-type {
      padding-right: 12px;
    }
  }

  /* fix: getInnerDimensions returns 0,0 randomly */
  /* more menu at the right top instead of mdc-anchor */
  .more-menu {
    transform-origin: right top 0px;
    right: 0px; 
    top: 0px;
  }
</style>

<!-- import refresh spin animation shared style -->
<style lang="scss">
  @import "./refresh-spin.scss";
  
  /* monospace font */
  .mono {
    font-family: Roboto Mono, Courier New, Courier, monospace;
  }
</style>

<!-- import MDC styles -->
<style lang="scss">
  @import "@material/button/mdc-button";
  @import "@material/dialog/mdc-dialog";
  @import "@material/drawer/mdc-drawer";
  @import "@material/elevation/mdc-elevation";
  @import "@material/list/mdc-list";
  @import "@material/menu/mdc-menu";
  @import "@material/toolbar/mdc-toolbar";
  @import "@material/theme/mdc-theme";
  @import "@material/typography/mdc-typography";
</style>

