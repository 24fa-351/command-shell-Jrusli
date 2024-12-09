CC = gcc
CFLAGS = -Wall -Wextra -g

# Source files
SRCS = main.c command.c

# Header files
HDRS = command.h

# Output binary
TARGET = main

# Build rules
all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean rules
clean:
	rm -f $(TARGET)

.PHONY: all clean
