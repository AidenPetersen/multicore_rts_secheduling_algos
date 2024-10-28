RUN_EXEC := run_emu
LLREF_EXEC := llref_emu
BUILD_DIR := ./build

RUN_SRC := run
LLREF_SRC := llref
COMMON_SRC := common

RUN_SRCS := $(shell find $(RUN_SRC) -name '*.cpp')
LLREF_SRCS := $(shell find $(LLREF_SRC) -name '*.cpp')
COMMON_SRCS := $(shell find $(COMMON_SRC) -name '*.cpp')

RUN_OBJS := $(RUN_SRCS:%=$(BUILD_DIR)/%.o)
LLREF_OBJS := $(LLREF_SRCS:%=$(BUILD_DIR)/%.o)
COMMON_OBJS := $(COMMON_SRCS:%=$(BUILD_DIR)/%.o)

RUN_INC_DIRS := $(shell find $(RUN_SRC) -type d)
LLREF_INC_DIRS := $(shell find $(LLREF_SRC) -type d)
COMMON_INC_DIRS := $(shell find $(COMMON_SRC) -type d)

RUN_INC_FLAGS := -I$(RUN_INC_DIRS) -I$(COMMON_INC_DIRS)
LLREF_INC_FLAGS := -I$(LLREF_INC_DIRS) -I$(COMMON_INC_DIRS)

CXX=g++
LIBS=
CFLAGS=-Wall

all: $(LLREF_EXEC) $(RUN_EXEC)


$(LLREF_EXEC): $(LLREF_OBJS) common
	$(CXX) $(LLREF_OBJS) $(CFLAGS) $(LIBS) $(LLREF_INC_FLAGS) -o $@

$(LLREF_OBJS): $(LLREF_SRCS) common
	mkdir -p $(dir $@)
	$(CXX) -c -o $@ $< $(CFLAGS) $(LLREF_INC_FLAGS) $(CFLAGS)

$(RUN_EXEC): $(RUN_OBJS)
	$(CXX) $(RUN_OBJS) $(CFLAGS) $(LIBS) $(RUN_INC_FLAGS) -o $@

$(RUN_OBJS): $(RUN_SRCS)
	mkdir -p $(dir $@)
	$(CXX) -c -o $@ $< $(CFLAGS) $(RUN_INC_FLAGS) $(CFLAGS)

common: $(COMMON_EXEC)

$(COMMON_EXEC): $(COMMON_OBJS) 
	$(CXX) $(COMMON_OBJS) $(CFLAGS) $(LIBS) $(COMMON_INC_FLAGS) -o $@

$(COMMON_OBJS): $(COMMON_SRCS)
	mkdir -p $(dir $@)
	$(CXX) -c -o $@ $< $(CFLAGS) $(COMMON_INC_FLAGS) $(CFLAGS)

clean:
	rm -rf build
	rm $(RUN_EXEC)
	rm $(LLREF_EXEC)
