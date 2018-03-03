CXXFLAGS = -g3 -O0 -w
CXX = g++
CC = gcc 

BUILDDIR = ./src
TARGET = $(BUILDDIR)/libutils.a
TARGETDIR = ./install

SRC = $(wildcard src/*.c src/*.cpp src/*.cc)

OBJS = $(patsubst %.cpp,%.o,$(SRC))   

INCLUDES = -I./ -I./src
LIBDIR = -lpthrea -ldl


all:$(TARGET)
	
	
$(TARGET): $(OBJS)
	ar cr   $(TARGET) $(OBJS)
	
%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o : %.c
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o : %.cc
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	@rm -rvf $(OBJS) $(TARGET)
	@rm -rf $(TARGETDIR)

install:$(TARGET)
	install -d $(TARGETDIR)/lib
	install -d $(TARGETDIR)/include
	install -m 0755 $(TARGET)  $(TARGETDIR)/lib
	install -m 0755 $(BUILDDIR)/*.h  $(TARGETDIR)/include
