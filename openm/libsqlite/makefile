# platform name: Linux or Darwin
PLATFORM_UNAME := $(shell uname -s)

CXX = g++
CC = gcc
CPP = $(CC)
AR = ar

ifndef BUILD_DIR
  BUILD_DIR = ../../build
endif

ifndef OUT_PREFIX
  OUT_PREFIX = ../..
endif

BUILD_PROJ_DIR = $(BUILD_DIR)/libsqlite
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

# address sanitizer
CC_ASAN_FLAGS =
ifdef USE_ASAN
  CC_ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
endif

# libsqlite specific
OM_DB_DEF = OM_DB_SQLITE

ifndef OM_UCVT_DEF
  OM_UCVT_DEF = OM_UCVT_ICONV
endif

# recognize dependency files
SUFFIXES += .d

CFLAGS = -Wall -pthread -fPIC -D_REENTRANT -DSQLITE_ENABLE_COLUMN_METADATA -DSQLITE_OMIT_LOAD_EXTENSION $(CC_ASAN_FLAGS) $(BD_CFLAGS)
CPPFLAGS = $(CFLAGS)

LIBSQLITE_A = libsqlite$(BIN_POSTFIX).a

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
	$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(@F)

$(OUT_DIR)/$(LIBSQLITE_A) : $(OBJS)
	$(AR) rcs $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(DEPS_DIR)/*.d

.PHONY: clean-all
clean-all: clean
	rm -f $(OUT_DIR)/$(LIBSQLITE_A)
	rm -rf $(BUILD_PROJ_DIR)
	@if [ -d $(OUT_DIR) ] ; then rmdir --ignore-fail-on-non-empty $(OUT_DIR) ; fi 

.PHONY: prepare
prepare:
	@if [ ! -d $(DEPS_DIR) ] ; then mkdir -p $(DEPS_DIR) ; fi
	@if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	@if [ ! -d $(OUT_DIR) ] ; then mkdir -p $(OUT_DIR) ; fi

# include dependencies for each .c file
# if target is not clean or prepare
ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean clean-all prepare)))
    -include $(DEPS)
endif

