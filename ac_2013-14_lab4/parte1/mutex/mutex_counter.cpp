#include <thread>
#include <iostream>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

// Access mutex. 
std::mutex m;

int counter = 0;

void f(int it){
	for (int iteration = 0; iteration < it; iteration++){
    		std::lock_guard<std::mutex> l(m);
		counter++;
	}
}

int main(int argc, char ** argv){
	if (argc !=3){
		std::cerr << "Invalid args" << std::endl;
		return -1;
	}
  
	int number_threads = std::stoi(argv[1]);
	int iterations = std::stoi(argv[2]);

	auto t1 = std::chrono::system_clock::now();
	
	std::vector<std::thread> pool;
	
	for (int index = 0; index < number_threads; index++){
		pool.push_back(std::thread(f, iterations));
	}

	for (int index = 0; index < number_threads; index++){
		pool[index].join();
	}
	
	auto t2 = std::chrono::system_clock::now();
	std::chrono::microseconds dif = t2-t1;

	std::cout << "Final value: " << counter << " threads: " << number_threads << " iterations: " << iterations << " time: " << dif.count() << std::endl;

	return 0;
}

