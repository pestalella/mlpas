WRKDIR = `pwd`

CC = gcc
CXX = g++
LD = g++

INC = 
LIBDIR = 
LIB =
#LDFLAGS =
LDFLAGS = -g3

#CXXFLAGS = -O2
CXXFLAGS = -g3 
CXXFLAGS += -Wall -W -std=c++11
OBJDIR = obj

SRCDIR = src
OUT_BINARY = $(OBJDIR)/mlpas

# Add .d to Make's recognized suffixes.
SUFFIXES += .d

#We don't need to clean up when we're making these targets
NODEPS:=clean
#Find all the C++ files in the src/ directory
SOURCES:=$(shell find $(SRCDIR) -name "*.cpp")
_OBJ := $(addsuffix .o,$(basename $(notdir $(SOURCES))))
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
#These are the dependency files, which make will clean up after it creates them
DEPFILES:=$(patsubst %.cpp,%.d,$(SOURCES))

-include $(DEPFILES)

all: $(OUT_BINARY)

$(SRCDIR)/grammar.hpp: $(SRCDIR)/grammar.yy
	bison -o $(SRCDIR)/grammar.cpp -d $<

$(SRCDIR)/syntax.cpp: $(SRCDIR)/syntax.l $(SRCDIR)/grammar.hpp
	flex -o $(SRCDIR)/syntax.cpp $<

# Create the dependency files
$(SRCDIR)/%.d: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst src/%.cpp,obj/%.o,$<)' $< -MF $@

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(SRCDIR)/%.d
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR):
	test -d $(OBJDIR) || mkdir -p $(OBJDIR)

$(OUT_BINARY): $(OBJDIR) $(OBJS)
	$(LD) $(LIBDIR) -o $@ $(OBJS) $(LDFLAGS) $(LIB)

clean:
	rm -f $(OBJS) $(OUT_BINARY)
	rm -rf $(OBJDIR)

.PHONY: all clean
