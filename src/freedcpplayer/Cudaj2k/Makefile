CXXFLAGS+= -pedantic -Wall 
CXXFLAGS+=-O3
CPP = gcc
SHAREDINCPATH = ../asdcplib-master/src/
LIBDIRS = -L../asdcplib-master/src/  -L/usr/local/cuda/lib64 -L/usr/lib/x86_64-linux-gnu/
OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

INCDIRS = -I/usr/include -I/usr/local/include  -I$(SHAREDINCPATH)    
             

CXXFLAGS+=$(shell sdl2-config --cflags)
CXXFLAGS+= -g $(INCDIRS)
LDFLAGS+=$(shell sdl2-config --libs)
LDFLAGS+=-ljpeg
LDFLAGS+=-lm
LDFLAGS+=-lstdc++
LDFLAGS+=-lnvjpeg2k  -lcrypto -lssl -lpthread -lstdc++fs  -lcudart -lSDL2_ttf
LDFLAGS+=libkumu.a libasdcp.a

TARGET=freedcpplayer

bin: $(TARGET)

$(OBJS): %.o: %.cpp
	$(CPP) -c $< $(CXXFLAGS) $(CPPFLAGS) -o  $@

$(TARGET): $(OBJS)
	$(CPP) $(OBJS) $(LDFLAGS) $(LIBDIRS) -o $@


clean:
	$(RM) $(OBJS) $(TARGET)

