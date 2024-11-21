# Compiler and linker
CXX = g++
CXXFLAGS = -g -O2
LD_SCRIPT = custom.ld

# Source and output files for hello_world
SRC_HELLO = hello_world.cpp
OBJ_HELLO = hello_world.o
ELF_HELLO = hello_world.elf

# Source and output files for loader
SRC_LOADER = loader.cpp section.cpp program.cpp auxv_util.cpp
EXE_LOADER = loader
HDR_LOADER = section.hpp program.hpp auxv_util.hpp
OBJ_LOADER = $(SRC_LOADER:.cpp=.o)

# Default target
all: $(ELF_HELLO) $(EXE_LOADER)

# Rule to build the hello_world ELF file
$(ELF_HELLO): $(OBJ_HELLO) $(LD_SCRIPT)
	$(CXX) -T $(LD_SCRIPT) -static $(OBJ_HELLO) -o $(ELF_HELLO)

# Rule to compile the hello_world object file
$(OBJ_HELLO): $(SRC_HELLO)
	$(CXX) $(CXXFLAGS) -c $(SRC_HELLO) -o $(OBJ_HELLO)

# Rule to compile .cpp files into .o object files
%.o: %.cpp $(HDR_LOADER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile the loader executable
$(EXE_LOADER): $(OBJ_LOADER)
	$(CXX) $(CXXFLAGS) $(OBJ_LOADER) -o $(EXE_LOADER)

# Clean up generated files
clean:
	rm -f $(OBJ_HELLO) $(ELF_HELLO) $(OBJ_LOADER) $(EXE_LOADER)

# Phony targets
.PHONY: all clean
