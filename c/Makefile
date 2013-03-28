CC=g++
TARGET=mshcd
CFLAGS=`pkg-config --cflags opencv`
LIBS=`pkg-config --libs opencv`

all: $(TARGET)

$(TARGET): $(TARGET).cpp GetHaarCascade.cpp main.cpp
	$(CC) $+ $(LIBS) $(CFLAGS) -o $@

clean:
	rm -rf $(TARGET)

