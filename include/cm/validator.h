#ifndef _CM_VALIDATOR_
#define _CM_VALIDATOR_

#include <string>
#include <stdexcept>
#include <atomic>
#include <memory>
#include <type_traits>
#include <iostream>

namespace cm {


/**
 * @class
 *
 * @brief
 */
class range : public std::initializer_list<char> {

	public:

		static constexpr const char *kEmpty = "";

		typedef std::string                   value_type;
		typedef value_type::size_type         size_type;
		typedef value_type::reference         reference;
		typedef value_type::const_reference   const_reference;

		typedef value_type::iterator          iterator;
		typedef value_type::const_iterator    const_iterator;

		/// A pointer for a range object
		typedef std::shared_ptr<range> ptr;


		/**
		 * @brief
		 *
		 * @param value
		 * @param offset
		 * @param count
		 */
		range(const value_type &value,
				size_type offset = 0,
				size_type count = 0 ) throw (std::out_of_range) : _value(value) {

			size_type sz = value.size();

			if (count == value_type::npos)
				count = 0;

			/* Empty range */
			if (sz == 0 && offset == 0 && count == 0 ) {
				_offset = 0;
				_count = 0;
				return;
			}

			//              sz   =>  10
			//0123456789    max i => 9
			//    ^     |
			//    |     |
			//    o     t


			if (offset >= sz)
				throw std::out_of_range("range offset error");

			if ( ( offset + count ) >= sz)
				throw std::out_of_range("range count error off=["+
						std::to_string(offset) +"] count=["         +
						std::to_string(count)  +"] sz=["            +
						std::to_string(sz)     +"] ");

			_offset = offset;

			if (count)
				_count = count;
			else
				_count = (sz - _offset); /* Calculates the count until end of input value */

		}

		void operator++() {

			if (( _offset + 1) > _value.size())
				throw std::out_of_range("range incr error");

			++_offset;

			if (_count > 0)
				--_count;

		}

		void operator+=(unsigned i) {

			for (size_t j =0; j < i ; ++j)
				operator++();
		}

		// cast operator
		operator value_type() const {

			if (empty())
				return kEmpty;

			return value_type(_value, _offset, _count);
		}

		inline bool empty() const         { return _count == 0; }

		inline bool has_next() const { return (_count > 1 ); }


		inline size_type get_size() const    { return _count; }
		inline size_type get_count() const    { return _count; }
		inline size_type get_offset() const   { return _offset; }

		iterator begin() const            {
			iterator it = std::remove_const<value_type>::type(_value).begin();
			std::advance(it, _offset);
			return it;
			//return std::remove_const<value_type>::type(_value).begin();
		}
		iterator end() const              {

			iterator it = std::remove_const<value_type>::type(_value).begin();
			std::advance(it, _offset + _count);
			return it;
			//return std::remove_const<value_type>::type(_value).end();
		}

		const_reference front() const {
			return _value[_offset];
		}

		/**
		 * @brief range object factory method
		 *
		 * Allocates and creates a pointer to range object
		 *
		 * @param in The input argument
		 * @param offset //TODO
		 * @param count  //TODO
		 *
		 * @return The shared pointer
		 *
		 */
		static ptr create(const std::string & in,
				size_type offset = 0,
				size_type count = 0) {

			ptr p = nullptr;

			try {
				p = ptr(new range(in, offset, count));
			} catch(std::out_of_range &) {
				p = ptr(new range(kEmpty, 0, 0));
			}

			return p;
		}

		static ptr create_empty() {
			return  ptr(new range(kEmpty, 0, 0));
		}

	private:

		/**
		 * @brief
		 *
		 * @param os
		 * @param range
		 *
		 * @return
		 */
		friend std::ostream & operator<<  ( std::ostream &os, const range & range);

		///
		const value_type & _value;

		///
		size_type _offset;

		///
		size_type _count;

};

std::ostream & operator<<  ( std::ostream &os, const range & range) {

	if (range.empty())
		return os;

	return os << static_cast<const std::string &>(range);
}

/**
 * @brief
 *
 *
 * Throwing excpetions is time costly. Some say otherwise!
 * http://stackoverflow.com/questions/13835817/are-exceptions-in-c-really-slow
 *
 * If you need, you may use this base class, for error handling and use exceptions
 * when using derived classes combined with another design patterns.
 *
 */
class error_check {

	public:

		/**
		 * @brief Shows which error occurred while parsing the input value
		 *
		 * @return The error description. It returns empty if no error.
		 */
		inline const std::string & error() const { return _err; }

		/**
		 * @brief Checks if an error occurred while parsing the input value
		 *
		 * @return The boolean result.
		 */
		inline bool has_error() const { return ! _err.empty(); }

		/**
		 * @brief Sets the error found
		 *
		 */
		inline void set_error(const std::string &err)  { _err = err; }

#define error_check_assert(cond, s) if(cond) { set_error(s); return; }

		/**
		 * @brief Sets the error found from another instance
		 *
		 * @param other
		 */
		inline void set_error(const error_check &other)  { _err = other._err; }

	protected:

		/// The error message
		std::string     _err;

		friend std::ostream & operator<<  ( std::ostream &os, const error_check & e);


};

std::ostream & operator<<  ( std::ostream &os, const error_check & e) {
	if (e.has_error())
		return 	os << "ERR: [" << e.error() << "]";

	return	os << "OK.";

}

/// @cond INTERNAL_DETAIL
// http://stackoverflow.com/questions/12117912/add-remove-data-members-with-template-parameters

struct count  {

	public:
		/**
		 * @brief The number of good addresses.
		 *
		 */
		inline const size_t good() const { return _good; }

		/**
		 * @brief The number of bad addresses.
		 *
		 */
		inline const size_t bad() const { return _bad; }

	protected:

		explicit count() : _good(0), _bad(0) { }

		void incr_good () { ++_good; }
		void incr_bad () { ++_bad; }

		std::atomic<size_t> _good;
		std::atomic<size_t> _bad;
};

struct no_count {
	protected:
		void incr_good() {};
		void incr_bad() {};
};

///@endcond


/**
 * @class validator
 * @brief Functor class - can act as a template validator service
 *
 * Template parameter list
 * T - Object type to instantiate
 - Must be a descendent of error_check class type
 - Must have the following methods
 - T::T(const std::string &) ctor
 - bool T::has_error(void)
 - std::string T::error(void)

 E - Exception type to throw
 * do_count - Boolean: enables counting good/bad validator feature
 *
 */
template <class T, class E = std::invalid_argument, bool do_count = false >
class validator : public std::conditional<do_count, count, no_count >::type {

	static_assert(std::is_base_of<error_check, T>::value, "T must be a descendant of error_check");

	typedef typename std::conditional<do_count, count, no_count >::type count_type;

	public:

	/// The template class type for validation
	typedef T class_type;
	/// The template exception class if validation fails
	typedef E exception_type;

	/**
	 * @brief The validator constructor.
	 *
	 * The instance can be reused to validate a list of input strings.
	 *
	 * It keeps count of how many good and bad objects are submitted to validation.
	 */
	validator () {}

	/**
	 * @brief The function. Returns an validated object from a input string
	 *
	 * @param in The input string argument.
	 *
	 * @return The resulting object.
	 *
	 * @throw exception_type
	 */
	T operator()(const std::string & in) throw(E) {

		T val (in);

		if (val.has_error()) {
			count_type::incr_bad();
			throw E(val.error());
		}

		count_type::incr_good();

		return val;
	}


}; // class validator

} // namespace cm

#endif //_CM_VALIDATOR_
