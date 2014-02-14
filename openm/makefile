BUILD_DIR = ./build
OUT_PREFIX = .

LIBSQLITE_DIR = libsqlite
LIBOPENM_DIR = libopenm
OMC_DIR = omc
MODEL_ONE_DIR = modelOne
MODEL_WCB_DIR = wizardCaseBased
MODEL_WTB_DIR = WizardTimeBased
MODEL_A1_DIR = Alpha1
MODEL_A2_DIR = Alpha2

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
all: prepare libsqlite libopenm omc
# all: prepare libsqlite libopenm model_one omc model_wcb model_wtb model_alpha1 model_alpha2

libsqlite: prepare
	$(MAKE) -C $(LIBSQLITE_DIR) 

libopenm: prepare libsqlite
	$(MAKE) -C $(LIBOPENM_DIR) libopenm

lib_omc: prepare
	$(MAKE) -C $(LIBOPENM_DIR) lib_omc

omc: lib_omc
	$(MAKE) -C $(OMC_DIR) 

model_one: libopenm
	$(MAKE) -C $(MODEL_ONE_DIR) model

model_wcb: libopenm omc
	$(MAKE) -C $(MODEL_WCB_DIR) model
    
model_wtb: libopenm omc
	$(MAKE) -C $(MODEL_WTB_DIR) model
    
model_alpha1: libopenm omc
	$(MAKE) -C $(MODEL_A1_DIR) model
    
model_alpha2: libopenm omc
	$(MAKE) -C $(MODEL_A2_DIR) model
    
.PHONY : prepare
prepare:
	@if [ ! -d $(BUILD_DIR) ] ; then mkdir -p $(BUILD_DIR) ; fi
	@if [ ! -d $(OUT_PREFIX) ] ; then mkdir -p $(OUT_PREFIX) ; fi

.PHONY : clean
clean:
	$(MAKE) -C $(MODEL_A2_DIR) clean
	$(MAKE) -C $(MODEL_A1_DIR) clean
	$(MAKE) -C $(MODEL_WTB_DIR) clean
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

