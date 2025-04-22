# Define compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Define the source files and the target executable
SRCS = server.c parsefile.c
OBJS = $(SRCS:.c=.o)
TARGET = server

# Default rule to build the target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Rule to compile .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Rule to run the server after building it
run: $(TARGET)
	./$(TARGET)

# Phony targets (not actual files)
.PHONY: all clean run
