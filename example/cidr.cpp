#include <iostream>
#include <iomanip>
#include <string>

#include <cm/stopwatch.h>
#include <cm/net.h>


int main(int argc, char **argv) {

	bool with_exceptions = false;

	if (argc > 1)
		with_exceptions = true;

	std::string line;

	std::cout << "-----------------------------------------------------------------" << std::endl;

	cm::hires_stopwatch::duration elapsed;
	cm::hires_stopwatch w(elapsed, true);

 	cm::net::cidr::validator_type validator;

	while (std::getline(std::cin, line)) {

		w.reset();
		w.start();

		std::cout << line;

		// Parses and processes lines
		if (with_exceptions)  {

			//	throw exceptions if syntax err is found
			try {

				cm::net::cidr addr = validator(line);
				w.stop();

				std::cout <<  " OK " ;

			} catch(std::invalid_argument &ex) {
				std::cout <<  " EXC : " << ex.what();
				w.stop();
			}
		} else {

			cm::net::cidr addr(line);
			w.stop();

			if (addr.has_error()) {
				std::cout <<  " ERR : " << addr.error();
			} else {
				std::cout <<  " OK";
			}

		}


timings:

		std::cout << " (" << std::setprecision(10) << std::fixed << cm::to_us(elapsed) << "µs)";
		std::cout << std::endl;

	}

	std::cout << "-----------------------------------------------------------------" << std::endl;

	return 0;
}
