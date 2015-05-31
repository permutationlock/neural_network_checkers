all: checker_run_gens checker_get_age neural_net_test

checker_run_gens: computeGenerations.cpp
	g++ computeGenerations.cpp -std=c++1y -O2 -o checker_run_gens

checker_get_age: getAge.cpp
	g++ getAge.cpp -std=c++1y -O2 -o checker_get_age

neural_net_test: neural_net_test.cpp
	g++ neural_net_test.cpp -std=c++1y -O2 -o neural_net_test

clean:
	rm checker_run_gens checker_get_age neural_net_test

