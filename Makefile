CC = gcc
LDFLAGS = -lm

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR_BASE = build
BIN_DIR_BASE = bin
OUT_DIR_BASE = artifacts

# Debug and release flags
CFLAGS_COMMON = -Wall -Wextra -Wpedantic -Werror -I$(INC_DIR)

# Dynamic CFLAGS configuration based on MAKECMDGOALS
# Base flags (Debug vs Release)
ifneq (,$(findstring debug,$(MAKECMDGOALS)))
ifneq (,$(findstring release,$(MAKECMDGOALS)))
$(error Error: Cannot specify both debug and release)
endif
    CFLAGS_BASE = $(CFLAGS_COMMON) -g -O0 -DDEBUG
else
    CFLAGS_BASE = $(CFLAGS_COMMON) -O3
endif

# System flags (Trappist vs Sun)
CFLAGS_SYS =
ifneq (,$(findstring trappist,$(MAKECMDGOALS)))
ifneq (,$(findstring sun,$(MAKECMDGOALS)))
	$(error Error: Cannot specify both sun and trappist)
endif
    CFLAGS_SYS += -DTRAPPIST
	BIN_DIR = $(BIN_DIR_BASE)/trappist_system
	OBJ_DIR = $(OBJ_DIR_BASE)/trappist_system
	OUT_DIR = $(OUT_DIR_BASE)/trappist_system
	TARGET = trappist_system
else
	BIN_DIR = $(BIN_DIR_BASE)/solar_system
	OBJ_DIR = $(OBJ_DIR_BASE)/solar_system
	OUT_DIR = $(OUT_DIR_BASE)/solar_system
	TARGET = solar_system
endif

# Method flags (Euler vs RK4)
CFLAGS_METHOD =
ifneq (,$(findstring euler,$(MAKECMDGOALS)))
ifneq (,$(findstring rk4,$(MAKECMDGOALS)))
$(error Error: Cannot specify both euler and rk4)
endif
    CFLAGS_METHOD += -DEULER
else
    CFLAGS_METHOD += -DRK4
endif

# Combined CFLAGS
CFLAGS = $(CFLAGS_BASE) $(CFLAGS_SYS) $(CFLAGS_METHOD)

# Directory targets
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# Targets
TARGETS = $(BIN_DIR)/$(TARGET) $(BIN_DIR)/bin2ascii
SRCS = $(filter-out $(SRC_DIR)/bin2ascii.c, $(wildcard $(SRC_DIR)/*.c))
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
HEADERS = $(wildcard $(INC_DIR)/*.h)

# Arguments
ARGS = $(filter-out $@,$(MAKECMDGOALS))

# Default target
all: sun bin2ascii

# Flexible Configuration Targets
# Matches targets containing keywords to trigger a clean build with appropriate flags
.PHONY: config_targets
config_targets:
	$(MAKE) $(TARGETS) CFLAGS="$(CFLAGS)"

# Match keywords and hyphenated combinations
sun trappist debug release euler rk4: config_targets
 	# idk why I need to put an empty target or it will not work

# mixed targets
sun-% trappist-% debug-% release-% euler-% rk4-% %-sun %-trappist %-debug %-release %-euler %-rk4: config_targets
	@:

# Compile binary to ASCII converter
bin2ascii: $(BIN_DIR)/bin2ascii
	@echo "Binary written to $(BIN_DIR)/bin2ascii"

# Build binary to ASCII converter
$(BIN_DIR)/bin2ascii: $(SRC_DIR)/bin2ascii.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Build executable
$(BIN_DIR)/$(TARGET): $(OBJS) | $(BIN_DIR) $(SRC_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build and bin directories
clean:
	rm -rf $(OBJ_DIR_BASE) $(BIN_DIR_BASE) $(OUT_DIR_BASE)

# Run executable
run: $(BIN_DIR)/$(TARGET)
# redirect arguments to the executable, if none use default arguments
	@args="$(filter-out $@,$(MAKECMDGOALS))"; \
	if [ -z "$$args" ]; then \
		echo "Running default arguments (Solar System): 1.98e30 4 0.39 0.72 1.0 1.52 0.001"; \
		./$(BIN_DIR)/$(TARGET) 1.98e30 4 0.39 0.72 1.0 1.52 0.001; \
	else \
		./$(BIN_DIR)/$(TARGET) $$args; \
	fi

run-trappist: $(BIN_DIR)/$(TARGET)
	@args="$(filter-out $@,$(MAKECMDGOALS))"; \
	if [ -z "$$args" ]; then \
		echo "Running default arguments (TRAPPIST-1 System): 1.58e29 4 0.0115 0.0158 0.0223 0.0292 0.00001"; \
		./$(BIN_DIR)/$(TARGET) 1.58e29 4 0.0115 0.0158 0.0223 0.0292 0.00001; \
	else \
		./$(BIN_DIR)/$(TARGET) $$args; \
	fi

convert: $(BIN_DIR)/bin2ascii $(OUT_DIR)
	./$(BIN_DIR)/bin2ascii 5 $(OUT_DIR)

plot: convert
	@args="$(filter-out $@,$(MAKECMDGOALS))"; \
	if [ -z "$$args" ]; then \
		echo "Plotting default arguments (Solar System)"; \
		python3 plot.py; \
	else \
		python3 plot.py $$args; \
	fi

# Help
.PHONY: all clean debug run help sun trappist euler config_targets

# Help message
help:
	@echo "Usage: make [target]"
	@echo "Targets:"
	@echo "  all              Build the executable (Default: Sun)"
	@echo "  [config]         Build with specific config (e.g. trappist-debug, sun-euler)"
	@echo "  bin2ascii        Convert binary files to ASCII files"
	@echo "  run [args]       Run the executable"
	@echo "  convert          Convert binary files to ASCII files"
	@echo "  clean            Clean the build and bin directories"
	@echo "  help             Show this help message"
	@echo ""
	@echo "Configuration keywords (can be combined):"
	@echo "  sun, trappist    System selection (default is sun)"
	@echo "  debug, release   Debug or release build (default is release)"
	@echo "  euler, rk4       Method selection (default is rk4)"

%:
	@echo "Unknown target: $@"
	@echo "Run 'make help' to see valid options (e.g. sun-euler-debug)."
	@exit 1
