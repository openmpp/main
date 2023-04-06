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
  BUILD_DIR = ../../build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ../..
endif

BUILD_PROJ_DIR = $(BUILD_DIR)/libopenm$(MSG_POSTFIX)
DEPS_DIR = $(BUILD_PROJ_DIR)/deps

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG -Og
  ifeq ($(PLATFORM_UNAME), Linux)
    BD_CFLAGS = -g -D_DEBUG -O0
  endif
  OUT_DIR = $(OUT_PREFIX)/lib
  OBJ_DIR = $(BUILD_PROJ_DIR)/debug
  BIN_POSTFIX = D
else
  BD_CFLAGS = -DNDEBUG -O3
  OUT_DIR = $(OUT_PREFIX)/lib
  OBJ_DIR = $(BUILD_PROJ_DIR)/release
  BIN_POSTFIX =
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

# address sanitizer
CC_ASAN_FLAGS =
ifdef USE_ASAN
  CC_ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
endif

# recognize dependency files
SUFFIXES += .d

CPPFLAGS = -Wall -pthread -fPIC -D$(OM_DB_DEF) -D$(OM_MSG_DEF) -D$(OM_UCVT_DEF) \
  -I$(INCLUDE_G_DIR) -I$(INCLUDE_L_DIR) $(OM_DB_CFLAGS) $(CC_ASAN_FLAGS) $(BD_CFLAGS)

CFLAGS = $(CPPFLAGS)
CXXFLAGS = -std=c++17 $(CPPFLAGS)

LIBOPENM_A = libopenm$(BIN_POSTFIX)$(MSG_POSTFIX).a

LIB_OMC_NAME = libopenm_omc_db$(BIN_POSTFIX)
LIB_OMC_A = $(LIB_OMC_NAME).a

LIBOPENM_CPPLIST = \
  main.cpp \
  common/argReader.cpp \
  common/crc32.cpp \
  common/file.cpp \
  common/helper.cpp \
  common/iniReader.cpp \
  common/log.cpp \
  common/md5.cpp \
  common/os.cpp \
  common/splitCsv.cpp \
  common/utf8Convert.cpp \
  db/dbExec.cpp \
  db/dbExecBase.cpp \
  db/dbExecProvider.cpp \
  db/dbExecSqlite.cpp \
  db/dbMetaRow.cpp \
  db/dbValue.cpp \
  db/groupLstTable.cpp \
  db/groupPcTable.cpp \
  db/groupTxtTable.cpp \
  db/entityAttrTable.cpp \
  db/entityAttrTxtTable.cpp \
  db/entityDicTable.cpp \
  db/entityDicTxtTable.cpp \
  db/langLstTable.cpp \
  db/langWordTable.cpp \
  db/modelDicTable.cpp \
  db/modelDicTxtTable.cpp \
  db/modelWordTable.cpp \
  db/outputTableReader.cpp \
  db/outputTableWriter.cpp \
  db/paramDicTable.cpp \
  db/paramDicTxtTable.cpp \
  db/paramDimsTable.cpp \
  db/paramDimsTxtTable.cpp \
  db/paramImportTable.cpp \
  db/parameterReader.cpp \
  db/parameterWriter.cpp \
  db/paramImportTable.cpp \
  db/profileLstTable.cpp \
  db/profileOptionTable.cpp \
  db/runLstTable.cpp \
  db/runOptionTable.cpp \
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
  model/childController.cpp \
  model/metaHolder.cpp \
  model/metaLoader.cpp \
  model/modelBase.cpp \
  model/modelHelper.cpp \
  model/modelRunState.cpp \
  model/restartController.cpp \
  model/rootController.cpp \
  model/runController.cpp \
  model/runControllerMicrodata.cpp \
  model/runControllerNewRun.cpp \
  model/runControllerParams.cpp \
  model/singleController.cpp \
  msg/msgCommon.cpp \
  msg/msgExecBase.cpp \
  msg/msgMpiExec.cpp \
  msg/msgMpiMetaPacked.cpp \
  msg/msgMpiPacked.cpp \
  msg/msgMpiRecv.cpp \
  msg/msgMpiSend.cpp 

LIB_OMC_CPPLIST = \
  common/argReader.cpp \
  common/crc32.cpp \
  common/file.cpp \
  common/helper.cpp \
  common/iniReader.cpp \
  common/log.cpp \
  common/md5.cpp \
  common/os.cpp \
  common/splitCsv.cpp \
  common/utf8Convert.cpp \
  db/dbExec.cpp \
  db/dbExecBase.cpp \
  db/dbExecProvider.cpp \
  db/dbExecSqlite.cpp \
  db/dbMetaRow.cpp \
  db/dbValue.cpp \
  db/entityAttrTable.cpp \
  db/entityAttrTxtTable.cpp \
  db/entityDicTable.cpp \
  db/entityDicTxtTable.cpp \
  db/groupLstTable.cpp \
  db/groupPcTable.cpp \
  db/groupTxtTable.cpp \
  db/langLstTable.cpp \
  db/langWordTable.cpp \
  db/modelDicTable.cpp \
  db/modelDicTxtTable.cpp \
  db/modelWordTable.cpp \
  db/outputTableWriter.cpp \
  db/paramDicTable.cpp \
  db/paramDicTxtTable.cpp \
  db/paramDimsTable.cpp \
  db/paramDimsTxtTable.cpp \
  db/paramImportTable.cpp \
  db/parameterReader.cpp \
  db/parameterWriter.cpp \
  db/profileLstTable.cpp \
  db/profileOptionTable.cpp \
  db/runLstTable.cpp \
  db/runOptionTable.cpp \
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
  db/omc/modelExpressionSql.cpp \
  db/omc/modelInsertSql.cpp \
  db/omc/modelSqlBuilder.cpp \
  db/omc/modelSqlBuilderPrepare.cpp \
  db/omc/modelSqlWriter.cpp

RT_OBJS := $(foreach root,$(LIBOPENM_CPPLIST:.cpp=.o),$(OBJ_DIR)/$(notdir $(root)))
RT_DEPS := $(foreach root,$(LIBOPENM_CPPLIST:.cpp=.d),$(DEPS_DIR)/$(notdir $(root)))

OMC_OBJS := $(foreach root,$(LIB_OMC_CPPLIST:.cpp=.o),$(OBJ_DIR)/$(notdir $(root)))
OMC_DEPS := $(foreach root,$(LIB_OMC_CPPLIST:.cpp=.d),$(DEPS_DIR)/$(notdir $(root)))

vpath %.cpp $(CURDIR)
vpath %.cpp $(CURDIR)/common
vpath %.cpp $(CURDIR)/db
vpath %.cpp $(CURDIR)/db/omc
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

$(OBJ_DIR)/xz_crc64.o : common/xz_crc64.c $(INCLUDE_G_DIR)/libopenm/common/xz_crc64.h
	$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(@F)
	
$(OUT_DIR)/$(LIBOPENM_A) : $(RT_OBJS) $(OBJ_DIR)/xz_crc64.o
	$(AR) rcs $@ $^

$(OUT_DIR)/$(LIB_OMC_A) : $(OMC_OBJS)
	$(AR) rcs $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d

.PHONY: clean-all
clean-all: clean
	rm -f $(OUT_DIR)/$(LIBOPENM_A)
	rm -f $(OUT_DIR)/$(LIB_OMC_A)
	rm -rf $(BUILD_PROJ_DIR)

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_DIR) ] ; then mkdir -p $(OUT_DIR) ; fi

# include dependencies for each .cpp file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean clean-all prepare)))
    -include $(RT_DEPS)
    -include $(OMC_DEPS)
endif

