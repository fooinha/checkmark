#ifndef _CM_NET__
#define _CM_NET_

#include <string>
#include <stdexcept>
#include <memory>
#include <algorithm>

#include <type_traits>

#include <arpa/inet.h>
#include <cm/validator.h>

namespace cm {
/**
 * @namespace cm::net
 * @brief Classes for Internet addresses
 */
namespace net {

/// @cond INTERNAL_DETAIL
namespace detail {
/**
 * @brief
 *
 * @param af
 * @param addr
 *
 * @return
 */
inline bool is_ip(int af, const std::string &addr, unsigned char *buf ) {

	if ( inet_pton(af, addr.c_str(), (void *) buf) > 0 )
		return true;

	return false;

}
} // namespace detail
/// @endcond

namespace exceptions {
/**
 * @class invalid_ip_address
 * @brief An exception class to indicate that an address could not be construted because
 *        of an invalid input.
 */
class invalid_ip_address : public std::invalid_argument {
	// C++11 inheriting constructors
	using invalid_argument::invalid_argument;
};

/**
 * @class invalid_ipv4_address
 * @brief An exception class to indicate that an address could not be construted because
 *        of an invalid input.
 */
class invalid_ipv4_address : public std::invalid_argument {
	// C++11 inheriting constructors
	using invalid_argument::invalid_argument;
};

/**
 * @class invalid_ipv6_address
 * @brief An exception class to indicate that an address could not be construted because
 *        of an invalid input.
 */
class invalid_ipv6_address : public std::invalid_argument {
	// C++11 inheriting constructors
	using invalid_argument::invalid_argument;
};

/**
 * @class invalid_cidr
 * @brief An exception class to indicate that a CIDR could not be construted because
 *        of an invalid input.
 */
class invalid_cidr : public std::invalid_argument {
	// C++11 inheriting constructors
	using invalid_argument::invalid_argument;
};

/**
 * @class invalid_port
 * @brief An exception class to indicate that a TCP/UDP port could not be construted because
 *        of an invalid input.
 */
class invalid_port : public std::invalid_argument {
	// C++11 inheriting constructors
	using invalid_argument::invalid_argument;
};


} // namespace exceptions

/**
 * @brief
 *
 *
 */
class port: public error_check {

	public:

		/// The validator type
		typedef cm::validator<port, exceptions::invalid_port> validator_type;

		/**
		 * @brief
		 *
		 * @tparam T
		 * @param in
		 */
		template< class T = std::string>
			port(const T& in) {

				static_assert(std::is_base_of<range, T>::value || std::is_base_of<std::string, T>::value ,
						"T must be a descendant of std::string or cm::range");

				unsigned long p = 0;
				error_check_assert(in.empty(), "Empty port!");


				for(const auto &c : in ) {

					error_check_assert(! ::isdigit(c), "Invalid character for port.");
					p *= 10;

					/* Converts digit character to int */
					p += (c - 48);

					if (p > 65535)
						error_check_assert(p > 65535, "Number for port too big.");

					_value = p;

				}

			}
		//TODO: integral based constructors
		/*
			port(int p) {
			error_check_assert(p > 65535, "Number for port too big.");
			error_check_assert(p < 0,     "Number for port cannot be negative.");

			_value = p;

			}



		// overload is enabled via a template parameter
		template<class T, typename std::enable_if<std::numeric_limits<T>::is_exact>::type>
		port (T p)
		{

		error_check_assert(p > 65535, "Number for port too big.");
		error_check_assert(p < 0,     "Number for port cannot be negative.");

		_value = p;

		}
		 */
		inline unsigned short int value() const { return _value; }

	private:

		unsigned short int _value = 0;


};

/**
 * @brief
 */
class ip_base : public error_check {


	public:

		inline bool is_ipv6() const { return _af == AF_INET6; }
		inline bool is_ipv4() const { return _af == AF_INET; }
		inline int  family()  const { return _af; }

		/**
		 * @brief
		 *
		 * @return
		 */
		operator std::string() const {
			char str[INET6_ADDRSTRLEN] = {0};

			if (inet_ntop(_af, _buf, str, INET6_ADDRSTRLEN) == NULL) {
				throw std::runtime_error("Invalid IP address conversion");
			}

			return str;
		}

	protected:
		ip_base(int af) : _af(af) {}

		unsigned char _buf[sizeof(struct in6_addr)] = {0};
		int           _af;

};


/**
 * @brief
 */
class ipv6 : public ip_base {

	public:

		/// The validator type
		typedef cm::validator<ipv6, exceptions::invalid_ipv6_address> validator_type;

		/**
		 * @brief
		 *
		 * @param addr
		 */
		ipv6(const std::string &addr) : ip_base(AF_INET6) {

			if (! detail::is_ip(_af, addr, _buf)) {
				set_error("Invalid IPv6 address.");
			}
		}

};


/**
 * @brief
 */
class ipv4 : public ip_base {

