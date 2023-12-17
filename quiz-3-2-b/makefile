CXX=g++
CXXFLAGS=-std=c++17 -g -O0 -pedantic -Wall -Wextra -fno-omit-frame-pointer -pthread
LDLIBS=-lrt

SRCS=library.cpp
BINS=$(SRCS:%.cpp=%.exe)
OBJS=$(DEPS:%.cpp=%.o)

all: clean $(BINS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.exe: %.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(patsubst %.exe,%,$@) $^ $(LDLIBS)

.PHONY: clean 

clean:
	rm -f library