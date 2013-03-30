CC=g++
CPPFLAGS=-Wall `pkg-config --cflags opencv`
LIBS=`pkg-config --libs opencv`
TARGET=mshcd

all: $(TARGET)

main.o: main.cpp Detector.hpp Stage.hpp Tree.hpp Feature.hpp Rectangle.hpp \
 Point.hpp Image.hpp CannyPruner.hpp mshcd.hpp


$(TARGET): main.o GetHaarCascade.cpp
	$(CC) $(CPPFLAGS) $+ $(LIBS) -o $@


clean:
	rm -rf $(TARGET) $(TARGET).o main.o

