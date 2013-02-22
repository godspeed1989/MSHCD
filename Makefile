CC=g++
TARGET=mshcd
CFLAGS=-g -Wall

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(TARGET)

