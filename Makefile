# gamepad-viewer/Makefile

all: gamepad-viewer.exe


CXX := g++

CXXFLAGS := -g -Wall -Werror

LDFLAGS := -g -Wall -municode

LIBS :=


OBJS :=
OBJS += gamepad-viewer.o


%.o: %.cc
	$(CXX) -c -o $@ $(CXXFLAGS) $<

gamepad-viewer.exe: $(OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)


# EOF
