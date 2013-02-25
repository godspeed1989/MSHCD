CC=g++
TARGET1=raw
TARGET2=draw
LIBS=`pkg-config opencv --libs`
CPPFLAGS=`pkg-config opencv --cflags` -g


all: $(TARGET1) $(TARGET2)

$(TARGET1): $(TARGET1).cpp
	$(CC) $(CPPFLAGS) -o $@ $< $(LIBS)

$(TARGET2): $(TARGET2).cpp
	$(CC) $(CPPFLAGS) -o $@ $< $(LIBS)

clean:
	rm -rf $(TARGET1) $(TARGET2)

