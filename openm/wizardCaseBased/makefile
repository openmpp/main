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

BUILD_DIR = ../build
OUT_PREFIX = ..
INCLUDE_DIR = ../include
DEPS_DIR = $(BUILD_DIR)/wizardCaseBased/deps
OMC_OUT_DIR = $(BUILD_DIR)/wizardCaseBased/src

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  OBJ_DIR = $(BUILD_DIR)/wizardCaseBased/debug
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/debug
  OUT_LIB_DIR = $(OUT_PREFIX)/lib/debug
else
  BD_CFLAGS = -DNDEBUG -O3
  OBJ_DIR = $(BUILD_DIR)/wizardCaseBased/release
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/release
  OUT_LIB_DIR = $(OUT_PREFIX)/lib/release
endif

OMC_EXE = $(OUT_BIN_DIR)/omc
LIBOPENM_A = libopenm.a
LIBSQLITE_A = libsqlite.a

ifndef OM_DB_LIB
#  OM_DB_LIB = sqlite3
  OM_DB_LIB = sqlite
endif

CXXFLAGS = -Wall -std=c++11 -D_REENTRANT -I$(INCLUDE_DIR) -I$(OMC_OUT_DIR) $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

MODEL_EXE = wizardCaseBased
MODEL_SQL_PREFIX = WizardCaseBased

MODEL_MPP = \
  PersonCore.mpp \
  WizardCaseBased.mpp \
  WizardCaseBasedFR.mpp

MODEL_OMC_SQL = \
  $(OMC_OUT_DIR)/$(MODEL_SQL_PREFIX)_create_model.sql \
  $(OMC_OUT_DIR)/$(MODEL_SQL_PREFIX)_optional_views.sql

MODEL_OMC_CPP = \
  $(OMC_OUT_DIR)/om_definitions.cpp \
  $(OMC_OUT_DIR)/om_developer.cpp

MODEL_CPPLIST = \
  hard_coded.cpp \
  simulation.cpp \
  $(MODEL_OMC_CPP)

sources = $(MODEL_CPPLIST)

objects = $(foreach root,$(sources:.cpp=.o),$(OBJ_DIR)/$(notdir $(root)))
    
vpath %.cpp $(CURDIR)
vpath %.cpp $(OMC_OUT_DIR)
     
.PHONY : all
all: model 

.PHONY : model
model: prepare $(OUT_BIN_DIR)/$(MODEL_EXE) model_sql

$(DEPS_DIR)/%.d : %.cpp
	@set -e; rm -f $@; \
	$(CPP) -MM $(CPPFLAGS) -c $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g ' < $@.$$$$ > $(DEPS_DIR)/$(@F); \
	rm -f $@.$$$$

$(MODEL_OMC_CPP) $(MODEL_OMC_SQL) : $(MODEL_MPP)
	$(OMC_EXE) -i $(CURDIR) -o $(OMC_OUT_DIR)

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OUT_BIN_DIR)/$(MODEL_EXE) : $(objects) $(OUT_LIB_DIR)/$(LIBOPENM_A) $(OUT_LIB_DIR)/$(LIBSQLITE_A)
	$(CXX) -L$(OUT_LIB_DIR) -lopenm -l$(OM_DB_LIB) -lstdc++ -lpthread -o $@ $(objects)

model_sql : $(MODEL_OMC_SQL)
	cp -pf $(OMC_OUT_DIR)/$(MODEL_SQL_PREFIX)*.sql $(OUT_BIN_DIR)

.PHONY: clean
clean:
	rm -f $(OUT_BIN_DIR)/$(MODEL_EXE)
	rm -f $(OUT_BIN_DIR)/$(MODEL_SQL_PREFIX)*.sql
	rm -f $(OBJ_DIR)/*.o

.PHONY: cleanall
cleanall: clean
	rm -f $(DEPS_DIR)/*
	rm -f $(OBJ_DIR)/*
	rm -rf $(OUT_BIN_DIR)

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_BIN_DIR) ] ; then mkdir -p $(OUT_BIN_DIR) ; fi

# This generates a dependency file for each .cpp file and includes it in the
# current makefile.
# $(DEPS_DIR) must exists before you run make
include $(foreach root,$(sources:.cpp=.d),$(DEPS_DIR)/$(notdir $(root)))

