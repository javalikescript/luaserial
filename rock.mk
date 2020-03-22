## Makefile for rockspec
# luarocks MAKE=make CC=gcc LD=gcc make --lua-dir ../../luaclibs/lua/src

CC ?= gcc

PLAT ?= windows
LIBNAME = serial

#LUA_APP = $(LUA_BINDIR)/$(LUA)
LUA_APP = $(LUA)
LUA_VERSION = $(shell $(LUA_APP) -e "print(string.sub(_VERSION, 5))")
LUA_LIBNAME = lua$(subst .,,$(LUA_VERSION))

CFLAGS_windows = -Wall \
  -Wextra \
  -Wno-unused-parameter \
  -Wstrict-prototypes \
  -I$(LUA_INCDIR) \

LIBFLAG_windows = -O \
  -shared \
  -Wl,-s \
  -L$(LUA_LIBDIR) -l$(LUA_LIBNAME)

TARGET_windows = $(LIBNAME).dll

CFLAGS_linux = -pedantic  \
  -Wall \
  -Wextra \
  -Wno-unused-parameter \
  -Wstrict-prototypes \
  -I$(LUA_INCDIR)

LIBFLAG_linux= -static-libgcc \
  -Wl,-s \
  -L$(LUA_LIBDIR)

TARGET_linux = $(LIBNAME).so


TARGET = $(TARGET_$(PLAT))

SOURCES = luaserial.c

OBJS = luaserial.o

lib: $(TARGET)

install:
	cp $(TARGET) $(INST_LIBDIR)

show:
	@echo PLAT: $(PLAT)
	@echo LUA_VERSION: $(LUA_VERSION)
	@echo LUA_LIBNAME: $(LUA_LIBNAME)
	@echo CFLAGS: $(CFLAGS)
	@echo LIBFLAG: $(LIBFLAG)
	@echo LUA_LIBDIR: $(LUA_LIBDIR)
	@echo LUA_BINDIR: $(LUA_BINDIR)
	@echo LUA_INCDIR: $(LUA_INCDIR)
	@echo LUA: $(LUA)
	@echo LUALIB: $(LUALIB)

show-install:
	@echo PREFIX: $(PREFIX) or $(INST_PREFIX)
	@echo BINDIR: $(BINDIR) or $(INST_BINDIR)
	@echo LIBDIR: $(LIBDIR) or $(INST_LIBDIR)
	@echo LUADIR: $(LUADIR) or $(INST_LUADIR)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBFLAG) $(LIBFLAG_$(PLAT)) -o $(TARGET)

clean:
	-$(RM) $(OBJS) $(TARGET)

$(OBJS): %.o : %.c $(SOURCES)
	$(CC) $(CFLAGS) $(CFLAGS_$(PLAT)) -c -o $@ $<
