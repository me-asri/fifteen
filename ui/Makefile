SRCDIR   := __src__
INCDIR   := __inc__
OBJDIR   := __obj__

FLSRCEXT := $(FLEXT).$(SRCEXT)
FLDEPEXT := $(FLEXT).$(DEPEXT)
FLOBJEXT := $(FLEXT).$(OBJEXT)

FLS      := $(wildcard *.$(FLEXT))

SRCS     := $(addprefix $(SRCDIR)/, $(FLS:.$(FLEXT)=.$(FLSRCEXT)))
DEPS     := $(addprefix $(INCDIR)/, $(FLS:.$(FLEXT)=.$(FLDEPEXT)))
OBJS     := $(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(SRCS:.$(SRCEXT)=.$(OBJEXT)))

.PHONY: all clean

all: $(OBJS)
	@$(RM) -r $(SRCDIR)

clean:
	$(RM) -r $(INCDIR) $(OBJDIR)

$(SRCDIR)/%.$(FLSRCEXT): %.$(FLEXT)
	$(FLUID) -c -o .$(FLSRCEXT) -h .$(FLDEPEXT) $<

	@mkdir -p $(SRCDIR) $(INCDIR)

	@mv $<.$(SRCEXT) $(SRCDIR)
	@mv $<.$(DEPEXT) $(INCDIR)

$(OBJDIR)/%.$(FLOBJEXT): $(SRCDIR)/%.$(FLSRCEXT)
	@mkdir -p $(OBJDIR)

	$(CXX) -c -o $@ $< $(CXXFLAGS) -I$(INCDIR)
