#ifndef _CM_UUID
#define _CM_UUID

#include <string>
#include <cctype>
#include <functional>

#include <cm/validator.h>

/**
 * @namespace cm::uuid
 * @brief Classes for UUID
 */
namespace cm {

namespace uuid {

namespace exceptions {

/**
 * @class invalid_uuid
 * @brief An exception class to indicate that an invalid uuid string representation
 */
class invalid_uuid : public std::invalid_argument {
	// C++11 inheriting constructors
	using invalid_argument::invalid_argument;
};

/**
 * @class invalid_nil_uuid
 * @brief An exception class to indicate that an invalid nil uuid string representation
 */
class invalid_nil_uuid : public std::invalid_argument {
	// C++11 inheriting constructors
	using invalid_argument::invalid_argument;
};


}//namespace exceptions

/// @cond INTERNAL_DETAIL
namespace detail {

inline bool allxdigits(const char *c, size_t n) {

	for(size_t i = 0; i < n ; ++i) {
		if (! ::isxdigit(*c)) {
			return false;
		}
		++c;
	}

	return true;
}

inline bool allzeros(const char *c, size_t n) {

	for(size_t i = 0; i < n ; ++i) {
		if (*c != '0') {
			return false;
		}
		++c;
	}
	return true;
}

} //namespace detail

class uuid_base : public error_check {

	private:

		constexpr static size_t SIZE = 36;

	public:
		uuid_base(const std::string & in ) {

			/* Check if empty */
			error_check_assert(in.empty(), "Empty UUID string.");

			/* Check size */
			error_check_assert(in.size() != SIZE ,
					"Invalid UUID string size. ("+std::to_string(in.size())+")");

		}

		typedef std::function<bool(const char *, size_t)>  digit_fn;
		typedef bool(*digit_fn_ptr)(const char *, size_t);

	protected:

		void check_digits (const char *c, digit_fn check ) {

			/* Check each part */

			error_check_assert(! check(c, 8),
					"Invalid non hex digits characters. #1");

			c+=8;

			error_check_assert(*c++ != '-', "Missing '-' (Dash) separator.");

			error_check_assert(! check(c, 4),
					"Invalid non hex digits characters. #2");

			c+=4;

			error_check_assert(*c++ != '-', "Missing '-' (Dash) separator.");

			error_check_assert(! check(c, 4),
					"Invalid non hex digits characters. #3");

			c+=4;

			error_check_assert(*c++ != '-', "Missing '-' (Dash) separator.");

			error_check_assert(! check(c, 4),
					"Invalid non hex digits characters. #4");

			c+=4;

			error_check_assert(*c++ != '-', "Missing '-' (Dash) separator.");

			error_check_assert(! check(c, 12),
					"Invalid non hex digits characters. #5");

		}


};


/**
 * @brief
 *
 * @tparam f
 * @tparam E
 */
template <uuid_base::digit_fn_ptr f = &detail::allxdigits, class E = exceptions::invalid_uuid >
class uuid_t : public uuid_base  {

	public:

		/// The validator type
		typedef cm::validator< uuid_t <f>, E > validator_type;

		uuid_t(const std::string & in )  :  uuid_base(in) {

			if (has_error())
				return;

			check_digits (in.c_str(), f);

		}
};

/**
https://tools.ietf.org/html/rfc4122#section-4.1

- UUID string representation
-------------------------------------------
| c  |12345678-1234-1234-1234-123456789012|
|----|------------------------------------|
| i  |012345678901234567890123456789012345|
-------------------------------------------
 */

/// UUID string representation
typedef uuid_t<>  uuid;

/**

  - NIL UUID string representation
  -------------------------------------------
  | c  |00000000-0000-0000-0000-000000000000|
  |----|------------------------------------|
  | i  |012345678901234567890123456789012345|
  -------------------------------------------
 */

/// Nil UUID string representation
typedef uuid_t<&detail::allzeros, exceptions::invalid_nil_uuid  >  nil_uuid;

}// namespace uuid
}// namespace cm

#endif // _CM_UUID
