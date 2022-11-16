# platform name: Linux or Darwin
PLATFORM_UNAME := $(shell uname -s)

ifeq ($(OM_MSG_USE), MPI)
  CXX = mpic++
  CC = mpicc
  OM_MSG_DEF = OM_MSG_MPI
  MSG_POSTFIX = _mpi
else
  CXX = g++
  CC = gcc
  OM_MSG_DEF = OM_MSG_EMPTY
  MSG_POSTFIX =
endif
CPP = $(CC)
AR = ar

ifndef BUILD_DIR
  BUILD_DIR = ../../build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ../..
endif

BUILD_PROJ_DIR = $(BUILD_DIR)/libopenm$(MSG_POSTFIX)
DEPS_DIR = $(BUILD_PROJ_DIR)/deps

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG -Og
  ifeq ($(PLATFORM_UNAME), Linux)
    BD_CFLAGS = -g -D_DEBUG -O0
  endif
  OUT_DIR = $(OUT_PREFIX)/lib
  OBJ_DIR = $(BUILD_PROJ_DIR)/debug
  BIN_POSTFIX = D
else
  BD_CFLAGS = -DNDEBUG -O3
  OUT_DIR = $(OUT_PREFIX)/lib
  OBJ_DIR = $(BUILD_PROJ_DIR)/release
  BIN_POSTFIX =
endif

INCLUDE_G_DIR = ../../include
INCLUDE_L_DIR = include

ifndef OM_DB_DEF
  OM_DB_DEF = OM_DB_SQLITE
  OM_DB_CFLAGS = -I../libsqlite
endif

ifndef OM_UCVT_DEF
  OM_UCVT_DEF = OM_UCVT_ICONV
endif

# address sanitizer
CC_ASAN_FLAGS =
ifdef USE_ASAN
  CC_ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
endif

# recognize dependency files
SUFFIXES += .d

CFLAGS = -pthread -fPIC -D$(OM_DB_DEF) -D$(OM_MSG_DEF) -D$(OM_UCVT_DEF) \
  -I$(INCLUDE_G_DIR) -I$(INCLUDE_L_DIR) $(OM_DB_CFLAGS) $(CC_ASAN_FLAGS) $(BD_CFLAGS)

CCFLAGS =  -Wall $(CFLAGS)
CXXFLAGS = -Wall -std=c++17 $(CFLAGS)

# build libopenm by default
# build libopenm_omc_db if: make lib_omc

ifeq (libopenm, $(filter libopenm,$(MAKECMDGOALS)))
  include makefile.openm.include.mak
endif
ifeq (all, $(MAKECMDGOALS))
  include makefile.openm.include.mak
endif
ifeq (, $(MAKECMDGOALS))
  include makefile.openm.include.mak
endif

ifeq (lib_omc, $(filter lib_omc,$(MAKECMDGOALS)))
  include makefile.lib_omc.include.mak
endif

LIB_A = $(LIB_TARGET)$(BIN_POSTFIX)$(MSG_POSTFIX).a

RT_SRC  := $(filter %.cpp,$(LIB_SRC))
RT_OBJS := $(foreach root,$(RT_SRC),$(OBJ_DIR)/$(notdir $(root:.cpp=.o)))
RT_DEPS := $(foreach root,$(RT_SRC),$(DEPS_DIR)/$(notdir $(root:.cpp=.d)))

RT_C_SRC  := $(filter %.c,$(LIB_SRC))
RT_C_OBJS := $(foreach root,$(RT_C_SRC),$(OBJ_DIR)/$(notdir $(root:.c=.o)))
RT_C_DEPS := $(foreach root,$(RT_C_SRC),$(DEPS_DIR)/$(notdir $(root:.c=.d)))

#
# targets and rules
#
.PHONY : all
all: libopenm

libopenm : prepare $(OUT_DIR)/$(LIB_A)

lib_omc : prepare $(OUT_DIR)/$(LIB_A)

$(RT_DEPS):   | prepare
$(RT_C_DEPS): | prepare
$(RT_OBJS):   | prepare
$(RT_C_OBJS): | prepare

$(RT_DEPS) : $(RT_SRC)
	$(CPP) -MM $(CXXFLAGS) $< -MF $@

$(RT_C_DEPS) : $(RT_C_SRC)
	$(CPP) -MM $(CCFLAGS) $< -MF $@

$(RT_OBJS) : $(RT_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $(OBJ_DIR)/$(@F)

$(RT_C_OBJS) : $(RT_C_SRC)
	$(CC) $(CCFLAGS) -c $< -o $(OBJ_DIR)/$(@F)

$(OUT_DIR)/$(LIB_A) : $(RT_OBJS) $(RT_C_OBJS)
	$(AR) rcs $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d

.PHONY: clean-all
clean-all: clean
	rm -f $(OUT_DIR)/$(LIB_A)
	rm -rf $(BUILD_PROJ_DIR)

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_DIR) ] ; then mkdir -p $(OUT_DIR) ; fi

# include dependencies for each .cpp file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean clean-all prepare)))
    -include $(RT_DEPS)
    -include $(RT_C_DEPS)
endif
