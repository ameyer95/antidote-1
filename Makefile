CXX=g++
CXXFLAGS=-O3

SRCDIR=src
BUILDDIR=build
BINDIR=bin
INCLUDEDIR=include

SRCS=$(shell find $(SRCDIR) -name "*.cpp")
OBJS=$(SRCS:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
DEPS=$(OBJS:.o=.d)

TARGET=$(BINDIR)/main

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I $(INCLUDEDIR) -MMD -c -o $@ $<


TEST_SRCDIR=test/src
TEST_BUILDDIR=test/build
TEST_BINDIR=test/bin
TEST_CATCHDIR=test/catch2

TEST_SRCS=$(shell find $(TEST_SRCDIR) -name *.cpp)
TEST_OBJS=$(TEST_SRCS:$(TEST_SRCDIR)/%.cpp=$(TEST_BUILDDIR)/%.o)
TEST_DEPS=$(TEST_OBJS:.o=.d)

TEST_TARGET=$(TEST_BINDIR)/tester

# When linking the tester, we need to exclude the actual project's main object
MAIN_OBJ=$(BUILDDIR)/main.o

$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
	@mkdir -p $(TEST_BINDIR)
	$(CXX) -o $@ $(TEST_OBJS) $(filter-out $(MAIN_OBJ), $(OBJS))

$(TEST_BUILDDIR)/%.o: $(TEST_SRCDIR)/%.cpp
	@mkdir -p $(TEST_BUILDDIR)
	$(CXX) -I $(INCLUDEDIR) -I $(TEST_CATCHDIR) -MMD -c -o $@ $<


.PHONY: clean test all

all: $(TARGET) $(TEST_TARGET)

clean:
	rm -rf $(BUILDDIR) $(BINDIR) $(TEST_BUILDDIR) $(TEST_BINDIR)

test: $(TEST_TARGET)
	$(TEST_TARGET)


-include $(DEPS) $(TEST_DEPS)
