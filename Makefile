SRCDIR    := src
INCDIR    := inc
UIDIR     := ui
OBJDIR    := obj
BINDIR    := bin

SRCEXT    := cpp
DEPEXT    := h
FLEXT     := fl
OBJEXT    := o

CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Werror -O3 -g
INC       := -I$(INCDIR)
LIB       :=

FLUID     := fluid
FLTKFLAGS :=

TARGET    := fifteen

# Add .exe suffix for binaries if using Windows
ifeq ($(OS),Windows_NT)
	TARGET := $(TARGET).exe
endif

SRCS      := $(wildcard $(SRCDIR)/*.$(SRCEXT) $(SRCDIR)/**/*.$(SRCEXT))
DEPS      := $(wildcard $(INCDIR)/*.$(DEPEXT) $(INCDIR)/**/*.$(DEPEXT))
OBJS      := $(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(SRCS:.$(SRCEXT)=.$(OBJEXT)))
BIN       := $(BINDIR)/$(TARGET)

# Add FLTK specific flags
CXXFLAGS  += $(shell fltk-config --cxxflags $(FLTKFLAGS))
LIB       += $(shell fltk-config --ldflags $(FLTKFLAGS))

# Add FLTK UI objects
FLOBJDIR  := $(UIDIR)/__obj__
FLINCDIR  := $(UIDIR)/__inc__
FLS       := $(wildcard $(UIDIR)/*.$(FLEXT)) $(wildcard $(UIDIR)/**/*.$(FLEXT))
OBJS      += $(patsubst $(UIDIR)/%, $(FLOBJDIR)/%, $(FLS:.$(FLEXT)=.$(FLEXT).$(OBJEXT)))
INC       += -I$(FLINCDIR)

export CXX CXXFLAGS FLUID SRCEXT DEPEXT FLEXT OBJEXT

.PHONY: all ui clean

all: ui
	@echo + Building $(TARGET)
	@$(MAKE) $(BIN)

ui:
	@echo + Building UI
	@$(MAKE) -C $(UIDIR)

$(BIN): $(OBJS)
	@mkdir -p $(BINDIR)

	$(CXX) -o $(BIN) $^ $(LIB)
	@echo + Built $(TARGET)

$(OBJDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT) $(DEPS)
	@mkdir -p $(dir $@)

	$(CXX) -c -o $@ $< $(CXXFLAGS) $(INC)

clean:
	$(RM) -r $(OBJDIR) $(BINDIR)

cleaner: clean
	@$(MAKE) -C $(UIDIR) clean
