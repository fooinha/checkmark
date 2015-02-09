#include <iostream>
#include <iomanip>
#include <string>

#include <cm/stopwatch.h>
#include <cm/luhn.h>


int main(int argc, char **argv) {

	std::string line;

	std::cout << "-----------------------------------------------------------------" << std::endl;

	cm::hires_stopwatch::duration elapsed;
	cm::hires_stopwatch w(elapsed, true);

	while (std::getline(std::cin, line)) {

		w.reset();
		w.start();


		cm::numbers::american_express u(line);

		if (u.has_error()) {
			std::cout <<  " ERR : " << u.error();
		} else {
			std::cout <<  " OK";
		}

		w.stop();

timings:

		std::cout << " (" << std::setprecision(10) << std::fixed << cm::to_us(elapsed) << "µs)";
		std::cout << std::endl;

	}

	std::cout << "-----------------------------------------------------------------" << std::endl;

	return 0;
}
