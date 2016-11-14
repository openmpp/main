ifndef OUT_PREFIX
  OUT_PREFIX = ..
endif
OUT_LIB_DIR = $(OUT_PREFIX)/lib
OUT_BIN_DIR = $(OUT_PREFIX)/bin
BUILD_DIR   = $(OUT_PREFIX)/build

LIBSQLITE_DIR = libsqlite
LIBOPENM_DIR = libopenm
OMC_DIR = omc

ifndef BIN_POSTFIX
  BIN_POSTFIX = 
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
all: prepare libsqlite libopenm omc

libsqlite: prepare
	$(MAKE) -C $(LIBSQLITE_DIR) 

libopenm: prepare libsqlite
	$(MAKE) -C $(LIBOPENM_DIR) libopenm

lib_omc: prepare libsqlite
	$(MAKE) -C $(LIBOPENM_DIR) lib_omc

omc: lib_omc
	$(MAKE) -C $(OMC_DIR) 

.PHONY : prepare
prepare:
	@if [ ! -d $(BUILD_DIR) ] ; then mkdir -p $(BUILD_DIR) ; fi
	@if [ ! -d $(OUT_PREFIX) ] ; then mkdir -p $(OUT_PREFIX) ; fi

.PHONY : clean
clean:
	$(MAKE) -C $(OMC_DIR) clean
	$(MAKE) -C $(LIBOPENM_DIR) clean
	$(MAKE) -C $(LIBSQLITE_DIR) clean

.PHONY : cleanall
cleanall:
	$(MAKE) -C $(OMC_DIR) cleanall
	rm -rf $(BUILD_DIR)
	rm -rf $(OUT_LIB_DIR)
	@if [ -e $(OUT_BIN_DIR)/omc ] ;  then rm -f $(OUT_BIN_DIR)/omc ; fi 
	@if [ -e $(OUT_BIN_DIR)/omcD ] ; then rm -f $(OUT_BIN_DIR)/omcD ; fi 
	@if [ -e $(OUT_BIN_DIR)/omc$(BIN_POSTFIX) ] ; then rm -f $(OUT_BIN_DIR)/omc$(BIN_POSTFIX) ; fi 
