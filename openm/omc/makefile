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

# recognize dependency files
SUFFIXES += .d

BUILD_DIR = ../build
OUT_PREFIX = ..

INCLUDE_DIR = ../include
OMC_BUILD_DIR = $(BUILD_DIR)/omc
DEPS_DIR = $(OMC_BUILD_DIR)/deps
BUILD_SRC_DIR = $(OMC_BUILD_DIR)/src
LIB_OMC_A = libopenm_omc_db.a

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  OBJ_DIR = $(OMC_BUILD_DIR)/debug
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/debug
  OUT_LIB_DIR = $(OUT_PREFIX)/lib/debug
else
  BD_CFLAGS = -DNDEBUG -O3
  OBJ_DIR = $(OMC_BUILD_DIR)/release
  OUT_BIN_DIR = $(OUT_PREFIX)/bin/release
  OUT_LIB_DIR = $(OUT_PREFIX)/lib/release
endif

CXXFLAGS = -Wall -std=c++11 -D_REENTRANT -I$(INCLUDE_DIR) -I$(BUILD_SRC_DIR) -I. $(BD_CFLAGS)
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
LinkToAgentVarSymbol.cpp \
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
    $(BUILD_SRC_DIR)/parser.cpp \
    $(BUILD_SRC_DIR)/scanner.cpp 

sources = $(OMC_CPPLIST)

OBJS := $(foreach root,$(sources:.cpp=.o),$(OBJ_DIR)/$(notdir $(root)))
DEPS := $(foreach root,$(sources:.cpp=.d),$(DEPS_DIR)/$(notdir $(root)))

vpath %.cpp $(CURDIR)
vpath %.cpp $(BUILD_SRC_DIR)
     
.PHONY : all
all: omc 

.PHONY : omc
omc: prepare $(OUT_BIN_DIR)/$(OMC_EXE)

$(OBJS): | prepare
$(DEPS): | prepare
$(BUILD_SRC_DIR)/parser.cpp: | prepare

$(BUILD_SRC_DIR)/parser.cpp : parser.y
	$(BISON) -b parser -o $@ $<

$(BUILD_SRC_DIR)/scanner.cpp : scanner.l $(BUILD_SRC_DIR)/parser.cpp
	$(FLEX) -Cem -o $@ scanner.l

# $(BUILD_SRC_DIR)/scanner.cpp : scanner.l
#	$(FLEX) -Cem -o $@ $<

$(DEPS_DIR)/%.d : %.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(DEPS_DIR)/parser.d : $(BUILD_SRC_DIR)/parser.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(DEPS_DIR)/scanner.d : $(BUILD_SRC_DIR)/scanner.cpp
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OUT_BIN_DIR)/$(OMC_EXE) : $(OBJS) $(OUT_LIB_DIR)/$(LIB_OMC_A)
	$(CXX) -L$(OUT_LIB_DIR) -lstdc++ -lpthread -o $@ $(OBJS) -lopenm_omc_db

.PHONY: clean
clean:
	rm -f $(OUT_BIN_DIR)/$(OMC_EXE)
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d

.PHONY: cleanall
cleanall: clean
	rm -rf $(OMC_BUILD_DIR)

.PHONY: prepare
prepare:
	@if [ ! -d $(BUILD_SRC_DIR) ] ; then mkdir -p $(BUILD_SRC_DIR) ; fi
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_BIN_DIR) ] ; then mkdir -p $(OUT_BIN_DIR) ; fi

# include dependencies for each .cpp file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean cleanall prepare)))
    -include $(DEPS)
endif
