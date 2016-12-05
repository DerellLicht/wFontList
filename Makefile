USE_DEBUG = NO

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -g
LFLAGS=
else
CFLAGS=-Wall -O3
LFLAGS=-s
endif

# FIX "This app has failed to start because libgcc_s_dw2-1.dll was not found."
# CFLAGS += -static-libgcc -static-libstdc++
CFLAGS += -Wno-write-strings
CFLAGS += -Weffc++
CFLAGS += -DUNICODE -D_UNICODE

# link library files
CFLAGS += -Ider_libs
CSRC=der_libs/common_funcs.cpp \
der_libs/common_win.cpp \
der_libs/statbar.cpp \
der_libs/wthread.cpp \
der_libs/winmsgs.cpp \
der_libs/vlistview.cpp 

# add application files
CSRC+=wfontlist.cpp font_list.cpp getfontfile.cpp

OBJS = $(CSRC:.cpp=.o) rc.o

BIN=wfontlist.exe

#************************************************************
%.o: %.cpp
	g++ $(CFLAGS) -c $< -o $@

all: $(BIN)

clean:
	rm -f *.exe *.zip *.bak $(OBJS) 

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) -ic:\lint9 -i../der_libs -dUNICODE -d_UNICODE mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CSRC)"

source:
	zip -D wfontlist.zip *
	zip -r wfontlist.zip ../der_libs/*

depend:
	makedepend $(CFLAGS) $(CSRC)

#************************************************************

$(BIN): $(OBJS)
	g++ $(CFLAGS) -mwindows -s $(OBJS) -o $@ -lcomctl32
#	cmd /C "\\InnoSetup5\iscc" /Q wFontList.iss

rc.o: wfontlist.rc 
	windres $< -O coff -o $@

# DO NOT DELETE

der_libs/common_funcs.o: der_libs/common.h
der_libs/common_win.o: der_libs/common.h der_libs/commonw.h
der_libs/statbar.o: der_libs/common.h der_libs/commonw.h der_libs/statbar.h
der_libs/wthread.o: der_libs/wthread.h
der_libs/vlistview.o: der_libs/common.h der_libs/commonw.h
der_libs/vlistview.o: der_libs/vlistview.h
wfontlist.o: resource.h der_libs/common.h der_libs/commonw.h
wfontlist.o: der_libs/statbar.h der_libs/vlistview.h font_list.h
font_list.o: der_libs/common.h der_libs/commonw.h der_libs/vlistview.h
font_list.o: font_list.h
getfontfile.o: der_libs/common.h der_libs/commonw.h
