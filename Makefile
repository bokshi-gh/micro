CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2 -Iinclude
LDFLAGS = 

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

TARGET = micro

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

PREFIX = /usr/local

.PHONY: all clean install uninstall debug release

all: $(BIN_DIR)/$(TARGET)

# Create directories
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

# Build dependencies
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@$(CXX) $(CXXFLAGS) -MM -MT '$(@:.d=.o)' $< > $@

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(BUILD_DIR)/%.d | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link
$(BIN_DIR)/$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

# Include dependencies
-include $(DEPENDS)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

install: $(BIN_DIR)/$(TARGET)
	install -Dm755 $(BIN_DIR)/$(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)

debug: CXXFLAGS += -g -DDEBUG -O0
debug: clean all

release: CXXFLAGS += -O3 -DNDEBUG
release: clean all

run: all
	./$(BIN_DIR)/$(TARGET) test.txt

test: all
	@echo "Creating test file..."
	@echo "This is a test file." > test.txt
	@echo "Line 2" >> test.txt
	@echo "Line 3 with some content" >> test.txt
	./$(BIN_DIR)/$(TARGET) test.txt
	@rm -f test.txt
