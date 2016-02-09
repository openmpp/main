ifeq ($(OM_MSG_USE), MPI)
  CXX = mpic++
  CC = mpicc
  OM_MSG_DEF = OM_MSG_MPI
else
  CXX = g++
  CC = gcc
  OM_MSG_DEF = OM_MSG_EMPTY
endif
CPP = $(CC)
AR = ar

ifndef OM_ROOT
  OM_ROOT = ../..
endif

ifndef OM_INC_DIR
  OM_INC_DIR = $(OM_ROOT)/include
endif

ifndef OM_BIN_DIR
  OM_BIN_DIR = $(OM_ROOT)/bin
endif

ifndef OM_LIB_DIR
  OM_LIB_DIR = $(OM_ROOT)/lib
endif

ifndef OM_SQLITE_DIR
  OMC_SQLITE_DIR = $(OM_ROOT)/sql/sqlite
endif

ifndef OMC_USE_DIR
  OMC_USE_DIR = $(OM_ROOT)/use
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ompp-linux/bin
endif

#
# default arguments for run control
#

ifdef MEMBERS
  RUN_OPT_MEMBERS = -General.Subsamples $(MEMBERS) $(RUN_OPTS)
endif

ifdef THREADS
  RUN_OPT_THREADS = -General.Threads $(THREADS) $(RUN_OPTS)
endif

#
# location of output database
#
ifndef PUBLISH_DIR
  PUBLISH_DIR = output-linux
endif

#
# model executable name: current dir name by default
# model name: same as executable name by default
#
ifndef MODEL_EXE
  MODEL_EXE = $(notdir $(CURDIR))
endif

ifndef MODEL_NAME
  MODEL_NAME = $(MODEL_EXE)
endif

#
# scenario name
#
ifndef SCENARIO_NAME
  SCENARIO_NAME = Default
endif

#
# model default parameters: if SCENARIO_NAME subdirectory exist the use it
#
ifndef OMC_DISABLE_AUTO_PARAM_DIR
  ifndef OMC_SCENARIO_PARAM_DIR
    ifneq (,$(wildcard parameters/$(SCENARIO_NAME)))
      OMC_SCENARIO_PARAM_DIR = parameters/$(SCENARIO_NAME)
    endif
  endif
endif

ifdef OMC_SCENARIO_PARAM_DIR
  OMC_SCENARIO_OPT = -p $(OMC_SCENARIO_PARAM_DIR)
endif

#
# model fixed parameters: if Fixed subdirectory exist the use it
#
ifndef OMC_DISABLE_AUTO_PARAM_DIR
  ifndef OMC_FIXED_PARAM_DIR
    ifneq (,$(wildcard parameters/Fixed))
      OMC_FIXED_PARAM_DIR = parameters/Fixed
    endif
  endif
endif

ifdef OMC_FIXED_PARAM_DIR
  OMC_FIXED_OPT = -f $(OMC_FIXED_PARAM_DIR)
endif

#
# source files subdirectory: .ompp .mpp .odat .dat
#
ifndef MODEL_CODE_DIR
  MODEL_CODE_DIR = code
endif

#
# build directories
# if model build directory defined globally 
#   then assume shared some/build/ location and use model name to avoid conflicts
#
ifndef BUILD_DIR
  MODEL_BUILD_DIR = ompp-linux/build
else
  MODEL_BUILD_DIR = $(BUILD_DIR)/$(MODEL_EXE)
endif
DEPS_DIR = $(MODEL_BUILD_DIR)/deps
OMC_OUT_DIR = $(MODEL_BUILD_DIR)/src

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  OBJ_DIR = $(MODEL_BUILD_DIR)/debug
  OUT_BIN_DIR = $(OUT_PREFIX)
else
  BD_CFLAGS = -DNDEBUG -O3
  OBJ_DIR = $(MODEL_BUILD_DIR)/release
  OUT_BIN_DIR = $(OUT_PREFIX)
endif

ifndef OM_DB_LIB
#  OM_DB_LIB = sqlite3
  OM_DB_LIB = sqlite
endif

OMC_EXE = $(OM_BIN_DIR)/omc

LIBOPENM_A = libopenm.a
LIBSQLITE_A = libsqlite.a

# recognize dependency files
SUFFIXES += .d

CXXFLAGS = -Wall -std=c++11 -fdiagnostics-color=auto -D_REENTRANT -I$(OM_INC_DIR) -I$(OMC_OUT_DIR) -I./$(MODEL_CODE_DIR) $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

