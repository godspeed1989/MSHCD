CC=g++
TARGET=raw
LIBS=`pkg-config opencv --libs`
CPPFLAGS=`pkg-config opencv --cflags` -g


all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CPPFLAGS) -o $@ $< $(LIBS) 

clean:
	rm -rf gray_img.raw $(TARGET)
