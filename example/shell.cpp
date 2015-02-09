#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <map>

#include <cm/stopwatch.h>
#include <cm/url.h>
#include <cm/smtp.h>
#include <cm/uuid.h>
#include <cm/media.h>

/*

 INPUT:

 > mode   // changes parsing mode
 value

*/

int main(int arg, char **argv) {

	enum MODE {
		M_URL = 0,
		M_EMAIL = 1,
		M_IP = 2,
		M_LITERAL_IP = 3,
		M_CIDR = 4,
		M_DOMAIN = 5,
		M_MEDIA_TYPE = 6,
		M_ATTRIBUTE = 7,
		M_UUID = 42,
		M_NONE = 666
	};

	const std::map<std::string,MODE> modes_names(
			{
			{ "url", M_URL },
			{ "email", M_EMAIL},
			{ "literal_ip", M_LITERAL_IP},
			{ "ip", M_IP},
			{ "uuid", M_UUID},
			{ "cidr", M_CIDR},
			{ "domain", M_DOMAIN},
			{ "attribute", M_ATTRIBUTE},
			{ "media-type", M_MEDIA_TYPE}
			}
			);

	MODE current = M_NONE;
	std::string mode_arguments;

	std::string line;
	cm::hires_stopwatch::duration elapsed;
	cm::hires_stopwatch w(elapsed, true);

	while (std::getline(std::cin, line)) {

		/* Discard empty lines */
		if (line.empty()) {
			std::cout << std::endl;
			continue;
		}

		/* Change parsing mode operator */
		if (line.front() == '>'){
			line.erase(0,1);

			if (line.empty()) {
				std::cerr << " ! Invalid mode." << std::endl;
				continue;
			}

			bool is_argument = false;

			/* Mode has arguments */
			if (line.front() == '>') {
				is_argument = true;
			}


			/* Trim spaces */
			line.erase(
					std::remove_if(line.begin(), line.end(), [](char x){return std::isspace(x);}),
					line.end());

			if (is_argument) {
				mode_arguments = line;
				std::cerr << "* Argument modes changes."  << std::endl;
				continue;
			}

			auto search = modes_names.find(line);

			if (search == modes_names.end()) {
				std::cerr << "! Invalid mode." << std::endl;
				continue;
			}

			current = search->second;
			std::cerr << "* Changed to mode ["<< search->first <<"]" << std::endl;
			continue;
		}


		std::cout << line << " |=> ";

		w.reset();
		w.start();

		switch(current) {

			case M_URL:
				{
					auto a = cm::url::factory::create(line);
					std::cout << (cm::error_check) (*a);
					break;
				}

			case M_EMAIL:
				{
					cm::smtp::address a(line);
					std::cout << (cm::error_check) (a);

					break;
				}

			case M_CIDR:
				{

					cm::net::cidr a(line);
					std::cout << (cm::error_check) (a);

					break;
				}

			case M_UUID:
				{

					cm::uuid::uuid a(line);
					std::cout << (cm::error_check) (a);

					break;
				}

			case M_LITERAL_IP:
				{

					cm::net::ip_literal_facade a(line, false);
					std::cout << (cm::error_check) (a);

					break;
				}

			case M_IP:
				{

					cm::net::ip<> a(line);
					std::cout << (cm::error_check) (a);

					break;
				}

			case M_DOMAIN:
				{

					cm::dns::domain a(line);
					std::cout << (cm::error_check) (a);

					break;
				}

			case M_ATTRIBUTE:
				{

					cm::net::media::attribute a(line);
					std::cout << (cm::error_check) (a);

					break;
				}

			case M_MEDIA_TYPE:
				{

					cm::net::media::type a(line);
					std::cout << (cm::error_check) (a);

					break;
				}


			case M_NONE:
				std::cerr << " ! Mode not selected." << std::endl;
				break;
			default:
				std::cerr << " ! Mode not implemented." << std::endl;

		}
		w.stop();

timings:

		std::cout << " (" << std::setprecision(10) << std::fixed << cm::to_us(elapsed) << "µs)";
		std::cout << std::endl;

	}


	return 0;
}
