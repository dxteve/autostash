# Define the compiler and flags
CC = gcc
# -Wall -Wextra: Show all warnings
# -g: Include debug symbols
# -I.: Look for headers in the current directory
CFLAGS = -Wall -Wextra -g -I.
LDFLAGS = -lpthread

# Define the executable name
TARGET = autostash

# List all source and object files
SRCS = main.c utilities.c copy_engine.c ui.c scheduler.c
OBJS = $(SRCS:.c=.o)
# List all headers so we can track changes to them
DEPS = config.h utilities.h copy_engine.h ui.h scheduler.h

# Default target: builds the executable
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile each C source file into an object file
# It now also depends on $(DEPS) so changing a header triggers a rebuild
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Target to clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets prevent conflicts with files named 'all' or 'clean'
.PHONY: all clean