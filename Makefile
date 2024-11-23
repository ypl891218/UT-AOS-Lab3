# Compiler and linker
CXX = g++
CXXFLAGS = -g -O0
LD_SCRIPT = default.ld

# Source and output files for hello_world
SRC_HELLO = hello_world.cpp
ELF_HELLO = hello_world.elf

SRC_NULL = null_deref.cpp
ELF_NULL = null_deref.elf

SRC_WORK = workload.cpp
ELF_WORK = workload.elf

SRC_APAGER = apager.cpp
OBJ_APAGER = apager.o

SRC_DPAGER = dpager.cpp
OBJ_DPAGER = dpager.o

SRC_HPAGER = hpager.cpp
OBJ_HPAGER = hpager.o

# Source and output files for loader
SRC_LOADER = section.cpp program.cpp auxv_util.cpp
HDR_LOADER = section.hpp program.hpp auxv_util.hpp
OBJ_LOADER = $(SRC_LOADER:.cpp=.o)

# Default target
all: $(ELF_HELLO) $(ELF_NULL) $(ELF_WORK) apager dpager hpager

# Rule to build the hello_world ELF file
$(ELF_HELLO): $(SRC_HELLO) $(LD_SCRIPT)
	$(CXX) $(CXXFLAGS) -T $(LD_SCRIPT) -static $(SRC_HELLO) -o $(ELF_HELLO)

$(ELF_NULL): $(SRC_NULL) $(LD_SCRIPT)
	$(CXX) $(CXXFLAGS) -T $(LD_SCRIPT) -static $(SRC_NULL) -o $(ELF_NULL)

$(ELF_WORK): $(SRC_WORK) $(LD_SCRIPT)
	$(CXX) $(CXXFLAGS) -T $(LD_SCRIPT) -static $(SRC_WORK) -o $(ELF_WORK)

# Rule to compile .cpp files into .o object files
%.o: %.cpp $(HDR_LOADER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile the loader executable
apager: $(OBJ_APAGER) $(OBJ_LOADER)
	$(CXX) $(CXXFLAGS) $(OBJ_LOADER) $(OBJ_APAGER) -o apager

dpager: $(OBJ_DPAGER) $(OBJ_LOADER)
	$(CXX) $(CXXFLAGS) $(OBJ_LOADER) $(OBJ_DPAGER) -o dpager

hpager: $(OBJ_HPAGER) $(OBJ_LOADER)
	$(CXX) $(CXXFLAGS) -static $(OBJ_LOADER) $(OBJ_HPAGER) -o hpager

# Clean up generated files
clean:
	rm -f $(OBJ_HELLO) $(ELF_HELLO) $(ELF_NULL) $(ELF_WORK) $(OBJ_LOADER) $(EXE_LOADER) apager dpager hpager

# Phony targets
.PHONY: all clean
