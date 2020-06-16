ifndef OUT_PREFIX
  OUT_PREFIX = ..
endif
BIN_DIR   = $(OUT_PREFIX)/bin
BUILD_DIR = $(OUT_PREFIX)/build

LOCAL_OUT_PREFIX = $(dir $(OUT_PREFIX))
LOCAL_BIN_DIR    = $(LOCAL_OUT_PREFIX)bin
LOCAL_BUILD_DIR  = $(LOCAL_OUT_PREFIX)build

ifndef OMC_CODE_PAGE
  OMC_CODE_PAGE = WINDOWS-1252
endif

OMC_NO_LINE_DEF=
ifdef OMC_NO_LINE
  OMC_NO_LINE_DEF = "OMC_NO_LINE=$(OMC_NO_LINE)"
endif

#
# models to build
#
ifndef MODEL_DIRS
  MODEL_DIRS = modelOne IDMM NewCaseBased NewTimeBased NewCaseBased_bilingual NewTimeBased_bilingual RiskPaths OzProj OzProjGen 
endif

# set OM_MSG_USE:
# MPI   - use MPI-based version (you must have MPI installed)
# EMPTY - use empty version of the library which does nothing
#
# OM_MSG_USE = MPI
# OM_MSG_USE = EMPTY

ifndef OM_MSG_USE
  OM_MSG_USE = EMPTY
endif

ifndef RELEASE
  OMC_SRC_DIR = debug/src
else
  OMC_SRC_DIR = release/src
endif
LOCAL_SQL_DIR = $(LOCAL_OUT_PREFIX)sql

#
# targets and rules
#
.PHONY : all
.PHONY : $(MODEL_DIRS)

all : prepare $(MODEL_DIRS)

$(MODEL_DIRS) :
	$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(BIN_DIR) OMC_CODE_PAGE=$(OMC_CODE_PAGE) $(OMC_NO_LINE_DEF) -C $@ || { exit $$?; }

.PHONY : publish
publish : prepare
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(BIN_DIR) -C $(d) publish || { exit $$?; };)
	cp -pf modelOne/*.sql $(LOCAL_SQL_DIR)
	$(foreach d,$(filter-out modelOne,$(MODEL_DIRS)),cp -pf $(LOCAL_BUILD_DIR)/$(d)/$(OMC_SRC_DIR)/*.sql $(LOCAL_SQL_DIR);)

.PHONY : publish-views
publish-views : publish
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(BIN_DIR) -C $(d) publish-views || { exit $$?; };)

.PHONY : run
run : prepare
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(BIN_DIR) -C $(d) run || { exit $$?; };)

.PHONY : prepare
prepare:
	@if [ ! -d $(LOCAL_BIN_DIR) ] ; then mkdir -p $(LOCAL_BIN_DIR) ; fi
	@if [ ! -d $(LOCAL_BUILD_DIR) ] ; then mkdir -p $(LOCAL_BUILD_DIR) ; fi
	@if [ ! -d $(LOCAL_SQL_DIR) ] ; then mkdir -p $(LOCAL_SQL_DIR) ; fi

.PHONY : clean
clean:
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(BIN_DIR) -C $(d) clean;)

.PHONY : cleanall
cleanall:
	rm -rf $(LOCAL_BIN_DIR)
	rm -rf $(LOCAL_BUILD_DIR)
	@if [ -d $(LOCAL_OUT_PREFIX)sql ] ;  then rm -rf $(LOCAL_OUT_PREFIX)sql ; fi
