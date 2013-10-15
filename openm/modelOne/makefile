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
DEPS_DIR = $(BUILD_DIR)/modelOne/deps

LIBOPENM_A = libopenm.a
LIBSQLITE_A = libsqlite.a

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  OBJ_DIR = $(BUILD_DIR)/modelOne/debug
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/debug
  OUT_LIB_DIR = $(OUT_PREFIX)/lib/debug
else
  BD_CFLAGS = -DNDEBUG -O3
  OBJ_DIR = $(BUILD_DIR)/modelOne/release
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/release
  OUT_LIB_DIR = $(OUT_PREFIX)/lib/release
endif

ifndef OM_DB_LIB
#  OM_DB_LIB = sqlite3
  OM_DB_LIB = sqlite
endif

CXXFLAGS = -Wall -std=c++11 -D_REENTRANT -I$(INCLUDE_DIR) $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

MODEL_EXE = modelOne
MODEL_SQL_PREFIX = modelOne

#MODEL_PATH = $(CURDIR)
MODEL_CPPLIST = \
    modelOne.cpp \
    modelOne_om.cpp

sources = $(MODEL_CPPLIST)

objects = $(foreach root,$(sources:.cpp=.o),$(OBJ_DIR)/$(root))
    
# vpath %.cpp $(MODEL_PATH)
     
.PHONY : all
all: model 

.PHONY : model
model: prepare $(OUT_BIN_DIR)/$(MODEL_EXE) model_sql

$(DEPS_DIR)/%.d : %.cpp
	@set -e; rm -f $@; \
	$(CPP) -MM $(CPPFLAGS) -c $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g ' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OUT_BIN_DIR)/$(MODEL_EXE) : $(objects) $(OUT_LIB_DIR)/$(LIBOPENM_A) $(OUT_LIB_DIR)/$(LIBSQLITE_A)
	$(CXX) -L$(OUT_LIB_DIR) -lopenm -l$(OM_DB_LIB) -lstdc++ -lpthread -o $@ $(objects)

.PHONY : model_sql
model_sql:
	cp -pf $(MODEL_SQL_PREFIX)*.sql $(OUT_BIN_DIR)

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
include $(foreach root,$(sources:.cpp=.d),$(DEPS_DIR)/$(root))


