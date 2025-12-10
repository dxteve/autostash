# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I.
LDFLAGS = -lpthread

# Define the executable name
TARGET = autostash

# List all source and object files
SRCS = main.c utilities.c copy_engine.c ui.c scheduler.c
OBJS = $(SRCS:.c=.o)

# Default target: builds the executable
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile each C source file into an object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Target to clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean