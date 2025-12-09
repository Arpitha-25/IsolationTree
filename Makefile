# Define the compiler and flags
CC = gcc
# -Wall and -Wextra enable common warnings; -std=c11 sets the C standard
# -lm links the math library (required for functions like log, pow, ceil)
CFLAGS = -Wall -Wextra -std=c11 -lm
# List all your source files in the src directory
SOURCES = src/main.c src/core_ds.c src/iforest.c src/stream_manager.c src/utils.c
EXECUTABLE = iforest_stream
OUTPUT_DIR = bin

all: $(OUTPUT_DIR)/$(EXECUTABLE)

# Rule to compile and link all source files
$(OUTPUT_DIR)/$(EXECUTABLE): $(SOURCES)
	@mkdir -p $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

clean:
	rm -rf $(OUTPUT_DIR)
	rm -f stream_data.txt