	public:

		/// The validator type
		typedef cm::validator<ipv4, exceptions::invalid_ipv4_address> validator_type;

		/**
		 * @brief
		 *
		 * @param addr
		 */
		ipv4(const std::string &addr) : ip_base(AF_INET) {

			_af = AF_INET;

			if (! detail::is_ip(_af, addr, _buf)) {
				set_error("Invalid IPv4 address.");
			}

		}

};




class ip_literal_facade : public error_check {

	private:
		static constexpr const char * ipv6_prefix = "IPv6:";

	public:

		//  [IPv6:::b4]
		//  [1.2.3.4]
		ip_literal_facade(const std::string &in, bool prefix = true) {

			/* [a] */
			if (in.size() < 3 ) {
				set_error("Literal value too small.");
				return;
			}

			if (std::count(in.begin(), in.end(), '[') != 1 ||
				 	std::count(in.begin(), in.end(), ']') != 1)   {
				set_error("Invalid literal value.");
				return;
			}

			if (in.front() != '[' || in.back() != ']') {
				set_error("Invalid enclosing literal value.");
			}

			/* Check if it must be an IPv6 address */
			bool must_be_ipv6 = ( in.find(':') != std::string::npos  ||
					( prefix && in.find(ipv6_prefix, 1) != std::string::npos ) ) ;

			if (must_be_ipv6) {

				size_t pos = (prefix ? 6 : 1 );
				std::string addr(in, pos, in.size() - ( pos + 1 ) );

				ipv6 i6(addr);

				if (i6.has_error())
					set_error("Literal value error. "+i6.error());

				return;
			}

			std::string addr(in, 1, in.size() - 2  );


			ipv4 i4(addr);

			if (i4.has_error())
				set_error("Literal value error. "+i4.error());
		}


};

template < class I = std::string >
class ip : public error_check {

	static_assert(std::is_base_of<ip_base, I>::value || std::is_base_of<std::string, I>::value , 
			"I must be a descendant of ip_base or a std::string");

	public:

	/// The validator type
	typedef cm::validator<ip<>, exceptions::invalid_ip_address> validator_type;

	ip(const ipv6 &in) : _value(in) {
		error_check_assert(in.has_error(), in.error());
	}

	ip(const ipv4 &in) : _value(in)  {
		error_check_assert(in.has_error(), in.error());
	}

	ip(const std::string &in) : _value(in) {

		/* Check if empty */
		error_check_assert(in.empty(), "Empty IP string.");

		bool must_be_ipv6 = ( in.find(':') != std::string::npos);
		if (must_be_ipv6) {

			ipv6 i(in);
			error_check_assert(i.has_error(), i.error());
			return;
		}

		ipv4 i(in);
		error_check_assert(i.has_error(), i.error());


	}

	inline const I & value() const { return _value; }

	private:
	const I &_value;
};//template class resource


/**
  CIDR notation is a compact representation of an IP address and its associated routing prefix.
  The notation is constructed from the IP address and the prefix size, the latter being equivalent
  to the number of leading 1 bits in the routing prefix mask.
  The IP address is expressed according to the standards of IPv4 or IPv6.
  It is followed by a separator character, the slash ('/') character, and the prefix size expressed
  as a decimal number.

  IPv4: The prefix length can range from 0 to 32
  IPv6: The prefix length can range from 0 to 128

*/
class cidr : public error_check {

	private:
		static constexpr const char * kIPv6 = "IPv6";
		static constexpr const char * kIPv4 = "IPv4";

	public:
		/// The validator type
		typedef cm::validator<cidr, exceptions::invalid_cidr> validator_type;

		cidr(const std::string &in) :  _prefix(0), _is_ipv6(false) {

			size_t sep_pos = in.find('/');

			if (sep_pos == std::string::npos) {
				set_error("Missing prefix slash separator character.");
				return;
			}

			_prefix = std::stoi(std::string(in, sep_pos + 1), nullptr, 10);
			_is_ipv6 = (in.find(':') != std::string::npos);

			int max_prefix = (_is_ipv6 ? 128 : 32 );

			if (_prefix < 0 || _prefix > max_prefix) {
				set_error("Bad " + std::string(_is_ipv6 ? kIPv6 :  kIPv4 ) + " prefix.");
				return;
			}

			_address = in.substr(0, sep_pos);

			if (_is_ipv6) {

				ipv6 i6(_address);
				if (i6.has_error()) {
					set_error(i6.error());
					return;
				}

			} else {

				ipv4 i4(_address);
				if (i4.has_error()) {
					set_error(i4.error());
					return;
				}
			}

		}

		inline const std::string &address() const { return _address; }
		inline int prefix() const { return _prefix; }
		inline bool is_ipv6() const { return _is_ipv6; }

	private:
		std::string  _address;
		int          _prefix;
		bool         _is_ipv6;

};


}//namespace net
}//namespace cm

#endif //_CM_NET_

