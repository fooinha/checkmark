#ifndef _CM_BITCOIN_
#define _CM_BITCOIN_

#include <cm/validator.h>
#include <initializer_list>

namespace cm {
namespace bitcoin {

namespace exceptions {

/**
 * @class invalid_address
 * @brief An exception class to indicate that an address could not be construted because
 *        of an invalid input.
 */
class invalid_address : public std::invalid_argument {
	// C++11 inheriting constructors
	using invalid_argument::invalid_argument;
};

} //namespace exceptions



/**
 * @brief
 *
 * A Bitcoin address, or simply address, is an identifier of 26-35 alphanumeric characters,
 * beginning with the number 1 or 3, that represents a possible destination for a Bitcoin payment.
 *
 * http://rosettacode.org/wiki/Bitcoin/address_validation
 */
class address : public error_check {


	public:

	// The validator type
	typedef cm::validator<address, exceptions::invalid_address> validator_type;

	address(const std::string &in) {

		error_check_assert(in.empty(), "Empty Bitcoin address.");
		error_check_assert(in.front() != '1' || in.front() != '3' , "Invalid Bitcoin address version.");
		error_check_assert(in.size() < 26 ||  in.size() > 35 , "Invalid Bitcoin address size.");
		error_check_assert( ! std::all_of(in.begin(), in.end(), ::isalpha),
				"Invalid Bitcoin non alphanumeric character.");

		//TODO: TO perform hash validation

		_value = in;
	}

	private:

	std::string _value;





};


}//namespace bitcoin
}//namespace cm

#endif //_CM_BITCOIN_
