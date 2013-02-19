CC=g++
TARGET=mshcd

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) -g -o $@ $<

