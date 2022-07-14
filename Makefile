CXX         = g++
LINK        = $(CXX)
CXXFLAGS    = -std=c++17 -Wall -O2 -g
LDFLAGS     =
# Append FLTK specific flags
CXXFLAGS   += $(shell fltk-config --use-images --cxxflags)
LDFLAGS    += $(shell fltk-config --use-images --ldflags)

TARGET      = fifteen
SOURCES     = $(wildcard *.cpp)
DEPS        = $(wildcard *.h)
OBJECTS     = $(SOURCES:.cpp=.o)

UI_SOURCES  = $(wildcard ui/*.fl)
UI_OBJECTS  = $(UI_SOURCES:.fl=.o)

# Use .exe suffix for binaries in Windows
ifeq ($(OS),Windows_NT)
	TARGET := $(TARGET).exe
endif

.PHONY: all ui clean clean-all

all: ui
	$(MAKE) $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)

clean-all:
	$(MAKE) -C ui clean
	$(MAKE) clean

ui:
	$(MAKE) -C ui all

$(TARGET): $(OBJECTS) $(UI_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o %.fl.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)
