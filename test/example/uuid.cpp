#include <iostream>
#include <iomanip>
#include <string>

#include <cm/stopwatch.h>
#include <cm/uuid.h>


int main(int argc, char **argv) {

	bool nil_uuid = false;

	if (argc > 1)
		nil_uuid = true;

	std::string line;

	std::cout << "-----------------------------------------------------------------" << std::endl;

	cm::hires_stopwatch::duration elapsed;
	cm::hires_stopwatch w(elapsed, true);

	while (std::getline(std::cin, line)) {

		w.reset();
		w.start();


		std::cout << line;

		// Parses and processes lines
		if (nil_uuid)  {
			cm::uuid::nil_uuid u(line);

			if (u.has_error()) {
				std::cout <<  " ERR : " << u.error();
			} else {
				std::cout <<  " OK";
			}



		} else {
			cm::uuid::uuid u(line);

			if (u.has_error()) {
				std::cout <<  " ERR : " << u.error();
			} else {
				std::cout <<  " OK";
			}

		}

		w.stop();

timings:

		std::cout << " (" << std::setprecision(10) << std::fixed << cm::to_us(elapsed) << "µs)";
		std::cout << std::endl;

	}

	std::cout << "-----------------------------------------------------------------" << std::endl;

	return 0;
}
