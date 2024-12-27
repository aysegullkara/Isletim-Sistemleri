# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Target program
TARGET = Program

# Source files
SRCS = Program.c

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean target
clean:
	rm -f $(TARGET)
