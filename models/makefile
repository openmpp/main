ifndef OUT_PREFIX
  OUT_PREFIX = ..
endif
BIN_DIR   = $(OUT_PREFIX)/bin
BUILD_DIR = $(OUT_PREFIX)/build

LOCAL_OUT_PREFIX = $(dir $(OUT_PREFIX))
LOCAL_BIN_DIR    = $(LOCAL_OUT_PREFIX)bin
LOCAL_BUILD_DIR  = $(LOCAL_OUT_PREFIX)build

#
# models to build
#
MODEL_DIRS = \
  modelOne \
  IDMM \
  NewCaseBased \
  NewTimeBased \
  RiskPaths \
  OzProj \
  OzProjGen \
  Alpha1 
#  Alpha2 \
#  GiniProbe \
#  IDMM_extra \
#  TableProbe1 \
#  TestEx \
#  THIM \
#  TypeProbe1 \

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
LOCAL_SQL_DIR = $(LOCAL_OUT_PREFIX)sql$(BIN_POSTFIX)

#
# targets and rules
#
.PHONY : all
.PHONY : $(MODEL_DIRS)

all : prepare $(MODEL_DIRS)

$(MODEL_DIRS) :
	$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(BIN_DIR) -C $@

.PHONY : publish
publish : prepare
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(BIN_DIR) -C $(d) publish;)
	cp -pf modelOne/*_sqlite.sql $(LOCAL_SQL_DIR)
	@if [ -e modelOne/modelOne.ini ] ; then cp -pf modelOne/modelOne.ini $(LOCAL_BIN_DIR) ; fi
	$(foreach d,$(filter-out modelOne,$(MODEL_DIRS)),cp -pf $(LOCAL_BUILD_DIR)/$(d)/$(OMC_SRC_DIR)/*.sql $(LOCAL_SQL_DIR);)

.PHONY : run
run : prepare
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(BIN_DIR) -C $(d) run;)
	
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
	@if [ -d $(LOCAL_OUT_PREFIX)sqlD ] ; then rm -rf $(LOCAL_OUT_PREFIX)sqlD ; fi
	@if [ -d $(LOCAL_OUT_PREFIX)sql$(BIN_POSTFIX) ] ; then rm -rf $(LOCAL_OUT_PREFIX)sql$(BIN_POSTFIX) ; fi
