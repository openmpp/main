CXX = g++
CC = gcc
CPP = $(CC)
AR = ar

ifndef BUILD_DIR
  BUILD_DIR = ../build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ..
endif

ifndef OM_ROOT
  OM_ROOT = ../..
endif

OM_INC_DIR = $(OM_ROOT)/include
OM_INC_LIBOPENM_DIR = $(OM_ROOT)/openm/libopenm/include
OM_BIN_DIR = $(OM_ROOT)/bin
OM_LIB_DIR = $(OM_ROOT)/lib
OM_SQL_DIR = $(OM_ROOT)/sql
OM_SQLITE_DIR = $(OM_SQL_DIR)/sqlite

#
# model name: current dir name by default
#
CUR_SUBDIR = $(notdir $(CURDIR))

#
# build directories
# if model build directory defined globally 
#   then assume shared some/build/ location and use model name to avoid conflicts
#
MODEL_BUILD_DIR = $(BUILD_DIR)/$(CUR_SUBDIR)

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  DEPS_DIR = $(MODEL_BUILD_DIR)/debug/deps
  OBJ_DIR = $(MODEL_BUILD_DIR)/debug/obj
  OUT_BIN_DIR = $(OUT_PREFIX)/bin
  BIN_POSTFIX = D
else
  BD_CFLAGS = -DNDEBUG -O3
  DEPS_DIR = $(MODEL_BUILD_DIR)/release/deps
  OBJ_DIR = $(MODEL_BUILD_DIR)/release/obj
  OUT_BIN_DIR = $(OUT_PREFIX)/bin
  BIN_POSTFIX =
endif

ifndef OM_DB_LIB
#  OM_DB_LIB = sqlite3
  OM_DB_LIB = sqlite$(BIN_POSTFIX)
endif

LIBOPENM_A = libopenm$(BIN_POSTFIX).a
LIBSQLITE_A = libsqlite$(BIN_POSTFIX).a

# recognize dependency files
SUFFIXES += .d

CXXFLAGS = -Wall -std=c++11 -D_REENTRANT -I$(OM_INC_DIR) -I$(OM_INC_LIBOPENM_DIR) $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

MODEL_EXE = $(CUR_SUBDIR)$(BIN_POSTFIX)

#MODEL_PATH = $(CURDIR)
MODEL_CPPLIST = modelOneDb.cpp 

sources = $(MODEL_CPPLIST)

OBJS := $(foreach root,$(sources:.cpp=.o),$(OBJ_DIR)/$(root))
DEPS := $(foreach root,$(sources:.cpp=.d),$(DEPS_DIR)/$(root))
    
# vpath %.cpp $(MODEL_PATH)
     
.PHONY : all
all: model 

.PHONY : model
model: prepare $(OUT_BIN_DIR)/$(MODEL_EXE)

$(OBJS): | prepare
$(DEPS): | prepare

$(DEPS_DIR)/%.d : %.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OUT_BIN_DIR)/$(MODEL_EXE) : $(OBJS) $(OM_LIB_DIR)/$(LIBOPENM_A) $(OM_LIB_DIR)/$(LIBSQLITE_A)
	$(CXX) -L$(OM_LIB_DIR) -o $@ $(OBJS) -lopenm$(BIN_POSTFIX) -l$(OM_DB_LIB) -lstdc++ -lpthread

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d

.PHONY: clean-all
clean-all: clean
	rm -rf $(MODEL_BUILD_DIR)
	rm -f $(OUT_BIN_DIR)/$(MODEL_EXE)
	rm -f $(MODEL_SQLITE)
	@if [ -d $(OUT_BIN_DIR) ] ; then rmdir --ignore-fail-on-non-empty $(OUT_BIN_DIR) ; fi 

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_BIN_DIR) ] ; then mkdir -p $(OUT_BIN_DIR) ; fi

# include dependencies for each .cpp file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean clean-all prepare)))
    -include $(DEPS)
endif


