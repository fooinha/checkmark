#ifndef _CM_DOMAIN_
#define _CM_DOMAIN_

#include <string>
#include <stdexcept>
#include <memory>
#include <sstream>
#include <algorithm>
#include <vector>

#include <cm/validator.h>
#include <cm/net.h>

namespace cm {
namespace dns {
namespace exceptions {

/**
 * @class invalid_domain
 * @brief An exception class to indicate that an address could not be construted because
 *        of an invalid input.
 */
class invalid_domain : public std::invalid_argument {
	// C++11 inheriting constructors
	using invalid_argument::invalid_argument;
};

} //namespace exceptions


/**
 * @class domain
 * @brief Represents the Internet domain name with a valid syntax
 *
 *  The name should be no longer than 255 characters, should
 *  contain only letters, digits, dots and hyphens, no adjacent
 *  dots and hyphens, no leading or trailing dots or hyphens,
 *  no labels longer than 63 characters, and it
 *  should not be all numeric.
 *
 *  It also supports UTF-8 characters.

 *  Does not check the real-life existence of domain's name suffixes.
 *  Does not throw any exception in case of invalid arguments.
 *  Caller must check the domain::has_error().
 *
 */
class domain : public error_check {


	public:

		/// The underlying type for the domain name value
		typedef std::string                 value_type;

		// The validator type
		typedef cm::validator<domain, exceptions::invalid_domain> validator_type;

		/// A pointer for a domain object
		typedef std::shared_ptr<domain>     ptr;

		/// Maximum number of bytes for a domain name label
		static constexpr size_t max_label_size = 63; // RFC 1035

		/// Maximum number of bytes for a domain name
		static constexpr size_t max_name_size = 255; // RFC 1035

		/**
		 * @brief Domain name object factory method
		 *
		 * Allocates and creates a pointer to domain object
		 *
		 * @param in The input argument
		 *
		 * @return The shared pointer
		 */
		static ptr create(const std::string & in) {
			return ptr(new domain(in));
		}

		/**
		 * @brief Gets the value used to create the domain object.
		 *
		 * @return The original underlying string value for the domain name
		 */
		inline const value_type & value() const { return _value; }


		/**
		 * @brief Create a list of domain name labels
		 *
		 * @return Returns as a vector,
		 */
		inline std::vector<value_type> labels() const {

			std::vector<value_type> ret;

			if (has_error())
				return ret;

			value_type current;

			for( auto c: value()) {
				if (c == '.') {
					ret.emplace_back(current);
					current.clear();
				}
			}

			ret.emplace_back(current);

			return ret;
		}


		/**
		 * @brief Constructs a domain from a std::string.
		 *
		 * @param in The input argument.
		 */
		domain(const std::string &in) : _value(in) {

			if (in.empty()) {
				_err = "Domain name is empty.";
				return;
			}

			if (in.size() > max_name_size ) {
				_err = "Domain name is too big.";
				return;
			}

			/* No leading space */
			if (std::isspace(in.front())) {
				_err = "Domain name with leading whitespace.";
				return;
			}

			/* No trailing space */
			if (std::isspace(in.back())) {
				_err = "Domain name with trailing whitespace.";
				return;
			}

			/* Dot (.) at start of local part */
			if (in.front() == '.') {
				_err = "Domain name begins with the '.' (Dot) character.";
				return;
			}

			/* Dot (.) at end of local part */
			if (in.back() == '.') {
				_err = "Domain name ends with the '.' (Dot) character.";
				return;
			}

			/* Hyphen (-) at start of local part */
			if (in.front() == '-') {
				_err = "Domain name begins with the '-' (Hyphen) character.";
				return;
			}

			/* Hyphen (-) At end of local part */
			if (in.back() == '-') {
				_err = "Domain name ends with the '-' (Hyphen) character.";
				return;
			}



			/* Check if value can be IPv4/IPv6 literal */
			bool is_literal = false;
			if (in.front() == '[' && in.back() == ']') {
				is_literal = true;
			}


			/* Check address if literal */
			if (is_literal) {

				net::ip_literal_facade f(in);

				if (f.has_error()) {
					set_error(f);
				}


			} else {

				size_t cnt_digits = 0;

				/* Check if all chars are valid */
				if ( ! std::all_of(in.cbegin(), in.cend(), [&cnt_digits](char c){

							// * Uppercase and lowercase English letters (a–z, A–Z) (ASCII: 65–90, 97–122)
							if ((c >= 65 && c<= 90) || (c>=97 && c<= 122)) return true;

							// * Digits 0 to 9 (ASCII: 48–57)
							if (c >= 48 && c<= 57) {
							++cnt_digits;
							return true;
							}

							// * Dots and hyphens
							if (c == '.' || c == '-' || c == ' ') return true;

							//TODO: Not sure about if this is enough
							// * International characters above U+007F, encoded as UTF-8
							unsigned u = (unsigned ) (c & 0x000000ff);
							if (u > 0x7f) return true;

							return false;

				})) {

					_err = "Domain name has invalid characters.";
					return;
				}

				if (cnt_digits == in.size()) {
					_err = "The domain name is composed only by digit characters." ;
					return;
				}

			}


			char previous = '\0';
			size_t label_len = 0;

			/* Checks if there's any invalid adjacent characters */
			for (size_t i = 0; i < in.size() ; ++i) {

				if (label_len > max_label_size) {
					_err = "Label size too big for domain at position " + std::to_string(i) ;
					return;
				}

				if ( ( in[i] == '-' || in[i] == '.') && previous == '.') {
					_err = "Invalid sequence of characters for domain at position " + std::to_string(i) ;
					return;
				}

				if ( in[i] == '.' && previous == '-') {
					_err = "Invalid sequence of characters for domain at position " + std::to_string(i) ;
					return;
				}


				previous = in[i];
				++label_len;

				if (in[i] == '.') label_len = 0;
			}

		}

	private:
		domain() = delete; // Disables the empty constructor

		std::string _value;

};



}//namespace domain
}//namespace cm

#endif //_CM_DOMAIN_
