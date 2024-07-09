# gamepad-viewer/Makefile

all: gamepad-viewer.exe


CXX := g++

CXXFLAGS :=
CXXFLAGS += -g
CXXFLAGS += -Wall
CXXFLAGS += -Werror
CXXFLAGS += -DUNICODE

LDFLAGS := -g -Wall -municode

LIBS :=


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
