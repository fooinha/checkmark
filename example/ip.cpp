
#include <iostream>
#include <iomanip>
#include <string>

#include <cm/stopwatch.h>
#include <cm/net.h>


int main(int argc, char **argv) {

	std::string line;

	bool is_ipv6 = false;

	if (argc > 1) {
		int i = std::stoi( argv[1]);

		if (i == 6)
			is_ipv6 = true;
	}


	std::cout << "-----------------------------------------------------------------" << std::endl;
	if (is_ipv6) {
		std::cout << " * Processing IPv6 addresses " << std::endl;
	} else {
		std::cout << " * Processing IPv4 addresses " << std::endl;
	}

	cm::hires_stopwatch::duration elapsed;
	cm::hires_stopwatch w(elapsed, true);

   cm::net::ipv4::validator_type ipv4_validator;
   cm::net::ipv6::validator_type ipv6_validator;

	while (std::getline(std::cin, line)) {

		w.reset();
		w.start();

		std::cout << line;


		//throw exceptions if syntax err is found
		try {

			if (is_ipv6) {
				cm::net::ipv6 addr = ipv6_validator(line);
			} else {
				cm::net::ipv4 addr = ipv4_validator(line);
			}

			w.stop();

			std::cout <<  " OK " ;

		} catch(std::invalid_argument &ex) {
			std::cout <<  " EXC : " << ex.what();
			w.stop();
		}


		std::cout << " (" << std::setprecision(10) << std::fixed << cm::to_us(elapsed) << "µs)";
		std::cout << std::endl;

	}

	std::cout << "-----------------------------------------------------------------" << std::endl;

	return 0;
}
