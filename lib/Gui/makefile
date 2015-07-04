# Specify compiler
COMP = g++ -std=c++1y -O2

# Specify target
all: checker_gui

# Build graphical executable
checker_gui: checkerGame.cpp
	$(COMP) checkerGame.cpp -fopenmp -lGL -lGLU -lglut -lGLEW -o checker_gui

# Clean build
clean:
	rm checker_gui

