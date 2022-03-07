CXXFLAGS+= -pedantic -Wall 
CXXFLAGS+=-O1
CPP = g++
LIBDIRS = -L/usr/lib/x86_64-linux-gnu/
OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

INCDIRS = -I/usr/include -I/usr/local/include    
             

CXXFLAGS+=$(shell sdl2-config --cflags)
CXXFLAGS+=$(shell wx-config --cxxflags)
CXXFLAGS+= -g $(INCDIRS)
LDFLAGS+=$(shell sdl2-config --libs)
LDFLAGS+=$(shell wx-config  --libs)
LDFLAGS+=-lm
LDFLAGS+=-lstdc++fs -lstdc++


TARGET=RunMyDcpPlayer

bin: $(TARGET)

$(OBJS): %.o: %.cpp
	$(CPP) -c $< $(CXXFLAGS) $(CPPFLAGS) -o  $@

$(TARGET): $(OBJS)
	$(CPP) $(OBJS) $(LDFLAGS) $(LIBDIRS) -o $@


clean:
	$(RM) $(OBJS) $(TARGET)

