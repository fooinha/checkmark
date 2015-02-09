#ifndef _CM_LUHN_
#define _CM_LUHN_

#include <cm/validator.h>
#include <algorithm>
#include <initializer_list>

namespace cm {
namespace numbers {

namespace detail {


}//namespace detail

namespace algorithm {

/**
 * @brief
 *
 * http://rosettacode.org/wiki/Luhn_test_of_credit_card_numbers#C.2B.2B11
 * @param id
 *
 * @return
 */
inline bool luhn( const std::string& id) {
	constexpr static int m[10]  = {0,2,4,6,8,1,3,5,7,9}; // mapping for rule 3

	bool is_odd_dgt = false;

	auto lambda = [&](int a, char c) {return a + ((is_odd_dgt = !is_odd_dgt) ? c-'0' : m[c-'0']);};
	int s = std::accumulate(id.rbegin(), id.rend(), 0, lambda);

	return (  s%10 == 0 );
}

}//namespace algorithm

template<class ... Types> struct ranges {

	/**
	 * @brief Is v within the ranges?
	 *
	 * @return
	 */
	bool in(int v) {
		return false;
	}

};



/**
 * @brief
 *
 * @tparam min_digits
 * @tparam max_digits
 */
template<size_t min_digits, size_t max_digits = min_digits>
class cc : public error_check {

	public:
		cc(const std::string &in) {

			error_check_assert(in.empty(), "Empty CC number string.");
			error_check_assert(in.size() < min_digits || in.size() > max_digits,
					"Invalid length for this type of CC number.");
			error_check_assert(! algorithm::luhn(in), "Invalid luhn checksum.");

		}

	private:

};

typedef cc<13,16>         visa;
typedef cc<16>            electron;
typedef cc<15>            amex;
typedef amex              american_express;


/*

TODO: Template base class for specific types

.--------------------------------+-------------------------------------+----------------------------·
|	Credit Card Issuer            |  Starts With ( IIN Range )          | Length ( Number of digits )|
·--------------------------------+-------------------------------------+----------------------------·
|	American Express              |           34, 37                    |           15               |
|	Diners Club - Carte Blanche   |  300, 301, 302, 303, 304, 305       |           14               |
|	Diners Club - International   |             36                      |           14               |
|	Diners Club - USA & Canada    |             54                      |           16               |
|	Discover                      |  6011, 622126 to 622925, 644,       |                            |
|                                |   645, 646, 647, 648, 649, 65       |           16               |
|	InstaPayment                  |      637, 638, 639                  |           16               |
|	JCB                           |        3528 to 3589                 |           16               |
|	Laser                         |    6304, 6706, 6771, 6709           |         16-19              |
|	Maestro                       | 5018, 5020, 5038, 5893, 6304,       |                            |
|                                |     6759, 6761, 6762, 6763          |         16-19              |
|	MasterCard                    |    51, 52, 53, 54, 55               |         16-19              |
|	Visa                          |              4                      |         13-16              |
|	Visa Electron                 |4026, 417500, 4508, 4844, 4913, 4917 |         16                 |
·--------------------------------+-------------------------------------+----------------------------·

*/

}//namespace numbers
}//namespace cm

#endif //_CM_LUHN_
