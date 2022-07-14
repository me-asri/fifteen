FLUID       = fluid
CXX         = g++
LINK        = $(CXX)
CXXFLAGS    = -std=c++17 -Wall -O2 -g

SOURCES     = $(wildcard *.fl)
CODES       = $(SOURCES:.fl=.cpp)
HEADERS     = $(SOURCES:.fl=.h)
OBJECTS     = $(CODES:.cpp=.o)

.PHONY: all clean

all: $(CODES) $(OBJECTS)

clean:
	rm -f $(CODES) $(HEADERS) $(OBJECTS)

%.cpp: %.fl
	$(FLUID) -c -o .cpp -h .h $<

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)
