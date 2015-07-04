# Specify compiler
COMP = g++ -std=c++1y -O2

# Specify target
all: checker_client

# Build network client executable
checker_client: aiclient.cpp
	$(COMP) aiclient.cpp -fopenmp -o checker_client

# Clean build
clean:
	rm checker_client

