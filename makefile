
# Compiler
CXX = g++
CXXFLAGS = -Wall -std=c++17 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs)

# Source files
SRC = main.cpp
OUT = myapp

# Default target
all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)
