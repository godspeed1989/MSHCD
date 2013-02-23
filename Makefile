CC=g++
TARGET=mshcd
CFLAGS = -Wall
#CFLAGS+= -g -DDEBUG

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(TARGET)

