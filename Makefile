CC=g++
TARGET=mshcd

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) -o $@ $<

