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

ifndef OM_ROOT
  error $(error Environmemt variable OM_ROOT must be defined in order to build the model)
endif

OM_INC_DIR = $(OM_ROOT)/include
OM_BIN_DIR = $(OM_ROOT)/bin
OM_LIB_DIR = $(OM_ROOT)/lib
OMC_USE_DIR = $(OM_ROOT)/use
OM_SQL_DIR = $(OM_ROOT)/sql
OM_SQLITE_DIR = $(OM_SQL_DIR)/sqlite
OM_MODELS_DIR = $(OM_ROOT)/models

ifndef OUT_PREFIX
  OUT_PREFIX = ompp-linux
  ifeq ($(PLATFORM_UNAME), Darwin)
    OUT_PREFIX = ompp-mac
  endif
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
# if OMC_NO_LINE defined then disable generation of #line directives
#
ifdef OMC_NO_LINE
  OMC_NO_LINE_OPT = -Omc.NoLineDirectives
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
  OPT_FLAG = -O3
  ifdef OM_OPT_LEVEL
    OPT_FLAG = -O$(OM_OPT_LEVEL)
  endif
  BD_CFLAGS = -DNDEBUG $(OPT_FLAG) $(OM_DEBUG_PARAM_OPT)
  DEPS_DIR = $(MODEL_BUILD_DIR)/release/deps
  OMC_OUT_DIR = $(MODEL_BUILD_DIR)/release/src
  OBJ_DIR = $(MODEL_BUILD_DIR)/release/obj
  BIN_POSTFIX =
endif

OUT_BIN_DIR = $(OUT_PREFIX)/bin

# model exe name: model name and optional D postfix in case of debug
MODEL_EXE = $(MODEL_NAME)$(BIN_POSTFIX)$(MSG_POSTFIX)

#
# model run arguments
#
RUN_OPT_INI =
ifdef MODEL_INI
  RUN_OPT_INI = -ini $(MODEL_INI)
endif

# set model root, for example: OM_NewCaseBased=/home/user/NewCaseBased
#
model_root_name = OM_$(MODEL_NAME)
model_root_value = $(CURDIR)

#
# location and name of output database
#
ifndef PUBLISH_DIR
  PUBLISH_DIR = $(OUT_BIN_DIR)
endif
MODEL_SQLITE = $(PUBLISH_DIR)/$(MODEL_NAME).sqlite

SQLITE_EXE = sqlite3
ifneq (,$(wildcard $(OM_BIN_DIR)/sqlite3))
  SQLITE_EXE = $(OM_BIN_DIR)/sqlite3
endif

#
# source files subdirectory: .ompp .mpp .odat .dat .cpp
#
ifndef MODEL_CODE_DIR
  MODEL_CODE_DIR = code
endif

# omc model documentation directories and options
#

# if not disabled then create model documentation
ifndef MODEL_DOC_DISABLE

  ifndef MODEL_INDOC_DIR
    MODEL_INDOC_DIR = doc
  endif

  ifndef MODEL_OUTDOC_DIR
    MODEL_OUTDOC_DIR = $(OUT_BIN_DIR)/doc
  endif

  OMC_MODEL_DOC_OPTS = -Omc.ModelDoc true -d $(MODEL_INDOC_DIR) -omc.OutDocDir $(MODEL_OUTDOC_DIR)
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
INFO_PLIST_FLAG =
ifeq ($(PLATFORM_UNAME), Darwin)
  L_UCVT_FLAG = -liconv
  STDC_FS_LIB = stdc++
  WL_PIE_FLAG = -Wl,-pie
  ifdef RELEASE
    INFO_PLIST_FLAG = -sectcreate __TEXT __info_plist $(MODEL_BUILD_DIR)/Info.plist
  endif
  ifndef BUNDLE_VERSION
    BUNDLE_VERSION = 0.0.1
  endif
endif

# address sanitizer
CC_ASAN_FLAGS =
LD_ASAN_FLAGS =
ifdef USE_ASAN
  CC_ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
  LD_ASAN_FLAGS = -fsanitize=address
endif

#
# cpp flags and special flags for omc-generated sources
#
CXXFLAGS = -Wall -std=c++20 -pthread -fPIE -fdiagnostics-color=auto -I$(OM_INC_DIR) -I$(OMC_OUT_DIR) -I./$(MODEL_CODE_DIR) $(CC_ASAN_FLAGS) $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

CXXFLAGS_OMC = -Wno-class-memaccess
ifeq ($(PLATFORM_UNAME), Darwin)
  CXXFLAGS_OMC = -Wno-pessimizing-move -Wno-missing-braces -Wno-braced-scalar-init
endif

#
# rules and targets
#

# recognize dependency files
SUFFIXES += .d

