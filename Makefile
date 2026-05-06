# ==========================================
# Kizen-N Compiler Engine - Advanced Makefile
# ==========================================

# --- Compiler Settings ---
CXX = clang++
LLVM_FLAGS = $(shell llvm-config --cxxflags)
LLVM_LIBS = $(shell llvm-config --ldflags --system-libs --libs all)

# Strict C++ warnings and auto-tracking for .h header dependencies
CXXFLAGS = -g -O3 -Wall -Wextra $(LLVM_FLAGS) -fexceptions -MMD -MP

# --- Files & Targets ---
# Intentionally left blank to force explicit script execution
SCRIPT ?= 
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
TARGET = kizen-n.out

# .PHONY prevents weird bugs if you create a file named "clean" or "all"
.PHONY: all clean run_program

# Default rule
all: $(TARGET) run_program

# --- Build Rules ---
# Link the compiler engine (Using @ to hide the raw commands)
$(TARGET): $(OBJS)
	@echo "🛠️  Linking compiler engine ($(TARGET))..."
	@$(CXX) -o $(TARGET) $(OBJS) $(LLVM_LIBS)

# Compile C++ source files into object files
%.o: %.cpp
	@echo "🔨 Compiling C++ source: $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Execution Rules ---
# Default run rule with safety guard
run_program: $(TARGET)
ifeq ($(strip $(SCRIPT)),)
	@echo "❌ Error: No script specified."
	@echo "Usage 1: make SCRIPT=filename.kzn"
	@echo "Usage 2: make filename (Recommended)"
	@exit 1
endif
	@echo "\n🚀 Running Kizen-N Compiler on $(SCRIPT)..."
	@./$(TARGET) $(SCRIPT)
	@echo "🔗 Linking native executable (my_program.exe)..."
	@clang output.o -o my_program.exe
	@echo "\n🔥 --- PROGRAM OUTPUT --- 🔥"
	@./my_program.exe

# Magic Pattern Rule for custom scripts (e.g., `make new` runs `new.kzn`)
%: %.kzn $(TARGET)
	@echo "\n🚀 Compiling Kizen-N Script: $<..."
	@./$(TARGET) $<
	@echo "🔗 Linking native executable ($@.exe)..."
	@clang output.o -o $@.exe
	@echo "\n🔥 --- PROGRAM OUTPUT --- 🔥"
	@./$@.exe

# --- Cleanup ---
clean:
	@echo "🧹 Cleaning up build artifacts..."
	@rm -f *.o *.d $(TARGET) *.exe output.o kizen-n.out

# Include auto-generated dependency files so headers trigger recompiles
-include $(DEPS)
