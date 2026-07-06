CC = gcc
CXX = g++

CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2 -Iinclude
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2 -Iinclude

SRC_C = src/main.c src/cli.c src/terminal.c src/errors.c
SRC_CPP = src/editor.cpp

OBJ = $(SRC_C:.c=.o) $(SRC_CPP:.cpp=.o)

TARGET = micro

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

install: $(TARGET)
	install -Dm755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

.PHONY: all clean install uninstall
