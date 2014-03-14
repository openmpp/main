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

ifndef BUILD_DIR
  BUILD_DIR = ../build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ..
endif

ifndef OM_INC_DIR
  OM_INC_DIR = ../../include
endif

ifndef OM_BIN_DIR
  OM_BIN_DIR = ../../bin
endif

ifndef OM_LIB_DIR
  OM_LIB_DIR = ../../lib
endif

MODEL_BUILD_DIR = $(BUILD_DIR)/modelOne
DEPS_DIR = $(MODEL_BUILD_DIR)/deps

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  OBJ_DIR = $(MODEL_BUILD_DIR)/debug
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/debug
else
  BD_CFLAGS = -DNDEBUG -O3
  OBJ_DIR = $(MODEL_BUILD_DIR)/release
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/release
endif

ifndef OM_DB_LIB
#  OM_DB_LIB = sqlite3
  OM_DB_LIB = sqlite
endif

LIBOPENM_A = libopenm.a
LIBSQLITE_A = libsqlite.a

# recognize dependency files
SUFFIXES += .d

CXXFLAGS = -Wall -std=c++11 -D_REENTRANT -I$(OM_INC_DIR) $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

MODEL_EXE = modelOne
MODEL_SQL_PREFIX = modelOne

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
model: prepare $(OUT_BIN_DIR)/$(MODEL_EXE) model_sql

$(OBJS): | prepare
$(DEPS): | prepare

$(DEPS_DIR)/%.d : %.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OUT_BIN_DIR)/$(MODEL_EXE) : $(OBJS) $(OM_LIB_DIR)/$(LIBOPENM_A) $(OM_LIB_DIR)/$(LIBSQLITE_A)
	$(CXX) -L$(OM_LIB_DIR) -lopenm -l$(OM_DB_LIB) -lstdc++ -lpthread -o $@ $(OBJS)

.PHONY : model_sql
model_sql:
	cp -pf $(MODEL_SQL_PREFIX)*.sql $(OUT_BIN_DIR)

.PHONY: clean
clean:
	rm -f $(OUT_BIN_DIR)/$(MODEL_EXE)
	rm -f $(OUT_BIN_DIR)/$(MODEL_SQL_PREFIX)*.sql
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d

.PHONY: cleanall
cleanall: clean
	rm -rf $(MODEL_BUILD_DIR)

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

