
#include <iostream>
#include <iomanip>
#include <string>

#include <cm/stopwatch.h>
#include <cm/url.h>


int main(int argc, char **argv) {
	std::string line;

	std::cout << "-----------------------------------------------------------------" << std::endl;

	cm::hires_stopwatch::duration elapsed;
	cm::hires_stopwatch w(elapsed, true);

	while (std::getline(std::cin, line)) {

		w.reset();
		w.start();

		std::cout << "-----------------------------------------------------------------" << std::endl;


		for(size_t i = 0; i < line.size() ; ++i) {
			std::cout << (i % 10 ) ;
		}

		std::cout << std::endl;


		size_t j = 0;
		for(size_t i = 0; i < line.size() ; ++i) {
			if ( i % 10 ) {
				std::cout << " ";
			}
			else {
				std::cout << j  ;
				++j;
			}
		}

		std::cout << std::endl << "-----------------------------------------------------------------" << std::endl;

		std::cout << line;

		try {
			cm::url::factory::create(line);

		} catch (std::invalid_argument &ex) {
			std::cerr << " ERR:" << ex.what();
		}
		w.stop();

timings:

		std::cout << " (" << std::setprecision(10) << std::fixed << cm::to_us(elapsed) << "µs)";
		std::cout << std::endl;

	}

	std::cout << "-----------------------------------------------------------------" << std::endl;

	return 0;
}
