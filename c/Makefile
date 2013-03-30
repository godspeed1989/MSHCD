CC=g++
TARGET=mshcd
CPPFLAGS=-Wall `pkg-config --cflags opencv`
LIBS=`pkg-config --libs opencv`

all: $(TARGET)

$(TARGET).o: $(TARGET).cpp $(TARGET).hpp
GetHaarCascade.o: GetHaarCascade.cpp
main.o: main.cpp

$(TARGET): $(TARGET).o GetHaarCascade.o main.o
	$(CC) $+ $(LIBS) $(CPPFLAGS) -o $@

clean:
	rm -rf $(TARGET) *.o

