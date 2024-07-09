# gamepad-viewer/Makefile

all: gamepad-viewer.exe


CXX := g++

CXXFLAGS :=
CXXFLAGS += -g
CXXFLAGS += -Wall
CXXFLAGS += -Werror
CXXFLAGS += -DUNICODE

LDFLAGS :=
LDFLAGS += -g
LDFLAGS += -Wall
LDFLAGS += -municode

LIBS :=

# Direct2D.  For me, the actual file is:
# /d/opt/winlibs-mingw64-13.2/x86_64-w64-mingw32/lib/libd2d1.a.
LIBS += -ld2d1

# DirectWrite.  For me the file is libdwrite.a next to libd2d1.a.
LIBS += -ldwrite


OBJS :=
OBJS += base-window.o
OBJS += gamepad-viewer.o
OBJS += winapi-util.o


%.o: %.cc
	$(CXX) -c -o $@ $(CXXFLAGS) $<

gamepad-viewer.exe: $(OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)


.PHONY: clean
clean:
	$(RM) *.o *.exe


# EOF
