CXX = g++
CC = gcc
CPP = $(CC)
AR = ar
ifndef BISON
  BISON = bison
endif
ifndef FLEX
  FLEX = flex
endif

BUILD_DIR = ../build
OUT_PREFIX = ..

INCLUDE_DIR = ../include
DEPS_DIR = $(BUILD_DIR)/omc/deps
BUILD_OMC_DIR = $(BUILD_DIR)/omc/src
LIB_OMC_A = libopenm_omc_db.a

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  OBJ_DIR = $(BUILD_DIR)/omc/debug
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/debug
  OUT_LIB_DIR = $(OUT_PREFIX)/lib/debug
else
  BD_CFLAGS = -DNDEBUG -O3
  OBJ_DIR = $(BUILD_DIR)/omc/release
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/release
  OUT_LIB_DIR = $(OUT_PREFIX)/lib/release
endif

CXXFLAGS = -Wall -std=c++11 -D_REENTRANT -I$(INCLUDE_DIR) -I$(BUILD_OMC_DIR) -I. $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

OMC_EXE = omc

OMC_CPPLIST = \
    omc.cpp \
	AgentDataMemberSymbol.cpp \
	AgentEventSymbol.cpp \
	AgentFuncSymbol.cpp \
	AgentInternalSymbol.cpp \
	AgentMemberSymbol.cpp \
	AgentSymbol.cpp \
	AgentVarSymbol.cpp \
	BoolSymbol.cpp \
	BuiltinAgentVarSymbol.cpp \
	ClassificationSymbol.cpp \
	CodeBlock.cpp \
	CodeGen.cpp \
	ConditionedDurationAgentVarSymbol.cpp \
	Driver.cpp \
	DurationAgentVarSymbol.cpp \
	EnumerationSymbol.cpp \
	EnumeratorSymbol.cpp \
	HeaderCheck.cpp \
	IdentityAgentVarSymbol.cpp \
	LanguageSymbol.cpp \
	LinkAgentVarSymbol.cpp \
	ModelSymbol.cpp \
	ModelTypeSymbol.cpp \
	NumericSymbol.cpp \
	ParameterSymbol.cpp \
	PartitionSymbol.cpp \
	RangeSymbol.cpp \
	RealSymbol.cpp \
	SimpleAgentVarSymbol.cpp \
	Symbol.cpp \
	TableAccumulatorSymbol.cpp \
	TableAnalysisAgentVarSymbol.cpp \
	TableExpressionSymbol.cpp \
	TableSymbol.cpp \
	TimeSymbol.cpp \
	TypeOfLinkSymbol.cpp \
	TypeSymbol.cpp \
	VersionSymbol.cpp \
    $(BUILD_OMC_DIR)/parser.cpp \
    $(BUILD_OMC_DIR)/scanner.cpp 

sources = $(OMC_CPPLIST)

objects = $(foreach root,$(sources:.cpp=.o),$(OBJ_DIR)/$(notdir $(root)))
    
vpath %.cpp $(CURDIR)
vpath %.cpp $(BUILD_OMC_DIR)
     
.PHONY : all
all: omc 

.PHONY : omc
omc: prepare $(OUT_BIN_DIR)/$(OMC_EXE)

$(BUILD_OMC_DIR)/parser.cpp : parser.y
	$(BISON) -b parser -o $@ $<

$(BUILD_OMC_DIR)/scanner.cpp : scanner.l parser.cpp
	$(FLEX) -Cem -o $@ scanner.l

# $(BUILD_OMC_DIR)/scanner.cpp : scanner.l
#	$(FLEX) -Cem -o $@ $<

$(DEPS_DIR)/%.d : %.cpp
	@set -e; rm -f $@; \
	$(CPP) -MM $(CPPFLAGS) -c $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g ' < $@.$$$$ > $(DEPS_DIR)/$(@F); \
	rm -f $@.$$$$

$(DEPS_DIR)/parser.d : $(BUILD_OMC_DIR)/parser.cpp
	@set -e; rm -f $@; \
	$(CPP) -MM $(CPPFLAGS) -c $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g ' < $@.$$$$ > $(DEPS_DIR)/$(@F); \
	rm -f $@.$$$$

$(DEPS_DIR)/scanner.d : $(BUILD_OMC_DIR)/scanner.cpp
	@set -e; rm -f $@; \
	$(CPP) -MM $(CPPFLAGS) -c $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g ' < $@.$$$$ > $(DEPS_DIR)/$(@F); \
	rm -f $@.$$$$

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OUT_BIN_DIR)/$(OMC_EXE) : $(objects) $(OUT_LIB_DIR)/$(LIB_OMC_A)
	$(CXX) -L$(OUT_LIB_DIR) -lstdc++ -lpthread -o $@ $(objects) -lopenm_omc_db

.PHONY: clean
clean:
	rm -f $(OUT_BIN_DIR)/$(OMC_EXE)
	rm -f $(OBJ_DIR)/*.o

.PHONY: cleanall
cleanall: clean
	rm -f $(BUILD_OMC_DIR)/*
	rm -f $(DEPS_DIR)/*
	rm -f $(OBJ_DIR)/*
	rm -rf $(OUT_BIN_DIR)

.PHONY: prepare
prepare:
	@if [ ! -d $(BUILD_OMC_DIR) ] ; then mkdir -p $(BUILD_OMC_DIR) ; fi
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_BIN_DIR) ] ; then mkdir -p $(OUT_BIN_DIR) ; fi

# This generates a dependency file for each .cpp file and includes it in the
# current makefile.
# $(DEPS_DIR) must exists before you run make
include $(foreach root,$(sources:.cpp=.d),$(DEPS_DIR)/$(notdir $(root)))


