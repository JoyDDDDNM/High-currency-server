#include <thread>
#include <iostream>

#include <mutex> 
#include <atomic>	 

std::mutex m; // construct mutex

void printHello(int j) {
	for (int n = 0; n < 10; n++) {
		m.lock(); // lock cout object
		std::cout << j << " Hello, child thread" << std::endl;
		m.unlock();
	}
}

int main() {
	
	std::thread t1{ printHello, 1 };

	std::thread t2{ printHello, 2 };
	 
	t1.detach();
	
	t2.detach();

	for (int n = 0; n < 10; n++) {
		std::cout << "Hello, main thread" << std::endl;
	}

	while(true){}
	return 0;
}
