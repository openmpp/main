CXX = g++
CC = gcc
CPP = $(CC)
AR = ar

# recognize dependency files
SUFFIXES += .d

BUILD_DIR = ../build
OUT_PREFIX = ..

# INCLUDE_DIR = .
DEPS_DIR = $(BUILD_DIR)/libsqlite/deps

ifndef RELEASE
  BD_CFLAGS = -g -D_DEBUG
  OBJ_DIR = $(BUILD_DIR)/libsqlite/debug
  OUT_DIR = $(OUT_PREFIX)/lib/debug
else
  BD_CFLAGS = -DNDEBUG -O3
  OBJ_DIR = $(BUILD_DIR)/libsqlite/release
  OUT_DIR = $(OUT_PREFIX)/lib/release
endif

ifndef OM_DB_DEF
  OM_DB_DEF = OM_DB_SQLITE
endif

ifndef OM_UCVT_DEF
  OM_UCVT_DEF = OM_UCVT_ICONV
endif

CXXFLAGS = -Wall -D_REENTRANT -DSQLITE_ENABLE_COLUMN_METADATA -DSQLITE_OMIT_LOAD_EXTENSION $(BD_CFLAGS)
CCFLAGS = $(CXXFLAGS)
CPPFLAGS = $(CXXFLAGS)

LIBSQLITE_NAME = libsqlite
LIBSQLITE_A = $(LIBSQLITE_NAME).a

LIBSQLITE_CLIST := \
  sqlite3.c \
  extension-functions.c

OBJS := $(foreach root,$(LIBSQLITE_CLIST:.c=.o),$(OBJ_DIR)/$(notdir $(root)))
DEPS := $(foreach root,$(LIBSQLITE_CLIST:.c=.d),$(DEPS_DIR)/$(notdir $(root)))
    
.PHONY : all
all: libsqlite

.PHONY : libsqlite
libsqlite: prepare $(OUT_DIR)/$(LIBSQLITE_A)

$(DEPS): | prepare
$(OBJS): | prepare

$(DEPS_DIR)/%.d : %.c
	$(CPP) -MM $(CPPFLAGS) $< -MF $@

$(OBJ_DIR)/%.o : %.c
	$(CC) $(CCFLAGS) -c $< -o $(OBJ_DIR)/$(@F)
	
$(OUT_DIR)/$(LIBSQLITE_A) : $(OBJS)
	$(AR) rcs $@ $^

.PHONY: clean
clean:
	rm -f $(OUT_DIR)/*.a
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d

.PHONY: cleanall
cleanall: clean
	rm -f $(DEPS_DIR)/*
	rm -f $(OBJ_DIR)/*
	rm -rf $(OUT_DIR)

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_DIR) ] ; then mkdir -p $(OUT_DIR) ; fi

# include dependencies for each .c file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean cleanall prepare)))
    -include $(DEPS)
endif

