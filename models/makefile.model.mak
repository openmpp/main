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
  error $(error Environmemt variable OM_ROOT must be defined in order to build the model)
endif

OM_INC_DIR = $(OM_ROOT)/include
OM_BIN_DIR = $(OM_ROOT)/bin
OM_LIB_DIR = $(OM_ROOT)/lib
OMC_USE_DIR = $(OM_ROOT)/use
OM_SQLITE_DIR = $(OM_ROOT)/sql/sqlite

ifndef OUT_PREFIX
  OUT_PREFIX = ompp-linux
endif

#
# arguments for run control
#
ifdef MEMBERS
  RUN_OPT_MEMBERS = -General.Subsamples $(MEMBERS)
endif

ifdef THREADS
  RUN_OPT_THREADS = -General.Threads $(THREADS)
endif

#
# model name: current dir name by default
#
CUR_SUBDIR = $(notdir $(CURDIR))

ifndef MODEL_NAME
  MODEL_NAME = $(CUR_SUBDIR)
endif

#
# model default parameters: if SCENARIO_NAME subdirectory exist the use it
#
ifndef SCENARIO_NAME
  SCENARIO_NAME = Default
endif

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
# build directories
# if model build directory defined globally 
#   then assume shared some/build/ location and use model name to avoid conflicts
#
ifndef BUILD_DIR
  MODEL_BUILD_DIR = $(OUT_PREFIX)/build
else
  MODEL_BUILD_DIR = $(BUILD_DIR)/$(CUR_SUBDIR)
endif

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  DEPS_DIR = $(MODEL_BUILD_DIR)/debug/deps
  OMC_OUT_DIR = $(MODEL_BUILD_DIR)/debug/src
  OBJ_DIR = $(MODEL_BUILD_DIR)/debug/obj
else
  BD_CFLAGS = -DNDEBUG -O3
  DEPS_DIR = $(MODEL_BUILD_DIR)/release/deps
  OMC_OUT_DIR = $(MODEL_BUILD_DIR)/release/src
  OBJ_DIR = $(MODEL_BUILD_DIR)/release/obj
endif

OUT_BIN_DIR = $(OUT_PREFIX)/bin

# model exe name: model name and optional D postfix in case of debug
MODEL_EXE = $(MODEL_NAME)$(BIN_POSTFIX)

#
# location and name of output database
#
ifndef PUBLISH_DIR
  PUBLISH_DIR = output-linux
  MODEL_SQLITE = $(PUBLISH_DIR)/$(MODEL_EXE)_$(SCENARIO_NAME).sqlite
else
  MODEL_SQLITE = $(PUBLISH_DIR)/$(MODEL_EXE).sqlite
endif

#
# source files subdirectory: .ompp .mpp .odat .dat .cpp
#
ifndef MODEL_CODE_DIR
  MODEL_CODE_DIR = code
endif

#
# libraries and omc: openM++ compiler
#
# OMC_EXE = $(OM_BIN_DIR)/omc$(BIN_POSTFIX)
OMC_EXE = $(OM_BIN_DIR)/omc

ifndef OM_DB_LIB
#  OM_DB_LIB = sqlite3
  OM_DB_LIB = sqlite$(BIN_POSTFIX)
endif

LIBOPENM_A = libopenm$(BIN_POSTFIX).a
LIBSQLITE_A = libsqlite$(BIN_POSTFIX).a

#
# rules and targets
#

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
	$(CXX) -L$(OM_LIB_DIR) -lopenm$(BIN_POSTFIX) -l$(OM_DB_LIB) -lstdc++ -lpthread -o $@ $(OBJS)

#
# create output SQLite database
#
SQLITE_EXE = sqlite3

.PHONY : publish
publish : $(MODEL_SQLITE)

$(MODEL_SQLITE) : \
  $(OM_SQLITE_DIR)/create_db_sqlite.sql \
  $(OM_SQLITE_DIR)/optional_meta_views_sqlite.sql \
  $(OMC_OUT_DIR)/$(MODEL_NAME)_create_model.sql \
  $(OMC_OUT_DIR)/$(MODEL_NAME)_$(SCENARIO_NAME).sql \
  $(OMC_OUT_DIR)/$(MODEL_NAME)_optional_views.sql
	rm -f $(MODEL_SQLITE)
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OM_SQLITE_DIR)/create_db_sqlite.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OM_SQLITE_DIR)/optional_meta_views_sqlite.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OMC_OUT_DIR)/$(MODEL_NAME)_create_model.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OMC_OUT_DIR)/$(MODEL_NAME)_$(SCENARIO_NAME).sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OMC_OUT_DIR)/$(MODEL_NAME)_optional_views.sql

#
# run the model
#
.PHONY : run
run:
	$(OUT_BIN_DIR)/$(MODEL_EXE) $(RUN_OPT_MEMBERS) $(RUN_OPT_THREADS) \
		-OpenM.Database Database="$(MODEL_SQLITE);Timeout=86400;OpenMode=ReadWrite"

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d
	rm -f $(OMC_OUT_DIR)/*

.PHONY: cleanall
cleanall: clean
	rm -rf $(MODEL_BUILD_DIR)
	rm -f $(OUT_BIN_DIR)/$(MODEL_EXE)
	rm -f $(MODEL_SQLITE)
	@if [ -d $(OUT_BIN_DIR) ] ; then rmdir --ignore-fail-on-non-empty $(OUT_BIN_DIR) ; fi 
	@if [ -d $(PUBLISH_DIR) ] ; then rmdir --ignore-fail-on-non-empty $(PUBLISH_DIR) ; fi 
	@if [ -d $(OUT_PREFIX) ] ;  then rmdir --ignore-fail-on-non-empty $(OUT_PREFIX) ; fi 

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
