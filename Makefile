# SHELL=cmd.exe
USE_DEBUG = NO
USE_UNICODE = YES
USE_CLANG = YES

include der_libs\tool_select.mak

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -g -c
LFLAGS= -mwindows
else
CFLAGS=-Wall -O3 -c
LFLAGS=-s -mwindows
endif

ifeq ($(USE_UNICODE),YES)
CFLAGS += -DUNICODE -D_UNICODE
endif

CFLAGS += -Wno-write-strings
CFLAGS += -Weffc++

# this flag resolves errors from WM_NOTIFY messages, 
# which are signed-int values camouflaged as uint32_t values
ifeq ($(USE_CLANG),YES)
CFLAGS += -Wno-c++11-narrowing
endif

ifeq ($(USE_STATIC),YES)
LFLAGS += -static
endif

# This is required for *some* versions of makedepend
IFLAGS += -DNOMAKEDEPEND

# link library files
CFLAGS += -Ider_libs

# add application files
CAPPSRC=wfontlist.cpp font_list.cpp getfontfile.cpp config.cpp

CLIBSRC=der_libs/common_funcs.cpp \
der_libs/common_win.cpp \
der_libs/statbar.cpp \
der_libs/wthread.cpp \
der_libs/winmsgs.cpp \
der_libs/vlistview.cpp 

CSRC = $(CAPPSRC) $(CLIBSRC)

OBJS = $(CSRC:.cpp=.o) rc.o

BASE=wfontlist
BINS=$(BASE).exe

LIBS = -lcomctl32

# Automatically parse the latest version block
VERSION := $(shell grep -oE '\[[0-9]+\.[0-9]+\]' CHANGELOG.md | head -n 1 | tr -d '[]')
DIST_ZIP := $(BASE)V$(VERSION).zip
#************************************************************
%.o: %.cpp
	$(TOOLS)/$(GNAME) $(CFLAGS) $< -o $@

all: $(BINS)

clean:
	rm -f *.exe *.zip *.bak $(OBJS) 

wc:
	wc -l $(CSRC) *.rc
	
ctidy_all:
	cmd /C "clang-tidy $(CSRC) -- $(CFLAGS) 2>&1 | grep -oP '\[\K[a-z][a-z0-9-]+(?=\]$$)' | sort | uniq -c | sort -rn"

ctidy_local:
	cmd /C "clang-tidy $(CAPPSRC) -- $(CFLAGS) 2>&1 | grep -oP '\[\K[a-z][a-z0-9-]+(?=\]$$)' | sort | uniq -c | sort -rn"

ctidy_libs:
	cmd /C "clang-tidy $(CLIBSRC) -- $(CFLAGS) 2>&1 | grep -oP '\[\K[a-z][a-z0-9-]+(?=\]$$)' | sort | uniq -c | sort -rn"

clint:
	cmd /C "python ..\ClaudeLint.py --exclude der_libs"
	
cppc:
	cmd /C "cppcheck --project=compile_commands.json --std=c++14 --suppressions-list=./.suppress.cppcheck"

check:
	cmd /C "d:\llvm\bin\clang-tidy.exe $(CSRC) -- $(CFLAGS) "

# Your new automated release workflow
release:
	cmd /C "@echo Preparing GitHub release for v$(VERSION)..."
	sed -n '/## \['$(VERSION)'\]/,/## \[/p' CHANGELOG.md | sed '$$d' > temp_notes.md
	gh release create v$(VERSION) ./$(DIST_ZIP) ./CHANGELOG.md --notes-file temp_notes.md
	rm temp_notes.md
	cmd /C "@echo Release v$(VERSION) successfully uploaded to GitHub!"
	
dist:
	rm -f *.zip
	zip $(DIST_ZIP) $(BINS) readme.md CHANGELOG.md

depend:
	makedepend $(CFLAGS) $(CSRC)

#************************************************************

$(BINS): $(OBJS)
	$(TOOLS)/$(GNAME) $(OBJS) $(LFLAGS) -o $(BINS) $(LIBS) 

rc.o: wfontlist.rc 
	$(TOOLS)\$(WRNAME) $< -O COFF -o $@

# DO NOT DELETE

wfontlist.o: resource.h der_libs/common.h der_libs/commonw.h wfontlist.h
wfontlist.o: der_libs/statbar.h der_libs/vlistview.h font_list.h
font_list.o: der_libs/common.h der_libs/commonw.h der_libs/vlistview.h
font_list.o: font_list.h
getfontfile.o: der_libs/common.h der_libs/commonw.h
config.o: der_libs/common.h wfontlist.h
der_libs/common_funcs.o: der_libs/common.h
der_libs/common_win.o: der_libs/common.h der_libs/commonw.h
der_libs/statbar.o: der_libs/common.h der_libs/commonw.h der_libs/statbar.h
der_libs/wthread.o: der_libs/wthread.h
der_libs/vlistview.o: der_libs/common.h der_libs/commonw.h
der_libs/vlistview.o: der_libs/vlistview.h
