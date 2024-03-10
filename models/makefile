# platform name: Linux or Darwin
local_PLATFORM_UNAME := $(shell uname -s)

local_BIN_DIR = bin
local_SQL_DIR = sql
local_LOG_DIR = log
local_DOC_DIR = doc

ifdef OUT_PREFIX
  model_OUT_PREFIX = OUT_PREFIX
else
  model_OUT_PREFIX = ompp-linux
  ifeq ($(local_PLATFORM_UNAME), Darwin)
    model_OUT_PREFIX = ompp-mac
  endif
endif

ifndef RELEASE
  model_BUILD_SRC_DIR = $(model_OUT_PREFIX)/build/debug/src
else
  model_BUILD_SRC_DIR = $(model_OUT_PREFIX)/build/release/src
endif

model_BIN_DIR = $(model_OUT_PREFIX)/bin
model_DOC_DIR = $(model_OUT_PREFIX)/bin/doc

#
# models to build
#
ifndef MODEL_DIRS
  MODEL_DIRS = modelOne IDMM NewCaseBased NewTimeBased NewCaseBased_bilingual RiskPaths OzProjGenX OzProjX Alpha2
endif

#
# convert source files to utf-8 from Windows code page
#
ifndef OMC_CODE_PAGE
  OMC_CODE_PAGE = WINDOWS-1252
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

#
# targets and rules
#
.PHONY : all
.PHONY : $(MODEL_DIRS)

all : prepare $(MODEL_DIRS)

$(MODEL_DIRS) :
	$(MAKE) OMC_CODE_PAGE=$(OMC_CODE_PAGE) -C $@ || { exit $$?; }

.PHONY : install
install : all publish-all

.PHONY : publish
publish :
	$(foreach d,$(MODEL_DIRS),$(MAKE) -C $(d) publish || { exit $$?; };)

.PHONY : run
run :
	$(foreach d,$(MODEL_DIRS),$(MAKE) -C $(d) run || { exit $$?; };)

# OzProjX models reading microdata from ../../../OzProjGenX/microdata/OzProj22_5K.csv
#
OZ_PROJ_MODELS = OzProjGenX OzProjX

.PHONY : publish-all
publish-all : prepare publish
	$(foreach d,$(MODEL_DIRS),find $(d)/$(model_BIN_DIR) -name "*.log" -exec mv {} $(local_LOG_DIR) \;;)
	$(foreach d,$(filter-out $(OZ_PROJ_MODELS),$(MODEL_DIRS)),cp -pf $(d)/$(model_BIN_DIR)/$(d)* $(local_BIN_DIR);)
	$(foreach d,$(filter $(OZ_PROJ_MODELS),$(MODEL_DIRS)),mkdir -p $(local_BIN_DIR)/$d/ompp/bin;)
	$(foreach d,$(filter $(OZ_PROJ_MODELS),$(MODEL_DIRS)),cp -pf $(d)/$(model_BIN_DIR)/$(d)* $(local_BIN_DIR)/$d/ompp/bin/;)
	$(foreach d,$(filter $(OZ_PROJ_MODELS),$(MODEL_DIRS)),cp -prf $(d)/microdata $(local_BIN_DIR)/$d/;)
	$(foreach d,$(filter modelOne,$(MODEL_DIRS)),cp -pf modelOne/*.sql $(local_SQL_DIR);)
	$(foreach d,$(filter-out modelOne,$(MODEL_DIRS)),cp -pf $(d)/$(model_BUILD_SRC_DIR)/*.sql $(local_SQL_DIR);)
ifndef MODEL_DOC_DISABLE
	$(foreach d,$(filter-out modelOne,$(MODEL_DIRS)),cp -pf $(d)/$(model_DOC_DIR)/*.html $(local_DOC_DIR);)
	$(foreach d,$(filter-out modelOne,$(MODEL_DIRS)),if [ -d $(d)/doc ]; then find $(d)/doc -iname "*.pdf" -exec cp -pf {} $(local_DOC_DIR) \;; fi;)
endif

.PHONY : prepare
prepare:
	@if [ ! -d $(local_BIN_DIR) ] ; then mkdir -p $(local_BIN_DIR) ; fi
	@if [ ! -d $(local_SQL_DIR) ] ; then mkdir -p $(local_SQL_DIR) ; fi
	@if [ ! -d $(local_LOG_DIR) ] ; then mkdir -p $(local_LOG_DIR) ; fi

.PHONY : clean-all
clean-all: clean
	$(foreach d,$(MODEL_DIRS),$(MAKE) -C $(d) clean-all;)

.PHONY : clean
clean:
	rm -rf $(local_BIN_DIR)
	@if [ -d $(local_SQL_DIR) ] ;  then rm -rf $(local_SQL_DIR)   ; fi
	@if [ -d $(local_LOG_DIR) ] ;  then rm -rf $(local_LOG_DIR)/* ; fi
	@if [ -d $(local_DOC_DIR) ] ;  then rm -rf $(local_DOC_DIR)/* ; fi
