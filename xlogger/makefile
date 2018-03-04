CXXFLAGS = -g3 -O0 -w -fPIC
CXX = g++
CC = gcc 

BUILDDIR = ./src
TARGET = $(BUILDDIR)/libxlogger.so
TARGETDIR = ./install
LDFLAGS = -fPIC -shared

SRC = $(wildcard src/*.c src/*.cpp src/*.cc)

OBJS = $(patsubst %.cpp,%.o,$(SRC))   

INCLUDES = -I./ -I./src -I./include
LIBDIR = -lpthread -ldl -L./lib -Wl,-rpath=./,-rpath-link=./ -llog4cxx 


all:$(TARGET)
	
	
$(TARGET): $(OBJS)
	$(CXX)  $(LDFLAGS) $(LIBDIR) -o $@ $^ 
	
%.o : %.cpp
	$(CXX) $(INCLUDES) -c $(CXXFLAGS) $< -o $@

%.o : %.c
	$(CXX) $(INCLUDES) -c $(CXXFLAGS) $< -o $@

%.o : %.cc
	$(CXX) $(INCLUDES) -c $(CXXFLAGS) $< -o $@

clean:
	@rm -rvf $(OBJS) $(TARGET)
	@rm -rf $(TARGETDIR)

install:$(TARGET)
	install -d $(TARGETDIR)/lib
	install -d $(TARGETDIR)/include
	install -m 0755 $(TARGET)  $(TARGETDIR)/lib
	install -m 0755 $(BUILDDIR)/log4cxx.properties  $(TARGETDIR)/lib
	install -m 0755 $(BUILDDIR)/xloggerDefine.h  $(TARGETDIR)/include
	install -m 0755 $(BUILDDIR)/*.h  $(TARGETDIR)/include	
