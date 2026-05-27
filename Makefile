CC := gcc
LDFLAGS := -lm

# Directories
SRC_DIR := src
INC_DIR := include

# Optimization flags
OPTFLAGS ?= -O3
CFLAGS_COMMON := -Wall -Wextra -Wpedantic -Werror -I$(INC_DIR) $(OPTFLAGS)
CFLAGS_EXTRA ?= # Extra compiler flags
CFLAGS_SOLAR := $(CFLAGS_COMMON) -DRK4 $(strip $(CFLAGS_EXTRA)) # Solar system compiler flags
CFLAGS_TRAP := $(CFLAGS_COMMON) -DTRAPPIST -DRK4 $(strip $(CFLAGS_EXTRA)) # Trappist system compiler flags

# Solar system
SOLAR_OBJ := build/solar_system
SOLAR_BIN := bin/solar_system
SOLAR_OUT := artifacts/solar_system

# Trappist system
TRAP_OBJ := build/trappist_system
TRAP_BIN := bin/trappist_system
TRAP_OUT := artifacts/trappist_system

# Source and header files
SRCS := $(filter-out $(SRC_DIR)/bin2ascii.c, $(wildcard $(SRC_DIR)/*.c))
HEADERS := $(wildcard $(INC_DIR)/*.h)

# Object files
SOLAR_OBJS := $(SRCS:$(SRC_DIR)/%.c=$(SOLAR_OBJ)/%.o)
TRAP_OBJS := $(SRCS:$(SRC_DIR)/%.c=$(TRAP_OBJ)/%.o)

# Extra CLI tokens after run / run-trappist are simulator args, not Make goals.
KNOWN_GOALS := all clean sun trappist run run-trappist convert convert-trappist plot plot-trappist help
ifneq (,$(filter run run-trappist,$(MAKECMDGOALS)))
  $(foreach t,$(filter-out $(KNOWN_GOALS),$(MAKECMDGOALS)),$(eval $(t):;@:))
endif

.PHONY: all clean sun trappist run run-trappist convert convert-trappist plot plot-trappist help

all: sun

# Clean build and bin directories
clean:
	rm -rf build bin artifacts

# Create directories for build, bin, and artifacts
$(SOLAR_OBJ) $(TRAP_OBJ) $(SOLAR_BIN) $(TRAP_BIN) $(SOLAR_OUT) $(TRAP_OUT):
	mkdir -p $@

# Build solar system executable and bin2ascii converter
sun: $(SOLAR_BIN)/solar_system $(SOLAR_BIN)/bin2ascii

$(SOLAR_OBJ)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(SOLAR_OBJ)
	$(CC) $(CFLAGS_SOLAR) -c $< -o $@

$(SOLAR_BIN)/solar_system: $(SOLAR_OBJS) | $(SOLAR_BIN) $(SRC_DIR)
	$(CC) $(CFLAGS_SOLAR) -o $@ $^ $(LDFLAGS)

$(SOLAR_BIN)/bin2ascii: $(SRC_DIR)/bin2ascii.c | $(SOLAR_BIN)
	$(CC) $(CFLAGS_SOLAR) $< -o $@

# Build trappist system executable and bin2ascii converter
trappist: $(TRAP_BIN)/trappist_system $(TRAP_BIN)/bin2ascii

$(TRAP_OBJ)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(TRAP_OBJ)
	$(CC) $(CFLAGS_TRAP) -c $< -o $@

$(TRAP_BIN)/trappist_system: $(TRAP_OBJS) | $(TRAP_BIN) $(SRC_DIR)
	$(CC) $(CFLAGS_TRAP) -o $@ $^ $(LDFLAGS)

$(TRAP_BIN)/bin2ascii: $(SRC_DIR)/bin2ascii.c | $(TRAP_BIN)
	$(CC) $(CFLAGS_TRAP) $< -o $@

# Run executable
run: $(SOLAR_BIN)/solar_system | $(SOLAR_OUT)
	@args="$(filter-out run,$(MAKECMDGOALS))"; \
	if [ -z "$$args" ]; then \
		echo "Inner solar system defaults: 1.98e30 4 0.39 0.72 1.0 1.52 0.001"; \
		./$(SOLAR_BIN)/solar_system 1.98e30 4 0.39 0.72 1.0 1.52 0.001; \
	else \
		./$(SOLAR_BIN)/solar_system $$args; \
	fi

run-trappist: $(TRAP_BIN)/trappist_system | $(TRAP_OUT)
	@args="$(filter-out run-trappist,$(MAKECMDGOALS))"; \
	if [ -z "$$args" ]; then \
		echo "TRAPPIST-1 defaults: 1.58e29 4 0.0115 0.0158 0.0223 0.0292 0.00001"; \
		./$(TRAP_BIN)/trappist_system 1.58e29 4 0.0115 0.0158 0.0223 0.0292 0.00001; \
	else \
		./$(TRAP_BIN)/trappist_system $$args; \
	fi

# Convert binary files to ASCII files (stamp: skip bin2ascii if planet_*.bin unchanged)
$(SOLAR_OUT)/.converted: $(SOLAR_BIN)/bin2ascii $(wildcard $(SOLAR_OUT)/planet_*.bin)
	@if [ -z "$(wildcard $(SOLAR_OUT)/planet_*.bin)" ]; then \
		echo >&2 "No planet_*.bin in $(SOLAR_OUT); run make run first."; exit 1; fi
	./$(SOLAR_BIN)/bin2ascii $(if $(NDT),$(NDT),5) $(SOLAR_OUT)
	@touch $@

$(TRAP_OUT)/.converted: $(TRAP_BIN)/bin2ascii $(wildcard $(TRAP_OUT)/planet_*.bin)
	@if [ -z "$(wildcard $(TRAP_OUT)/planet_*.bin)" ]; then \
		echo >&2 "No planet_*.bin in $(TRAP_OUT); run make run-trappist first."; exit 1; fi
	./$(TRAP_BIN)/bin2ascii $(if $(NDT),$(NDT),5) $(TRAP_OUT)
	@touch $@

.PHONY: convert
convert: $(SOLAR_OUT)/.converted
	@:

.PHONY: convert-trappist
convert-trappist: $(TRAP_OUT)/.converted
	@:

# Plot orbits (gifs)
plot: convert
	@args="$(filter-out plot,$(MAKECMDGOALS))"; \
	python3 plot.py --dir $(SOLAR_OUT) $$args

plot-trappist: convert-trappist
	@args="$(filter-out plot-trappist,$(MAKECMDGOALS))"; \
	python3 plot.py --dir $(TRAP_OUT) $$args

# Help message
help:
	@echo "Usage: make [target]  (default target: all → sun)"
	@echo ""
	@echo "Build:"
	@echo "  all, sun         Inner solar system: bin/solar_system + bin2ascii"
	@echo "  trappist         TRAPPIST-1: bin/trappist_system + bin2ascii"
	@echo ""
	@echo "Run simulation (optional trailing args → passed to the executable):"
	@echo "  run              Writes planet_*.bin under artifacts/solar_system"
	@echo "  run-trappist     Writes planet_*.bin under artifacts/trappist_system"
	@echo ""
	@echo "Post-process:"
	@echo "  convert          .bin → .txt  (optionally: make convert NDT=10)"
	@echo "  convert-trappist Same, for artifacts/trappist_system (make convert-trappist NDT=10)"
	@echo "  plot             plot.py --dir artifacts/solar_system (runs convert if needed)"
	@echo "  plot-trappist    plot.py --dir artifacts/trappist_system"
	@echo ""
	@echo "Other:"
	@echo "  clean            rm -rf build bin artifacts"
	@echo "  help             This message"
	@echo ""
	@echo "Variables (settable at compile time using \"make [target] CFLAGS_EXTRA='[flags]'\"):"
	@echo "  -DTRAPPIST: selects the TRAPPIST-1 system and its parameter file"
	@echo "  -DEULER / -DRK4: chooses the integration method"
	@echo "  -DDEBUG: enables verbose state prints"
	@echo ""
	@echo "Typical flow:  make run && make convert && make plot"

%:
	@echo "Unknown target: $@"
	@echo "Run 'make help' for targets and variables."
	@exit 1