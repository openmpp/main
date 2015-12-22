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
  BUILD_DIR = ../../build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ../..
endif

BUILD_PROJ_DIR = $(BUILD_DIR)/libopenm
DEPS_DIR = $(BUILD_PROJ_DIR)/deps

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  OUT_DIR = $(OUT_PREFIX)/lib
  OBJ_DIR = $(BUILD_PROJ_DIR)/debug
#  OUT_DIR = $(OUT_PREFIX)/lib/debug
else
  BD_CFLAGS = -DNDEBUG -O3
  OUT_DIR = $(OUT_PREFIX)/lib
  OBJ_DIR = $(BUILD_PROJ_DIR)/release
#  OUT_DIR = $(OUT_PREFIX)/lib/release
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

# recognize dependency files
SUFFIXES += .d

CXXFLAGS = -Wall -std=c++11 -D$(OM_DB_DEF) -D$(OM_MSG_DEF) -D$(OM_UCVT_DEF) -D_REENTRANT \
  -I$(INCLUDE_G_DIR) -I$(INCLUDE_L_DIR) $(OM_DB_CFLAGS) $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

LIBOPENM_NAME = libopenm
LIBOPENM_A = $(LIBOPENM_NAME).a

LIB_OMC_NAME = libopenm_omc_db
LIB_OMC_A = $(LIB_OMC_NAME).a

LIBOPENM_CPPLIST = \
  main.cpp \
  common/argReader.cpp \
  common/file.cpp \
  common/helper.cpp \
  common/iniReader.cpp \
  common/log.cpp \
  common/utf8Convert.cpp \
  db/dbExec.cpp \
  db/dbExecSqlite.cpp \
  db/dbMetaRow.cpp \
  db/groupLstTable.cpp \
  db/groupPcTable.cpp \
  db/groupTxtTable.cpp \
  db/langLstTable.cpp \
  db/langWordTable.cpp \
  db/modelAggregationSql.cpp \
  db/modelDicTable.cpp \
  db/modelDicTxtTable.cpp \
  db/outputTableWriter.cpp \
  db/paramDicTable.cpp \
  db/paramDicTxtTable.cpp \
  db/paramDimsTable.cpp \
  db/paramDimsTxtTable.cpp \
  db/parameterReader.cpp \
  db/parameterWriter.cpp \
  db/paramRunTxtTable.cpp \
  db/profileLstTable.cpp \
  db/profileOptionTable.cpp \
  db/runLstTable.cpp \
  db/runOptionTable.cpp \
  db/runTxtTable.cpp \
  db/tableAccTable.cpp \
  db/tableAccTxtTable.cpp \
  db/tableDicTable.cpp \
  db/tableDicTxtTable.cpp \
  db/tableDimsTable.cpp \
  db/tableDimsTxtTable.cpp \
  db/tableExprTable.cpp \
  db/tableExprTxtTable.cpp \
  db/taskLstTable.cpp \
  db/taskRunLstTable.cpp \
  db/taskRunSetTable.cpp \
  db/taskSetTable.cpp \
  db/taskTxtTable.cpp \
  db/typeDicTable.cpp \
  db/typeDicTxtTable.cpp \
  db/typeEnumLstTable.cpp \
  db/typeEnumTxtTable.cpp \
  db/worksetLstTable.cpp \
  db/worksetParamTable.cpp \
  db/worksetParamTxtTable.cpp \
  db/worksetTxtTable.cpp \
  model/metaLoader.cpp \
  model/modelBase.cpp \
  model/modelRunState.cpp \
  model/runController.cpp \
  model/runBase.cpp \
  msg/msgCommon.cpp \
  msg/msgExecBase.cpp \
  msg/msgMpiExec.cpp \
  msg/msgMpiMetaPacked.cpp \
  msg/msgMpiPacked.cpp \
  msg/msgMpiRecv.cpp \
  msg/msgMpiSend.cpp 

LIB_OMC_CPPLIST = \
  common/argReader.cpp \
  common/file.cpp \
  common/helper.cpp \
  common/iniReader.cpp \
  common/log.cpp \
  common/utf8Convert.cpp \
  db/dbMetaRow.cpp \
  db/modelAggregationSql.cpp \
  db/modelInsertSql.cpp \
  db/modelSqlBuilder.cpp \
  db/modelSqlWriter.cpp

RT_OBJS := $(foreach root,$(LIBOPENM_CPPLIST:.cpp=.o),$(OBJ_DIR)/$(notdir $(root)))
RT_DEPS := $(foreach root,$(LIBOPENM_CPPLIST:.cpp=.d),$(DEPS_DIR)/$(notdir $(root)))

OMC_OBJS := $(foreach root,$(LIB_OMC_CPPLIST:.cpp=.o),$(OBJ_DIR)/$(notdir $(root)))
OMC_DEPS := $(foreach root,$(LIB_OMC_CPPLIST:.cpp=.d),$(DEPS_DIR)/$(notdir $(root)))

vpath %.cpp $(CURDIR)
vpath %.cpp $(CURDIR)/common
vpath %.cpp $(CURDIR)/db
vpath %.cpp $(CURDIR)/model
vpath %.cpp $(CURDIR)/msg
     
.PHONY : all
all: libopenm lib_omc

.PHONY : libopenm
libopenm: prepare $(OUT_DIR)/$(LIBOPENM_A)

.PHONY : lib_omc
lib_omc: prepare $(OUT_DIR)/$(LIB_OMC_A)

$(RT_DEPS): | prepare
$(RT_OBJS): | prepare

$(OMC_DEPS): | prepare
$(OMC_OBJS): | prepare

$(DEPS_DIR)/%.d : %.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $(OBJ_DIR)/$(@F)
	
$(OUT_DIR)/$(LIBOPENM_A) : $(RT_OBJS)
	$(AR) rcs $@ $^

$(OUT_DIR)/$(LIB_OMC_A) : $(OMC_OBJS)
	$(AR) rcs $@ $^

.PHONY: clean
clean:
	rm -f $(OUT_DIR)/$(LIBOPENM_A)
	rm -f $(OUT_DIR)/$(LIB_OMC_A)
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d

.PHONY: cleanall
cleanall: clean
	rm -rf $(BUILD_PROJ_DIR)
	# rm -rf $(OUT_DIR)

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_DIR) ] ; then mkdir -p $(OUT_DIR) ; fi

# include dependencies for each .cpp file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean cleanall prepare)))
    -include $(RT_DEPS)
    -include $(OMC_DEPS)
endif

