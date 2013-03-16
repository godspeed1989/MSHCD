CC=g++
TARGET=mshcd
CFLAGS = -Wall
CFLAGS+= -g

all: $(TARGET)

$(TARGET): $(TARGET).cpp GetHaarCascade.cpp main.cpp
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -rf $(TARGET)

