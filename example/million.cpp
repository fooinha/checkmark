#include <iostream>
#include <iomanip>

#include <cm/stopwatch.h>

void fn(){
	std::cout << ".";
}

int main(int argc, char **argv) {

	cm::hires_stopwatch::duration elapsed;
	{
		cm::hires_stopwatch sw(elapsed);

		for (size_t i = 0; i < 1000000; ++i)
			fn();
		std::cout << std::endl;
	}

	std::cerr << "secs > " << std::setprecision(5) <<
		cm::to_seconds(elapsed) << std::endl;

	return 0;
}
