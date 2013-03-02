CC=g++
CFLAGS= -g -Wall
TARGET=main

all: $(TARGET)

$(TARGET): $(TARGET).cpp *.hpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGET)

