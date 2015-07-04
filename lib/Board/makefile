# Specify compiler
COMP = g++ -std=c++1y -O2

# Specify target
all: checker_run_gens checker_get_age neural_net_test

# Build run generation executable
checker_run_gens: computeGenerations.cpp
	$(COMP) computeGenerations.cpp -fopenmp -o checker_run_gens

# Build get network age executable
checker_get_age: getAge.cpp
	$(COMP) getAge.cpp -o checker_get_age

# Build neural network test executable
neural_net_test: neural_net_test.cpp
	$(COMP) neural_net_test.cpp -o neural_net_test

clean:
	rm checker_run_gens checker_get_age neural_net_test
