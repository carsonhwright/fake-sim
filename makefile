# tool macros
CC = gcc# FILL: the C compiler
CXX := g++
INCLUDES = -I include -I . -I /usr/local/include #-pthread #TODO do I need this? probably
CFLAGS = -Wall -Wextra -g $(INCLUDES)# FILL: compile flags
#CFLAGS =-Wall -Wextra -g -pthread -L /usr/local/lib/libz $(INCLUDES)# FILL: compile flags
CXXFLAGS := -Wall -Wextra -g $(INCLUDES)
# raylib (visualization). Static lib in /usr/local/lib needs these system deps.
LDFLAGS := -L /usr/local/lib
LDLIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
DBGFLAGS = -g
COBJFLAGS = $(CFLAGS) -c
CXXOBJFLAGS = $(CXXFLAGS) -c
# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug
TEST_PATH := tests

# tests: one binary per tests/*.cpp, linked against the non-raylib objects
# (the sim/geo/math core). Add a new file to tests/ and it's picked up.
TEST_SRC := $(wildcard $(TEST_PATH)/*.cpp)
TEST_BIN := $(addprefix $(BIN_PATH)/, $(notdir $(basename $(TEST_SRC))))
TEST_LINK_OBJ := $(OBJ_PATH)/sim.o $(OBJ_PATH)/fake-geo.o $(OBJ_PATH)/fake-math.o

# compile macros
TARGET_NAME := fake # FILL: target name

TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)

# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# clean files list
DISTCLEAN_LIST := $(OBJ) \
                  $(OBJ_DEBUG)
CLEAN_LIST := $(TARGET) \
			  $(TARGET_DEBUG) \
			  $(DISTCLEAN_LIST)

# default rule
default: makedir all

# non-phony targets
$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(COBJFLAGS) -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CXXOBJFLAGS) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(COBJFLAGS) $(DBGFLAGS) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CXXOBJFLAGS) $(DBGFLAGS) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) $(OBJ_DEBUG) -o $@ $(LDFLAGS) $(LDLIBS)

# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET_DEBUG)

# build + run the test suite
$(BIN_PATH)/%: $(TEST_PATH)/%.cpp $(TEST_LINK_OBJ)
	@mkdir -p $(BIN_PATH)
	$(CXX) $(CXXFLAGS) -o $@ $< $(TEST_LINK_OBJ) -lm

.PHONY: test
test: $(TEST_BIN)
	@for t in $(TEST_BIN); do echo "== $$t =="; ./$$t || exit 1; done

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(DISTCLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)