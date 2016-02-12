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

ifndef BUILD_DIR
  BUILD_DIR = ../../build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ../..
endif

BUILD_PROJ_DIR = $(BUILD_DIR)/omc
DEPS_DIR = $(BUILD_PROJ_DIR)/deps
BUILD_SRC_DIR = $(BUILD_PROJ_DIR)/src

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  OUT_BIN_DIR = $(OUT_PREFIX)/bin
  OUT_LIB_DIR = $(OUT_PREFIX)/lib
  OBJ_DIR = $(BUILD_PROJ_DIR)/debug
  BIN_POSTFIX = D
else
  BD_CFLAGS = -DNDEBUG -O3
  OUT_BIN_DIR = $(OUT_PREFIX)/bin
  OUT_LIB_DIR = $(OUT_PREFIX)/lib
  OBJ_DIR = $(BUILD_PROJ_DIR)/release
  BIN_POSTFIX = 
endif

INCLUDE_DIR = ../../include
LIB_OMC_A = libopenm_omc_db$(BIN_POSTFIX).a

# recognize dependency files
SUFFIXES += .d

CXXFLAGS = -Wall -std=c++11 -D_REENTRANT -fdiagnostics-color=auto -I$(INCLUDE_DIR) -I$(BUILD_SRC_DIR) -I. $(BD_CFLAGS)
CPPFLAGS = $(CXXFLAGS)

OMC_EXE = omc$(BIN_POSTFIX)

OMC_CPPLIST = \
AttributeSymbol.cpp \
AggregationSymbol.cpp \
BoolSymbol.cpp \
BuiltinAttributeSymbol.cpp \
ClassificationSymbol.cpp \
CodeBlock.cpp \
CodeGen.cpp \
Constant.cpp \
ConstantSymbol.cpp \
DependencyGroupSymbol.cpp \
DerivedAttributeSymbol.cpp \
DerivedTableSymbol.cpp \
DimensionSymbol.cpp \
Driver.cpp \
EntityArrayMemberSymbol.cpp \
EntityDataMemberSymbol.cpp \
EntityEventSymbol.cpp \
EntityFuncSymbol.cpp \
EntityHookSymbol.cpp \
EntityIncrementSymbol.cpp \
EntityInternalSymbol.cpp \
EntityMemberSymbol.cpp \
EntityMultilinkSymbol.cpp \
EntitySetSymbol.cpp \
EntitySymbol.cpp \
EntityTableAccumulatorSymbol.cpp \
EntityTableMeasureAttributeSymbol.cpp \
EntityTableMeasureSymbol.cpp \
EntityTableSymbol.cpp \
EnumerationSymbol.cpp \
EnumerationWithEnumeratorsSymbol.cpp \
EnumeratorSymbol.cpp \
GlobalFuncSymbol.cpp \
GroupSymbol.cpp \
HeaderCheck.cpp \
HideGroupSymbol.cpp \
IdentityAttributeSymbol.cpp \
LanguageSymbol.cpp \
LinkAttributeSymbol.cpp \
LinkToAttributeSymbol.cpp \
md5.cpp \
ModelSymbol.cpp \
ModelTypeSymbol.cpp \
MultilinkAttributeSymbol.cpp \
NumericSymbol.cpp \
omc.cpp \
ParameterGroupSymbol.cpp \
ParameterSymbol.cpp \
PartitionSymbol.cpp \
RangeSymbol.cpp \
RealSymbol.cpp \
SimpleAttributeEnumSymbol.cpp \
SimpleAttributeSymbol.cpp \
StringSymbol.cpp \
StringTypeSymbol.cpp \
Symbol.cpp \
TableGroupSymbol.cpp \
TableMeasureSymbol.cpp \
TableSymbol.cpp \
TimeSymbol.cpp \
TypeOfLinkSymbol.cpp \
TypeSymbol.cpp \
UnknownTypeSymbol.cpp \
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
	$(CXX) -L$(OUT_LIB_DIR) -lstdc++ -lpthread -o $@ $(OBJS) -lopenm_omc_db$(BIN_POSTFIX)

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d
	rm -f $(BUILD_SRC_DIR)/*

.PHONY: cleanall
cleanall: clean
	rm -f $(OUT_BIN_DIR)/$(OMC_EXE)
	rm -rf $(BUILD_PROJ_DIR)
	@if [ -d $(OUT_BIN_DIR) ] ; then rmdir --ignore-fail-on-non-empty $(OUT_BIN_DIR) ; fi 

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
