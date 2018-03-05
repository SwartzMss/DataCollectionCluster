CXXFLAGS = -g3 -O0 -w -fPIC -std=c++11
CXX = g++
CC = gcc 

BUILDDIR = ./src
TARGET = $(BUILDDIR)/DcServer
TARGETDIR = ./Bin
LDFLAGS = -fPIC -shared

SRC = $(wildcard src/*.c src/*.cpp src/*.cc)

OBJS = $(patsubst %.cpp,%.o,$(SRC))   

INCLUDES = -I./ -I./src -I../depend/libevent/include -I../depend/thrift/include \
	   -I../depend/rapidjson/include -I../depend/utils/include \
	   -I../depend/xlogger/include

LIBDIR = -Wl,-rpath=./ -lpthread -ldl -L../depend/libevent/lib -levent -levent_pthreads\
	 -L../depend/xlogger/lib -llog4cxx -lxlogger -L../depend/utils/lib -lutils \
	 -L../depend/thrift/lib -lthrift -l thriftnb -L../depend/openssl/lib -lssl -lcrypto	


all:$(TARGET)
	
$(TARGET): $(OBJS)
	$(CXX)   $(LIBDIR) -o $@ $^ 
	
%.o : %.cpp
	$(CXX) $(INCLUDES) -c $(CXXFLAGS) $< -o $@

%.o : %.c
	$(CXX) $(INCLUDES) -c $(CXXFLAGS) $< -o $@

%.o : %.cc
	$(CXX) $(INCLUDES) -c $(CXXFLAGS) $< -o $@

clean:
	@rm -rvf $(OBJS) $(TARGET)
	@rm -rf $(TARGETDIR)/*

install:$(TARGET)
	install -d $(TARGETDIR)
	install -m 0755 $(TARGET)  $(TARGETDIR)
	@install -m 0755 src/log4cxx.properties  $(TARGETDIR)
	@install -m 0755 ../depend/xlogger/lib/lib*  $(TARGETDIR)
	@install -m 0755 ../depend/utils/lib/*  $(TARGETDIR)
	@install -m 0755 ../depend/libevent/lib/*  $(TARGETDIR)
	@install -m 0755 ../depend/thrift/lib/*  $(TARGETDIR)
	@install -m 0755 ../depend/openssl/lib/*  $(TARGETDIR)
