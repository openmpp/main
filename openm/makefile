BUILD_DIR = ./build
OUT_PREFIX = .
LIBSQLITE_DIR = libsqlite
LIBSQLITE_DEPS_DIR = $(BUILD_DIR)/libsqlite/deps

LIBOPENM_DIR = libopenm
LIBOPENM_DEPS_DIR = $(BUILD_DIR)/libopenm/deps

OMC_DIR = omc
OMC_DEPS_DIR = $(BUILD_DIR)/omc/deps
OMC_SRC_DIR = $(BUILD_DIR)/omc/src

MODEL_ONE_DIR = modelOne
MODEL_ONE_DEPS_DIR = $(BUILD_DIR)/modelOne/deps

MODEL_WCB_DIR = wizardCaseBased
MODEL_WCB_DEPS_DIR = $(BUILD_DIR)/wizardCaseBased/deps
MODEL_WCB_OMC_OUT_DIR = $(BUILD_DIR)/wizardCaseBased/src

MODEL_A1_DIR = Alpha1
MODEL_A1_DEPS_DIR = $(BUILD_DIR)/Alpha1/deps
MODEL_A1_OMC_OUT_DIR = $(BUILD_DIR)/Alpha1/src

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
all: prepare libsqlite libopenm model_one omc model_wcb model_alpha1

libsqlite: prepare_libsqlite
	$(MAKE) -C $(LIBSQLITE_DIR) 

libopenm: libsqlite prepare_libopenm
	$(MAKE) -C $(LIBOPENM_DIR) libopenm

lib_omc: prepare_libopenm
	$(MAKE) -C $(LIBOPENM_DIR) lib_omc

omc: lib_omc prepare_omc
	$(MAKE) -C $(OMC_DIR) 

model_one: libopenm prepare_one
	$(MAKE) -C $(MODEL_ONE_DIR) model

model_wcb: libopenm omc prepare_wcb
	$(MAKE) -C $(MODEL_WCB_DIR) model
    
model_alpha1: libopenm omc prepare_alpha1
	$(MAKE) -C $(MODEL_A1_DIR) model
    
.PHONY : prepare
prepare: prepare_dirs prepare_libsqlite prepare_libopenm prepare_omc prepare_one prepare_wcb

.PHONY : prepare_dirs
prepare_dirs:
	@if [ ! -d $(BUILD_DIR) ] ; then mkdir -p $(BUILD_DIR) ; fi
	@if [ ! -d $(OUT_PREFIX) ] ; then mkdir -p $(OUT_PREFIX) ; fi

.PHONY : prepare_libsqlite
prepare_libsqlite:
	@if [ ! -d $(LIBSQLITE_DEPS_DIR) ] ; then mkdir -p $(LIBSQLITE_DEPS_DIR) ; fi

.PHONY : prepare_libopenm
prepare_libopenm:
	@if [ ! -d $(LIBOPENM_DEPS_DIR) ] ; then mkdir -p $(LIBOPENM_DEPS_DIR) ; fi

.PHONY : prepare_omc
prepare_omc:
	@if [ ! -d $(OMC_DEPS_DIR) ] ; then mkdir -p $(OMC_DEPS_DIR) ; fi
	@if [ ! -d $(OMC_SRC_DIR) ] ; then mkdir -p $(OMC_SRC_DIR) ; fi

.PHONY : prepare_one
prepare_one:
	@if [ ! -d $(MODEL_ONE_DEPS_DIR) ] ; then mkdir -p $(MODEL_ONE_DEPS_DIR) ; fi

.PHONY : prepare_wcb
prepare_wcb:
	@if [ ! -d $(MODEL_WCB_DEPS_DIR) ] ; then mkdir -p $(MODEL_WCB_DEPS_DIR) ; fi
	@if [ ! -d $(MODEL_WCB_OMC_OUT_DIR) ] ; then mkdir -p $(MODEL_WCB_OMC_OUT_DIR) ; fi

.PHONY : prepare_alpha1
prepare_alpha1:
	@if [ ! -d $(MODEL_A1_DEPS_DIR) ] ; then mkdir -p $(MODEL_A1_DEPS_DIR) ; fi
	@if [ ! -d $(MODEL_A1_OMC_OUT_DIR) ] ; then mkdir -p $(MODEL_A1_OMC_OUT_DIR) ; fi

.PHONY : clean
clean:
	$(MAKE) -C $(MODEL_A1_DIR) clean
	$(MAKE) -C $(MODEL_WCB_DIR) clean
	$(MAKE) -C $(MODEL_ONE_DIR) clean
	$(MAKE) -C $(OMC_DIR) clean
	$(MAKE) -C $(LIBOPENM_DIR) clean
	$(MAKE) -C $(LIBSQLITE_DIR) clean

.PHONY : cleanall
cleanall:
	rm -rf $(OUT_PREFIX)/bin
	rm -rf $(OUT_PREFIX)/lib
	rm -rf $(BUILD_DIR)

