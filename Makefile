# gamepad-viewer/Makefile

all: gamepad-viewer.exe


CXX := g++

CXXFLAGS :=
CXXFLAGS += -g
CXXFLAGS += -Wall
CXXFLAGS += -Werror
CXXFLAGS += -DUNICODE
CXXFLAGS += -std=c++17

# Generate .d files.
CXXFLAGS += -MMD

LDFLAGS :=
LDFLAGS += -g
LDFLAGS += -Wall
LDFLAGS += -municode

# Do not require the winlibs DLLs at runtime.
LDFLAGS += -static

LIBS :=

# Direct2D.  For me, the actual file is:
# /d/opt/winlibs-mingw64-13.2/x86_64-w64-mingw32/lib/libd2d1.a.
LIBS += -ld2d1

# DirectWrite.  For me the file is libdwrite.a next to libd2d1.a.
LIBS += -ldwrite

# XInput.
LIBS += -lxinput

# For ChooseColor.
LIBS += -lcomdlg32


OBJS :=
OBJS += base-window.o
OBJS += gamepad-viewer.o
OBJS += gpv-config.o
OBJS += resources.o
OBJS += winapi-util.o


-include $(wildcard *.d)

%.o: %.cc
	$(CXX) -c -o $@ $(CXXFLAGS) $<

resources.o: gamepad-viewer.rc doc/icon.ico
	windres -o $@ $<

gamepad-viewer.exe: $(OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)


.PHONY: clean
clean:
	$(RM) *.o *.d *.exe


# EOF
