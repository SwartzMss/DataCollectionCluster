CXXFLAGS = -g3 -O0 -w -fPIC
CXX = g++
CC = gcc 

BUILDDIR = ./src
TARGET = $(BUILDDIR)/libmq.so
TARGETDIR = ./install
LDFLAGS = -fPIC -shared

SRC = $(wildcard src/*.c src/*.cpp src/*.cc)

OBJS = $(patsubst %.cpp,%.o,$(SRC))   

INCLUDES = -I./ -I./src -I../depend/xlogger/include -I../depend/activemq/include -I../depend/utils/include \
		   -I../depend/activemq/include/apr-1
LIBDIR = -Wl,-rpath=./,-rpath-link=./ -lpthread -ldl -L../depend/xlogger/lib -llog4cxx -lxlogger \
		 -L../depend/utils/lib -lutils -L../depend/openssl/lib -lssl -lcrypto \
		 -L../depend/activemq/lib -lactivemq-cpp


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
	install -m 0755 $(BUILDDIR)/libactivemq.h  $(TARGETDIR)/include
	@install -m 0755 ../depend/utils/lib/*  $(TARGETDIR)/lib
	@install -m 0755 ../depend/activemq/lib/*  $(TARGETDIR)/lib
	@install -m 0755 ../depend/xlogger/lib/*  $(TARGETDIR)/lib
	@install -m 0755 ../depend/openssl/lib/*  $(TARGETDIR)/lib

