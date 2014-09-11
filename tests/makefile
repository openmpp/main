ifndef BUILD_DIR
  BUILD_DIR = ./build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = .
endif

MODEL_DIRS = \
  modelOne \
  WizardCaseBased \
  WizardTimeBased \
  Alpha1 \
  Alpha2 \
  IDMM \
  RiskPaths

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
	$(MAKE) -C $@
	$(MAKE) -C $@ publish

.PHONY : prepare
prepare:
	@if [ ! -d $(BUILD_DIR) ] ; then mkdir -p $(BUILD_DIR) ; fi
	@if [ ! -d $(OUT_PREFIX) ] ; then mkdir -p $(OUT_PREFIX) ; fi

.PHONY : clean
clean:
	$(foreach d,$(MODEL_DIRS),$(MAKE) -C $(d) clean;)

.PHONY : cleanall
cleanall:
	rm -rf $(OUT_PREFIX)/bin
	rm -rf $(BUILD_DIR)
