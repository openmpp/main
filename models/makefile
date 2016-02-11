ifndef BUILD_DIR
  BUILD_DIR = ../build
  LOCAL_BUILD_DIR = ./build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ..
  LOCAL_OUT_DIR = ./bin
endif

ifndef PUBLISH_DIR
  PUBLISH_DIR = $(OUT_PREFIX)/bin
endif

MODEL_DIRS = \
  modelOne \
  Alpha1 \
  IDMM \
  RiskPaths \
  OzProj \
  OzProjGen \
  WizardCaseBased \
  WizardTimeBased
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

.PHONY : all
.PHONY : $(MODEL_DIRS)

all : prepare $(MODEL_DIRS)

$(MODEL_DIRS) :
	$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(PUBLISH_DIR) -C $@

.PHONY : publish
publish :
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(PUBLISH_DIR) -C $(d) publish;)
	$(foreach d,$(filter-out modelOne,$(MODEL_DIRS)),cp -pf $(LOCAL_BUILD_DIR)/$(d)/src/*.sql $(LOCAL_OUT_DIR);)

.PHONY : run
run :
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(PUBLISH_DIR) -C $(d) run;)
	
.PHONY : prepare
prepare:
	@if [ ! -d $(LOCAL_BUILD_DIR) ] ; then mkdir -p $(LOCAL_BUILD_DIR) ; fi
	@if [ ! -d $(LOCAL_OUT_DIR) ] ; then mkdir -p $(LOCAL_OUT_DIR) ; fi

.PHONY : clean
clean:
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) PUBLISH_DIR=$(PUBLISH_DIR) -C $(d) clean;)

.PHONY : cleanall
cleanall:
	rm -rf $(LOCAL_OUT_DIR)
	rm -rf $(LOCAL_BUILD_DIR)
