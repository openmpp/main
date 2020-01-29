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
  BUILD_DIR = ../build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ..
endif

ifndef OM_ROOT
  OM_ROOT = ../..
endif

OM_INC_DIR = $(OM_ROOT)/include
OM_BIN_DIR = $(OM_ROOT)/bin
OM_LIB_DIR = $(OM_ROOT)/lib
OM_SQL_DIR = $(OM_ROOT)/sql
OM_SQLITE_DIR = $(OM_SQL_DIR)/sqlite

#
# model name: current dir name by default
#
CUR_SUBDIR = $(notdir $(CURDIR))
ifndef MODEL_NAME
  MODEL_NAME = $(CUR_SUBDIR)
endif

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

LIBOPENM_A = libopenm$(BIN_POSTFIX)$(MSG_POSTFIX).a
LIBSQLITE_A = libsqlite$(BIN_POSTFIX).a

L_UCVT_FLAG =
ifeq ($(PLATFORM_UNAME), Darwin)
  L_UCVT_FLAG = -liconv
endif

# recognize dependency files
SUFFIXES += .d

CXXFLAGS = -Wall -std=c++17 -pthread -fPIE -I$(OM_INC_DIR) $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

MODEL_EXE = $(MODEL_NAME)$(BIN_POSTFIX)$(MSG_POSTFIX)

#MODEL_PATH = $(CURDIR)
MODEL_CPPLIST = \
    modelOne.cpp \
    modelOne_om.cpp

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
	$(CXX) -pthread -L$(OM_LIB_DIR) -pie -o $@ $(OBJS) -lopenm$(BIN_POSTFIX)$(MSG_POSTFIX) -l$(OM_DB_LIB) -lstdc++ $(L_UCVT_FLAG)

#
# create model SQLite database
#
SQLITE_EXE = sqlite3
MODEL_SQLITE = $(OUT_BIN_DIR)/$(MODEL_NAME).sqlite

.PHONY : publish
publish : $(MODEL_SQLITE) copy_ini

$(MODEL_SQLITE) : \
  $(OM_SQL_DIR)/create_db.sql \
  $(MODEL_NAME)_create_model.sql \
  $(MODEL_NAME)_insert_param.sql
	rm -f $(MODEL_SQLITE)
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OM_SQL_DIR)/create_db.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OM_SQL_DIR)/insert_default.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(MODEL_NAME)_create_model.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(MODEL_NAME)_insert_param.sql

.PHONY : publish-views
publish-views : \
  publish \
  $(OM_SQLITE_DIR)/optional_meta_views_sqlite.sql \
  $(MODEL_NAME)_optional_views.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(OM_SQLITE_DIR)/optional_meta_views_sqlite.sql
	$(SQLITE_EXE) $(MODEL_SQLITE) < $(MODEL_NAME)_optional_views.sql

.PHONY : copy_ini
copy_ini:
	@if [ -e modelOne.ini ] ; then cp -pf modelOne.ini $(OUT_BIN_DIR) ; fi
	@if [ -e modelOne.message.ini ] ; then cp -pf modelOne.message.ini $(OUT_BIN_DIR) ; fi

.PHONY : publish_views
publish_views : publish

#
# run the model
#
.PHONY : run
run:
	$(OUT_BIN_DIR)/$(MODEL_EXE) \
		-OpenM.RunName Default \
		-OpenM.Database Database="$(MODEL_SQLITE);Timeout=86400;OpenMode=ReadWrite"
	$(OUT_BIN_DIR)/$(MODEL_EXE) \
		-OpenM.SubValues 4 -OpenM.Threads 4 \
		-OpenM.Profile modelOne \
		-OpenM.RunName Default-4 \
		-OpenM.Database Database="$(MODEL_SQLITE);Timeout=86400;OpenMode=ReadWrite"
	$(OUT_BIN_DIR)/$(MODEL_EXE) \
		-OpenM.TaskName taskOne \
		-OpenM.TaskRunName "First Task Run" \
		-OpenM.Database Database="$(MODEL_SQLITE);Timeout=86400;OpenMode=ReadWrite"
	$(OUT_BIN_DIR)/$(MODEL_EXE) \
		-OpenM.SubValues 2 -SubFrom.salaryFull csv -OpenM.ParamDir ./csv \
		-OpenM.RunName "Parameter sub-values 2 from csv" \
		-OpenM.Database Database="$(MODEL_SQLITE);Timeout=86400;OpenMode=ReadWrite"
	$(OUT_BIN_DIR)/$(MODEL_EXE) \
		-OpenM.SubValues 4 -OpenM.Threads 4 \
		-SubFrom.baseSalary db -SubFrom.salaryFull db -SubFrom.filePath db -SubFrom.isOldAge db \
		-OpenM.RunName "Parameter sub-values 4" \
		-OpenM.Database Database="$(MODEL_SQLITE);Timeout=86400;OpenMode=ReadWrite"
	$(OUT_BIN_DIR)/$(MODEL_EXE) \
		-OpenM.SubValues 2 \
		-SubGroupFrom.AgeSexParameters csv -SubGroupValues.AgeSexParameters 2,3 -OpenM.ParamDir ./csv -SubFrom.salaryFull db \
		-OpenM.RunName "Parameter group sub-values 2 from csv" \
		-OpenM.Database Database="$(MODEL_SQLITE);Timeout=86400;OpenMode=ReadWrite"
	$(OUT_BIN_DIR)/$(MODEL_EXE) \
		-ImportRunName.modelOne Default \
		-OpenM.RunName "Import parameters from Default run" \
		-OpenM.Database Database="$(MODEL_SQLITE);Timeout=86400;OpenMode=ReadWrite"

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d

.PHONY: cleanall
cleanall: clean
	rm -rf $(MODEL_BUILD_DIR)
	rm -f $(OUT_BIN_DIR)/$(MODEL_EXE)
	rm -f $(MODEL_SQLITE)

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_BIN_DIR) ] ; then mkdir -p $(OUT_BIN_DIR) ; fi

# include dependencies for each .cpp file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean cleanall prepare)))
    -include $(DEPS)
endif

