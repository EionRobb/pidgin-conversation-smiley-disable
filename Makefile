
PIDGIN_TREE_TOP ?= ../pidgin-2.10.11
PIDGIN3_TREE_TOP ?= ../pidgin-main
LIBPURPLE_DIR ?= $(PIDGIN_TREE_TOP)/libpurple
PIDGIN_DIR ?= $(PIDGIN_TREE_TOP)/pidgin
WIN32_DEV_TOP ?= $(PIDGIN_TREE_TOP)/../win32-dev

WIN32_CC ?= $(WIN32_DEV_TOP)/mingw-4.7.2/bin/gcc

PKG_CONFIG ?= pkg-config
DIR_PERM = 0755
LIB_PERM = 0755
FILE_PERM = 0644
MAKENSIS ?= makensis
XGETTEXT ?= xgettext

CFLAGS	?= -O2 -g -pipe
LDFLAGS ?= 

# Do some nasty OS and purple version detection
ifeq ($(OS),Windows_NT)
  #only defined on 64-bit windows
  PROGFILES32 = ${ProgramFiles(x86)}
  ifndef PROGFILES32
    PROGFILES32 = $(PROGRAMFILES)
  endif
  TEAMS_TARGET = pidgin-conversation-smiley-disabled.dll
  TEAMS_DEST = "$(PROGFILES32)/Pidgin/plugins"
else

  UNAME_S := $(shell uname -s)

  #.. There are special flags we need for OSX
  ifeq ($(UNAME_S), Darwin)
    #
    #.. /opt/local/include and subdirs are included here to ensure this compiles
    #   for folks using Macports.  I believe Homebrew uses /usr/local/include
    #   so things should "just work".  You *must* make sure your packages are
    #   all up to date or you will most likely get compilation errors.
    #
    INCLUDES = -I/opt/local/include -lz $(OS)

    CC = gcc
  else
    INCLUDES = 
    CC ?= gcc
  endif

	ifeq ($(shell $(PKG_CONFIG) --exists purple 2>/dev/null && echo "true"),)
		TEAMS_TARGET = FAILNOPURPLE
		TEAMS_DEST =
	else
		TEAMS_TARGET = pidgin-conversation-smiley-disabled.so
		TEAMS_DEST = $(DESTDIR)`$(PKG_CONFIG) --variable=plugindir purple`
	endif
endif

WIN32_CFLAGS = -I$(WIN32_DEV_TOP)/gtk_2_0-2.14/include -I$(WIN32_DEV_TOP)/gtk_2_0-2.14/include/glib-2.0 -I$(WIN32_DEV_TOP)/gtk_2_0-2.14/lib/glib-2.0/include -I$(WIN32_DEV_TOP)/gtk_2_0-2.14/include/gtk-2.0 -I$(WIN32_DEV_TOP)/gtk_2_0-2.14/include/cairo -I$(WIN32_DEV_TOP)/gtk_2_0-2.14/include/pango-1.0 -I$(WIN32_DEV_TOP)/gtk_2_0-2.14/include/atk-1.0 -I$(WIN32_DEV_TOP)/gtk_2_0-2.14/lib/gtk-2.0/include -DENABLE_NLS -Wall -Wextra -Werror -Wno-deprecated-declarations -Wno-unused-parameter -fno-strict-aliasing -Wformat -Wno-sign-compare
WIN32_LDFLAGS = -L$(WIN32_DEV_TOP)/gtk_2_0-2.14/lib -lpidgin -lpurple -lintl -lglib-2.0 -lgobject-2.0 -lgtk-win32-2.0 -g -ggdb -static-libgcc -lz
WIN32_PIDGIN2_CFLAGS = -I$(PIDGIN_TREE_TOP)/libpurple -I$(PIDGIN_TREE_TOP)/pidgin -I$(PIDGIN_TREE_TOP)/pidgin/win32 -I$(PIDGIN_TREE_TOP) $(WIN32_CFLAGS)
WIN32_PIDGIN2_LDFLAGS = -L$(PIDGIN_TREE_TOP)/libpurple -L$(PIDGIN_TREE_TOP)/pidgin $(WIN32_LDFLAGS)

PURPLE_C_FILES := pidgin-conversation-smiley-disabled.c



.PHONY:	all install FAILNOPURPLE clean translations

all: $(TEAMS_TARGET)

pidgin-conversation-smiley-disabled.so: $(PURPLE_C_FILES) $(PURPLE_COMPAT_FILES)
	$(CC) -fPIC $(CFLAGS) -shared -o $@ $^ $(LDFLAGS) `$(PKG_CONFIG) pidgin glib-2.0 gtk+-2.0 zlib --libs --cflags`  $(INCLUDES) -g -ggdb

pidgin-conversation-smiley-disabled.dll: $(PURPLE_C_FILES) $(PURPLE_COMPAT_FILES)
	$(WIN32_CC) -shared -o $@ $^ $(WIN32_PIDGIN2_CFLAGS) $(WIN32_PIDGIN2_LDFLAGS)

install: $(TEAMS_TARGET)
	mkdir -m $(DIR_PERM) -p $(TEAMS_DEST)
	install -m $(LIB_PERM) -p $(TEAMS_TARGET) $(TEAMS_DEST)

translations: po/pidgin-conversation-smiley-disabled.pot

po/pidgin-conversation-smiley-disabled.pot: $(PURPLE_C_FILES)
	$(XGETTEXT) $^ -k_ --no-location -o $@

po/%.po: po/pidgin-conversation-smiley-disabled.pot
	msgmerge $@ po/pidgin-conversation-smiley-disabled.pot > tmp-$*
	mv -f tmp-$* $@

po/%.mo: po/%.po
	msgfmt -o $@ $^

%-locale-install: po/%.mo
	install -D -m $(FILE_PERM) -p po/$(*F).mo $(LOCALEDIR)/$(*F)/LC_MESSAGES/pidgin-conversation-smiley-disabled.mo
	
FAILNOPURPLE:
	echo "You need libpurple development headers installed to be able to compile this plugin"

clean:
	rm -f $(TEAMS_TARGET)