MODEL_MPP = $(wildcard $(MODEL_CODE_DIR)/*.mpp $(MODEL_CODE_DIR)/*.ompp $(MODEL_CODE_DIR)/*.dat $(MODEL_CODE_DIR)/*.odat)

MODEL_SCENARIO_DAT =
ifdef OMC_SCENARIO_PARAM_DIR
  MODEL_SCENARIO_DAT = $(wildcard $(OMC_SCENARIO_PARAM_DIR)/*.dat $(OMC_SCENARIO_PARAM_DIR)/*.odat)
endif

MODEL_FIXED_DAT =
ifdef OMC_FIXED_PARAM_DIR
  MODEL_FIXED_DAT = $(wildcard $(OMC_FIXED_PARAM_DIR)/*.dat $(OMC_FIXED_PARAM_DIR)/*.odat)
endif

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

.PHONY : install
install : all publish

$(MODEL_OMC_CPP) $(MODEL_CPP) : | prepare

.PHONY : omc_compile
$(MODEL_OMC_CPP) $(OMC_OUT_DIR)/$(MODEL_NAME)_create_sqlite.sql : $(MODEL_MPP) $(MODEL_SCENARIO_DAT) $(MODEL_FIXED_DAT)
	$(OMC_EXE) \
	-m $(MODEL_NAME) -s $(SCENARIO_NAME) -i $(CURDIR)/$(MODEL_CODE_DIR) -o $(OMC_OUT_DIR) -u $(OMC_USE_DIR) \
	$(OMC_SCENARIO_OPT) $(OMC_FIXED_OPT) $(OMC_CODE_PAGE_OPT) $(OMC_NO_LINE_OPT) -Omc.SqlDir $(OM_SQL_DIR) \
	$(OMC_MODEL_DOC_OPTS) \
	|| { echo "error at omc compile, exit code: " $$? ; kill $$PPID ; }

$(DEPS_DIR)/%.d : $(OMC_OUT_DIR)/%.cpp | omc_compile
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(DEPS_DIR)/%.d : $(MODEL_CODE_DIR)/%.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(OBJ_DIR)/%.o : $(OMC_OUT_DIR)/%.cpp | omc_compile
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_OMC) -c $< -o $@

$(OBJ_DIR)/%.o : $(MODEL_CODE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT_BIN_DIR)/$(MODEL_EXE) : $(OBJS) $(OM_LIB_DIR)/$(LIBOPENM_A) $(OM_LIB_DIR)/$(LIBSQLITE_A)
	$(CXX) \
	-pthread -L$(OM_LIB_DIR) $(WL_PIE_FLAG) $(LD_ASAN_FLAGS) $(INFO_PLIST_FLAG) \
	-o $@ \
	$(OBJS) \
	-lopenm$(BIN_POSTFIX)$(MSG_POSTFIX) -l$(OM_DB_LIB) -l$(STDC_FS_LIB) $(L_UCVT_FLAG)

#
# create output SQLite database
# copy model.ini and model.message.ini files into output folder
#
.PHONY : publish
publish : $(MODEL_SQLITE) publish-views copy_ini copy_extra_doc

$(MODEL_SQLITE) : $(OMC_OUT_DIR)/$(MODEL_NAME)_create_sqlite.sql
	mv -f $(OMC_OUT_DIR)/$(MODEL_NAME).sqlite $(MODEL_SQLITE)

.PHONY : copy_ini
copy_ini:
	@if [ -e $(MODEL_NAME).ini ] ; then cp -pvf $(MODEL_NAME).ini $(OUT_BIN_DIR) ; fi
	@if [ -e $(OMC_OUT_DIR)/$(MODEL_NAME).message.ini ] ; then cp -pvf $(OMC_OUT_DIR)/$(MODEL_NAME).message.ini $(OUT_BIN_DIR) ; fi

.PHONY : copy_extra_doc
copy_extra_doc:
ifndef MODEL_DOC_DISABLE
	@if [ -d $(MODEL_INDOC_DIR) ] ; then find $(MODEL_INDOC_DIR) -iname "*.pdf" -exec cp -pvf {} $(MODEL_OUTDOC_DIR) \;; fi
endif

.PHONY : publish-views
publish-views : \
  $(MODEL_SQLITE) \
  $(OM_SQLITE_DIR)/optional_meta_views_sqlite.sql \
  $(OMC_OUT_DIR)/$(MODEL_NAME)_optional_views_sqlite.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OM_SQLITE_DIR)/optional_meta_views_sqlite.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OMC_OUT_DIR)/$(MODEL_NAME)_optional_views_sqlite.sql

#
# run the model
#
.PHONY : run
run:
	cd $(OUT_BIN_DIR) && \
	$(model_root_name)=$(model_root_value) \
	./$(MODEL_EXE) $(RUN_OPT_INI) -OpenM.ProgressPercent 25

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d
	rm -f $(OMC_OUT_DIR)/*

.PHONY: clean-all
clean-all: clean
	rm -rf $(MODEL_BUILD_DIR)
	rm -f $(OUT_BIN_DIR)/$(MODEL_EXE)
	rm -f $(MODEL_SQLITE)
	rm -f $(OUT_BIN_DIR)/*.ini
	rm -f $(OUT_BIN_DIR)/*.log
	rm -rf $(MODEL_OUTDOC_DIR)
	rm -f $(OUT_BIN_DIR)/$(MODEL_NAME).extra.json

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OMC_OUT_DIR) ] ; then mkdir -p $(OMC_OUT_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_BIN_DIR) ] ; then mkdir -p $(OUT_BIN_DIR) ; fi
	@if [ ! -d $(PUBLISH_DIR) ] ; then mkdir -p $(PUBLISH_DIR) ; fi
ifndef MODEL_DOC_DISABLE
	@if [ ! -d $(MODEL_OUTDOC_DIR) ] ; then mkdir -p $(MODEL_OUTDOC_DIR) ; fi
endif
ifeq ($(PLATFORM_UNAME), Darwin)
	@sed \
	-e "s_<string>0.0.1</string>_<string>$(BUNDLE_VERSION)</string>_" \
	-e "s|<string>org.openmpp.MODEL</string>|<string>org.openmpp.$(MODEL_NAME)</string>|" \
	-e "s|<string>MODEL</string>|<string>$(MODEL_NAME)</string>|" \
	< $(OM_MODELS_DIR)/Model.Info.plist.txt \
	> $(MODEL_BUILD_DIR)/Info.plist
endif

# include dependencies for each .cpp file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean clean-all prepare)))
    -include $(DEPS)
endif

