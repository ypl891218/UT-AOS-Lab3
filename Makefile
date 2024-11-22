# Compiler and linker
CXX = g++
CXXFLAGS = -g -O0
LD_SCRIPT = default.ld

# Source and output files for hello_world
SRC_HELLO = hello_world.cpp
OBJ_HELLO = hello_world.o
ELF_HELLO = hello_world.elf

SRC_APAGER = apager.cpp
OBJ_APAGER = apager.o

SRC_DPAGER = dpager.cpp
OBJ_DPAGER = dpager.o

# Source and output files for loader
SRC_LOADER = section.cpp program.cpp auxv_util.cpp
HDR_LOADER = section.hpp program.hpp auxv_util.hpp
OBJ_LOADER = $(SRC_LOADER:.cpp=.o)

# Default target
all: $(ELF_HELLO) apager dpager

# Rule to build the hello_world ELF file
$(ELF_HELLO): $(OBJ_HELLO) $(LD_SCRIPT)
	$(CXX) $(CXXFLAGS) -T $(LD_SCRIPT) -static $(OBJ_HELLO) -o $(ELF_HELLO)

# Rule to compile the hello_world object file
$(OBJ_HELLO): $(SRC_HELLO)
	$(CXX) $(CXXFLAGS) -c $(SRC_HELLO) -o $(OBJ_HELLO)

# Rule to compile .cpp files into .o object files
%.o: %.cpp $(HDR_LOADER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile the loader executable
apager: $(OBJ_APAGER) $(OBJ_LOADER)
	$(CXX) $(CXXFLAGS) -static $(OBJ_LOADER) $(OBJ_APAGER) -o apager

dpager: $(OBJ_DPAGER) $(OBJ_LOADER)
	$(CXX) $(CXXFLAGS) -static $(OBJ_LOADER) $(OBJ_DPAGER) -o dpager

# Clean up generated files
clean:
	rm -f $(OBJ_HELLO) $(ELF_HELLO) $(OBJ_LOADER) $(EXE_LOADER)

# Phony targets
.PHONY: all clean
