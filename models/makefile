ifndef BUILD_DIR
  BUILD_DIR = ../build
  ALL_BUILD_DIR = ./build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ../bin
  ALL_OUT_DIR = ./bin
endif

MODEL_DIRS = \
  modelOne \
  Alpha1 \
  Alpha2 \
  GiniProbe \
  IDMM \
  IDMM_extra \
  OzProj \
  OzProjGen \
  RiskPaths \
  TableProbe1 \
  TestEx \
  THIM \
  TypeProbe1 \
  WizardCaseBased \
  WizardTimeBased

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
	$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) -C $@
	$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) -C $@ publish

.PHONY : prepare
prepare:
	@if [ ! -d $(ALL_BUILD_DIR) ] ; then mkdir -p $(ALL_BUILD_DIR) ; fi
	@if [ ! -d $(ALL_OUT_DIR) ] ; then mkdir -p $(ALL_OUT_DIR) ; fi

.PHONY : clean
clean:
	$(foreach d,$(MODEL_DIRS),$(MAKE) BUILD_DIR=$(BUILD_DIR) OUT_PREFIX=$(OUT_PREFIX) -C $(d) clean;)

.PHONY : cleanall
cleanall:
	rm -rf $(ALL_OUT_DIR)
	rm -rf $(ALL_BUILD_DIR)
