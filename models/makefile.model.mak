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
SQLITE_EXE = sqlite3

ifndef OM_ROOT
  error $(error Environmemt variable OM_ROOT must be defined in order to build the model)
endif

OM_INC_DIR = $(OM_ROOT)/include
OM_BIN_DIR = $(OM_ROOT)/bin
OM_LIB_DIR = $(OM_ROOT)/lib
OMC_USE_DIR = $(OM_ROOT)/use
OM_SQL_DIR = $(OM_ROOT)/sql
OM_SQLITE_DIR = $(OM_SQL_DIR)/sqlite

ifndef OUT_PREFIX
  OUT_PREFIX = ompp-linux
  ifeq ($(PLATFORM_UNAME), Darwin)
    OUT_PREFIX = ompp-mac
  endif
endif

#
# arguments for run control
#
ifdef MEMBERS
  RUN_OPT_MEMBERS = -OpenM.SubValues $(MEMBERS)
endif

ifdef THREADS
  RUN_OPT_THREADS = -OpenM.Threads $(THREADS)
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
# convert source files to utf-8 from Windows code page
#
ifdef OMC_CODE_PAGE
  OMC_CODE_PAGE_OPT = -Omc.CodePage $(OMC_CODE_PAGE)
endif

#
# if OMC_NO_LINE is true then disable generation of #line directives
# true case-insenstive "true" or "yes" or "1" anything else is false
#
ifdef OMC_NO_LINE
  OMC_NO_LINE_OPT = -Omc.NoLineDirectives $(OMC_NO_LINE)
endif

# OM_DEBUG_PARAMETERS:
#   if defined then parameter values visible in debug variable view (suitable for debug)
#   else parameters consume significantly less memory (suitable for release)
#
ifdef OM_DEBUG_PARAMETERS
    OM_DEBUG_PARAM_OPT = -DOM_DEBUG_PARAMETERS
else
  ifndef RELEASE
    OM_DEBUG_PARAM_OPT = -DOM_DEBUG_PARAMETERS
  endif
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
  BD_CFLAGS = -g -D_DEBUG -Og $(OM_DEBUG_PARAM_OPT)
  ifeq ($(PLATFORM_UNAME), Linux)
    BD_CFLAGS = -g -D_DEBUG -O0 $(OM_DEBUG_PARAM_OPT)
  endif
  DEPS_DIR = $(MODEL_BUILD_DIR)/debug/deps
  OMC_OUT_DIR = $(MODEL_BUILD_DIR)/debug/src
  OBJ_DIR = $(MODEL_BUILD_DIR)/debug/obj
  BIN_POSTFIX = D
else
  BD_CFLAGS = -DNDEBUG -O3 $(OM_DEBUG_PARAM_OPT)
  DEPS_DIR = $(MODEL_BUILD_DIR)/release/deps
  OMC_OUT_DIR = $(MODEL_BUILD_DIR)/release/src
  OBJ_DIR = $(MODEL_BUILD_DIR)/release/obj
  BIN_POSTFIX =
endif

OUT_BIN_DIR = $(OUT_PREFIX)/bin

# model exe name: model name and optional D postfix in case of debug
MODEL_EXE = $(MODEL_NAME)$(BIN_POSTFIX)$(MSG_POSTFIX)

#
# location and name of output database
#
ifndef PUBLISH_DIR
  PUBLISH_DIR = $(OUT_BIN_DIR)
endif
MODEL_SQLITE = $(PUBLISH_DIR)/$(MODEL_NAME).sqlite

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
  OM_DB_LIB = sqlite$(BIN_POSTFIX)
endif

LIBOPENM_A = libopenm$(BIN_POSTFIX)$(MSG_POSTFIX).a
LIBSQLITE_A = libsqlite$(BIN_POSTFIX).a

L_UCVT_FLAG =
STDC_FS_LIB = stdc++fs
WL_PIE_FLAG = -pie
ifeq ($(PLATFORM_UNAME), Darwin)
  L_UCVT_FLAG = -liconv
  STDC_FS_LIB = stdc++
  WL_PIE_FLAG = -Wl,-pie
endif

#
# cpp flags and special flags for omc-generated sources
#
CXXFLAGS = -Wall -std=c++17 -pthread -fPIE -fdiagnostics-color=auto -I$(OM_INC_DIR) -I$(OMC_OUT_DIR) -I./$(MODEL_CODE_DIR) $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

CXXFLAGS_OMC =
ifeq ($(PLATFORM_UNAME), Darwin)
  CXXFLAGS_OMC = -Wno-pessimizing-move -Wno-missing-braces
endif

#
# rules and targets
#

# recognize dependency files
SUFFIXES += .d

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

$(MODEL_OMC_CPP) $(OMC_OUT_DIR)/$(MODEL_NAME)_create_sqlite.sql : $(MODEL_MPP)
	$(OMC_EXE) -m $(MODEL_NAME) -s $(SCENARIO_NAME) -i $(CURDIR)/$(MODEL_CODE_DIR) -o $(OMC_OUT_DIR) -u $(OMC_USE_DIR) -Omc.SqlDir $(OM_SQL_DIR) $(OMC_SCENARIO_OPT) $(OMC_FIXED_OPT) $(OMC_CODE_PAGE_OPT) $(OMC_NO_LINE_OPT)

$(DEPS_DIR)/%.d : $(OMC_OUT_DIR)/%.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(DEPS_DIR)/%.d : $(MODEL_CODE_DIR)/%.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(OBJ_DIR)/%.o : $(OMC_OUT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_OMC) -c $< -o $@

$(OBJ_DIR)/%.o : $(MODEL_CODE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT_BIN_DIR)/$(MODEL_EXE) : $(OBJS) $(OM_LIB_DIR)/$(LIBOPENM_A) $(OM_LIB_DIR)/$(LIBSQLITE_A)
	$(CXX) -pthread -L$(OM_LIB_DIR) $(WL_PIE_FLAG) -o $@ $(OBJS) -lopenm$(BIN_POSTFIX)$(MSG_POSTFIX) -l$(OM_DB_LIB) -l$(STDC_FS_LIB) $(L_UCVT_FLAG)

#
# create output SQLite database
#
.PHONY : publish
publish : $(MODEL_SQLITE) copy_ini

$(MODEL_SQLITE) : $(OMC_OUT_DIR)/$(MODEL_NAME)_create_sqlite.sql
	mv -f $(OMC_OUT_DIR)/$(MODEL_NAME).sqlite $(MODEL_SQLITE)

.PHONY : copy_ini
copy_ini:
	@if [ -e $(MODEL_NAME).ini ] ; then cp -pf $(MODEL_NAME).ini $(OUT_BIN_DIR) ; fi
	@if [ -e $(OMC_OUT_DIR)/$(MODEL_NAME).message.ini ] ; then cp -pf $(OMC_OUT_DIR)/$(MODEL_NAME).message.ini $(OUT_BIN_DIR) ; fi

.PHONY : publish-views
publish-views : publish
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OM_SQLITE_DIR)/optional_meta_views_sqlite.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OMC_OUT_DIR)/$(MODEL_NAME)_optional_views_sqlite.sql

#
# run the model
#
.PHONY : run
run:
	$(OUT_BIN_DIR)/$(MODEL_EXE) $(RUN_OPT_MEMBERS) $(RUN_OPT_THREADS) \
		-OpenM.ProgressPercent 25 \
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

