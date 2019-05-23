CXX=g++

SRCDIR=src
BUILDDIR=build
BINDIR=bin
INCLUDEDIR=include

SRCS=$(shell find $(SRCDIR) -name *.cpp)
OBJS=$(SRCS:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
DEPS=$(OBJS:.o=.d)

TARGET=$(BINDIR)/main

$(TARGET): $(OBJS)
	mkdir -p $(BINDIR)
	$(CXX) -o $@ $(OBJS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(BUILDDIR)
	$(CXX) -I $(INCLUDEDIR) -MMD -c -o $@ $<

.PHONY: clean
clean:
	rm -rf $(BUILDDIR) $(BINDIR)

-include $(DEPS)