MODEL_MPP = $(wildcard $(MODEL_CODE_DIR)/*.mpp $(MODEL_CODE_DIR)/*.ompp $(MODEL_CODE_DIR)/*.dat $(MODEL_CODE_DIR)/*.odat)

MODEL_CPP = $(wildcard $(MODEL_CODE_DIR)/*.cpp)

MODEL_OMC_CPP = \
  $(OMC_OUT_DIR)/om_definitions.cpp \
  $(OMC_OUT_DIR)/om_developer.cpp \
  $(OMC_OUT_DIR)/om_fixed_parms.cpp

MODEL_CPPLIST = \
  $(MODEL_OMC_CPP) \
  $(MODEL_CPP)

sources = $(MODEL_CPPLIST)

OBJS := $(foreach root,$(sources:.cpp=.o),$(OBJ_DIR)/$(notdir $(root)))
DEPS := $(foreach root,$(sources:.cpp=.d),$(DEPS_DIR)/$(notdir $(root)))
    
.PHONY : all
all: model 

.PHONY : model
model: prepare $(OUT_BIN_DIR)/$(MODEL_EXE)

$(MODEL_OMC_CPP) $(MODEL_CPP) : | prepare

$(MODEL_OMC_CPP) : $(MODEL_MPP)
	$(OMC_EXE) -m $(MODEL_NAME) -s $(SCENARIO_NAME) -i $(CURDIR)/$(MODEL_CODE_DIR) -o $(OMC_OUT_DIR) -u $(OMC_USE_DIR) $(OMC_SCENARIO_OPT) $(OMC_FIXED_OPT)

$(DEPS_DIR)/%.d : $(OMC_OUT_DIR)/%.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(DEPS_DIR)/%.d : $(MODEL_CODE_DIR)/%.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(OBJ_DIR)/%.o : $(OMC_OUT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OBJ_DIR)/%.o : $(MODEL_CODE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OUT_BIN_DIR)/$(MODEL_EXE) : $(OBJS) $(OM_LIB_DIR)/$(LIBOPENM_A) $(OM_LIB_DIR)/$(LIBSQLITE_A)
	$(CXX) -L$(OM_LIB_DIR) -lopenm -l$(OM_DB_LIB) -lstdc++ -lpthread -o $@ $(OBJS)

#
# create output SQLite database
#
SQLITE_EXE = sqlite3
PUBLISH_SQLITE = $(PUBLISH_DIR)/$(MODEL_NAME)_$(SCENARIO_NAME).sqlite

.PHONY : publish
publish : $(PUBLISH_SQLITE)

$(PUBLISH_SQLITE) :
	rm -f $(PUBLISH_SQLITE)
	$(SQLITE_EXE) $(PUBLISH_SQLITE) < $(OMC_SQLITE_DIR)/create_db_sqlite.sql
	$(SQLITE_EXE) $(PUBLISH_SQLITE) < $(OMC_SQLITE_DIR)/optional_meta_views_sqlite.sql
	$(SQLITE_EXE) $(PUBLISH_SQLITE) < $(OMC_OUT_DIR)/$(MODEL_NAME)_create_model.sql
	$(SQLITE_EXE) $(PUBLISH_SQLITE) < $(OMC_OUT_DIR)/$(MODEL_NAME)_optional_views.sql
	$(SQLITE_EXE) $(PUBLISH_SQLITE) < $(OMC_OUT_DIR)/$(MODEL_NAME)_$(SCENARIO_NAME).sql

.PHONY : run
run:
	$(OUT_BIN_DIR)/$(MODEL_EXE) $(RUN_OPT_MEMBERS) $(RUN_OPT_THREADS) \
		-OpenM.Database Database="$(PUBLISH_SQLITE);Timeout=86400;OpenMode=ReadWrite"

.PHONY: clean
clean:
	rm -f $(OUT_BIN_DIR)/$(MODEL_EXE)
	rm -f $(OUT_BIN_DIR)/$(MODEL_NAME)*.sql
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d
	rm -f $(OMC_OUT_DIR)/*
	rm -f $(PUBLISH_SQLITE)

.PHONY: cleanall
cleanall: clean
	rm -rf $(MODEL_BUILD_DIR)
	rm -rf $(PUBLISH_DIR)

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OMC_OUT_DIR) ] ; then mkdir -p $(OMC_OUT_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_BIN_DIR) ] ; then mkdir -p $(OUT_BIN_DIR) ; fi
	@if [ ! -d $(PUBLISH_DIR) ] ; then mkdir -p $(PUBLISH_DIR) ; fi

# include dependencies for each .cpp file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean cleanall prepare)))
    -include $(DEPS)
endif

