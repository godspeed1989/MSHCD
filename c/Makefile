CC=g++
TARGET=mshcd
CFLAGS = -Wall
CFLAGS+= -g -DDEBUG

all: $(TARGET)

$(TARGET): $(TARGET).cpp GetHaarCascade.cpp
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -rf $(TARGET)

