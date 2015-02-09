#include <iostream>
#include <iomanip>
#include <string>

#include <cm/stopwatch.h>
#include <cm/smtp.h>


int main(int argc, char **argv) {

	std::string line;

	bool with_exceptions = false;

	if (argc > 1)
		with_exceptions = true;


	std::cout << "-----------------------------------------------------------------" << std::endl;

	cm::hires_stopwatch::duration elapsed;
	cm::hires_stopwatch w(elapsed, true);

   cm::smtp::address::validator_count_type email_validator;
   cm::dns::domain::validator_type domain_validator;

	while (std::getline(std::cin, line)) {

		w.reset();
		w.start();

		std::cout << line;

		// Parses and processes lines
		if (with_exceptions)  {

			// throw exceptions if syntax err is found
			try {

				cm::smtp::address addr = email_validator(line);
				w.stop();

				domain_validator(addr.get_domain().value());

				std::cout <<  " OK " ;

			} catch(std::invalid_argument &ex) {
				std::cout <<  " EXC : " << ex.what();
				w.stop();
			}
		} else {

			cm::smtp::address addr(line);
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
	if (with_exceptions)  {
		std::cout << " # GOOD:" << email_validator.good() << std::endl;
		std::cout << "  # BAD:" << email_validator.bad() << std::endl;

	}

	return 0;
}
