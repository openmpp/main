ifndef BUILD_DIR
  BUILD_DIR = ./build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = .
endif

ifndef OM_INC_DIR
  OM_INC_DIR = ../include
endif

ifndef OM_BIN_DIR
  OM_BIN_DIR = ../bin
endif

ifndef OM_LIB_DIR
  OM_LIB_DIR = ../lib
endif

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
all: prepare model_one model_wcb model_wtb model_alpha1 model_alpha2

model_one: prepare
	$(MAKE) -C $(MODEL_ONE_DIR) model

model_wcb: prepare
	$(MAKE) -C $(MODEL_WCB_DIR) model
    
model_wtb: prepare
	$(MAKE) -C $(MODEL_WTB_DIR) model
    
model_alpha1: prepare
	$(MAKE) -C $(MODEL_A1_DIR) model
    
model_alpha2: prepare
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

.PHONY : cleanall
cleanall:
	rm -rf $(OUT_PREFIX)/bin
	rm -rf $(BUILD_DIR)
