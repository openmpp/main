ifndef BUILD_DIR
  BUILD_DIR = ../build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ..
endif

LIBSQLITE_DIR = libsqlite
LIBOPENM_DIR = libopenm
OMC_DIR = omc
OMC_EXE = omc

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

libsqlite: prepare
	$(MAKE) -C $(LIBSQLITE_DIR) 

libopenm: prepare libsqlite
	$(MAKE) -C $(LIBOPENM_DIR) libopenm

lib_omc: prepare
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
	rm -rf $(OUT_PREFIX)/lib
	rm -rf $(BUILD_DIR)
