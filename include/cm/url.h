#ifndef _CM_URL
#define _CM_URL

#include <string>
#include <iterator>
#include <array>
#include <algorithm>
#include <iostream>
#include <utility>
#include <cstring>

#include <cm/validator.h>
#include <cm/smtp.h>


/**
 * @namespace cm::url
 * @brief Classes for URL


 RFC 3986 and http://www.w3.org/Addressing/URL/uri-spec.html.
 http://en.wikipedia.org/wiki/URI_scheme#Generic_syntax

 === GENERIC SYNTAX ===

  foo://username:password@example.com:8042/over/there/index.dtb?type=animal&name=narwhal#nose
  \_/   \_______________/ \_________/ \__/            \___/ \_/ \______________________/ \__/
   |           |               |       |                |    |            |                |
   |       userinfo           host    port              |    |          query          fragment
   |    \________________________________/\_____________|____|/ \__/        \__/
 scheme                 |                          |    |    |    |          |
  name              authority                      |    |    |    |          |
   |                                             path   |    |    interpretable as keys
   |                                                    |    |
   |    \_______________________________________________|____|/       \____/     \_____/
   |                         |                          |    |          |           |
 scheme              hierarchical part                  |    |    interpretable as values
  name                                                  |    |
   |            path               interpretable as filename |
   |   ___________|____________                              |
  / \ /                        \                             |
  urn:example:animal:ferret:nose               interpretable as extension

                path
         _________|________
 scheme /                  \
  name  userinfo  hostname       query
  _|__   ___|__   ____|____   _____|_____
 /    \ /      \ /         \ /           \
 mailto:username@example.com?subject=Topic


 */
namespace cm {
namespace url {

namespace details {

// List of allowed URL characters

/* May be encoded but it is not necessary */
inline bool is_unreserved(char c) {

	// * Uppercase and lowercase English letters (a–z, A–Z) (ASCII: 65–90, 97–122)
	if ((c >= 65 && c<= 90) || (c>=97 && c<= 122)) return true;

	// * Digits 0 to 9 (ASCII: 48–57)
	if (c >= 48 && c<= 57) return true;

	if (c == '.' || c == '-' || c == '_' || c == '~')
		return true;

	return false;

}

/* Have to be encoded sometimes */
/* TODO: not used */
inline bool is_reserved(char c) {
	if (  c == '!' || c == '*' || c == '\'' || c == '(' ||
			c == ')' || c == ';' || c == ':' || c == '@' ||
			c == '&' || c == '=' || c == '+' || c == '$' ||
			c == ',' || c == '/' || c == '?' || c == '%' ||
			c == '#' || c == '[' || c == ']'
		)
		return true;

	return false;

}

/*	  sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "=" */
inline bool is_sub_delims(char c) {

	if (  c == '!' || c == '$' || c == '&' || c == '\'' ||
			c == '(' || c == ')' || c == '*' || c == '+' ||
			c == ',' || c == ';' || c == '='
		)
		return true;

	return false;
}

/* TODO: to move this helper functions to elsewhere */

bool icompare_pred(unsigned char a, unsigned char b) {
	return std::tolower(a) != std::tolower(b);
}


bool icase_prefix(const std::string &in, const std::string &prefix) {

	if (in.size() < prefix.size())
		return false;

	std::string::const_iterator stop = in.begin();
	std::advance(stop, prefix.size());

	return std::lexicographical_compare(in.begin(), stop,
			prefix.begin(), prefix.end(), icompare_pred);
}

}


namespace syntax {

/**
 * @brief
 *
 *  userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
 */
class userinfo : public error_check {


	public:

		/// A pointer for a userinfo object
		typedef std::shared_ptr<userinfo> ptr;

		userinfo(const userinfo &other) {
			_value = other._value;
		}

		userinfo(const std::string &in) {

			if (in.empty())
				return;

			for (size_t i=0; i < in.size() ; ++i) {
				if (! is_valid_char(in,i)) {
					if (! has_error())
					 	set_error( "Invalid character in userinfo at position " +
								std::to_string(i) );
					return;
				}
			}

			/* Make a copy to value if not empty */
			if (! in.empty())
				_value = in;

		}

	 	inline bool is_valid_char(const std::string &in, size_t &pos) {

			char c = in[pos];

			/* special isolated allowed chars */
			if (c == ':' ) return true;

			/* unreserved */
			if (details::is_unreserved(c)) return true;

			/* sub-delims */
			if (details::is_sub_delims(c))
				return true;

   	 	// * Percentile encoding i.e. %20
		 	if (c == '%') {

		 	 	// Not enough space for a percentile encdoing. Too close to local part end
		 	 	if ( pos  > (in.size() - 2) ) {
		 	 		//TODO: to re-check this

		 	 		set_error ("Percentile encoding too late in non_hier at position "
		 	 				+ std::to_string(pos) );

					return false;
		 	 	}

		 	 	// Not hex chars right afer percent sign
		 	 	if ( ! std::isxdigit(in[pos+1])  || ! std::isxdigit(in[pos+2]) ) {
		 	 		set_error("Bad Percentile encoding in non_hier at position "
		 	 				+ std::to_string(pos) );
					return false;
		 	 	}

				return true;

		 	}

		 	return false;

		}


	 	static ptr clone(const userinfo& in) {
			return ptr(new userinfo(in));
	 	}

		inline const std::string &value() const { return _value; }

	private:
		std::string _value;

};//class userinfo

/*


	path          = path-abempty    ; begins with "/" or is empty
   / path-absolute   ; begins with "/" but not "//"
   / path-noscheme   ; begins with a non-colon segment
   / path-rootless   ; begins with a segment
   / path-empty      ; zero characters

   path-abempty  = *( "/" segment )
   path-absolute = "/" [ segment-nz *( "/" segment ) ]
   path-noscheme = segment-nz-nc *( "/" segment )
   path-rootless = segment-nz *( "/" segment )
   path-empty    = 0<pchar>

   segment       = *pchar
   segment-nz    = 1*pchar
   segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
   ; non-zero-length segment without any colon ":"

   pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"

 */
class path : public error_check {


	public:

		/// A pointer for a path object
		typedef std::shared_ptr<path> ptr;

		path(const path &other) {
			_value = other._value;
		}

		path(const std::string &in) {

			if (in.empty())
				return;

			error_check_assert(in.front() != '/', "Path does not begin with a '/' (slash) character.");

			for (size_t i=1; i < in.size() ; ++i) {
				if (! is_valid_char(in,i)) {
					if (! has_error())
					 	set_error( "Invalid character in non_hier at position " +
								std::to_string(i) );
					return;
				}
			}

			/* Make a copy to value if not empty */
			if (! in.empty())
				_value = in;

		}

		inline bool is_valid_char(const std::string &in, size_t &pos) {

			char c = in[pos];

			/* special isolated allowed chars */
			if (c == ':' || c == '@' ) return true;

			/* unreserved */
			if (details::is_unreserved(c)) return true;

			/* sub-delims */
			if (details::is_sub_delims(c))
				return true;

			// * Percentile encoding i.e. %20
			if (c == '%') {

				// Not enough space for a percentile encdoing. Too close to local part end
				if ( pos  > (in.size() - 2) ) {
					//TODO: to re-check this

					set_error ("Percentile encoding too late in non_hier at position "
							+ std::to_string(pos) );

					return false;
				}

				// Not hex chars right afer percent sign
				if ( ! std::isxdigit(in[pos+1])  || ! std::isxdigit(in[pos+2]) ) {
					set_error("Bad Percentile encoding in non_hier at position "
							+ std::to_string(pos) );
					return false;
				}

			}

			return false;

		}


		static ptr clone(const path& in) {
			return ptr(new path(in));
		}

		inline const std::string &value() const { return _value; }

	private:
		std::string _value;

};//class path

/**
 * @brief
 *
 *
 *	query/fragment    = *( pchar / "/" / "?" )
 *	pchar             = unreserved / pct-encoded / sub-delims / ":" / "@"
 *	unreserved        = ALPHA / DIGIT / "-" / "." / "_" / "~"
 *	pct-encoded       = "%" HEXDIG HEXDIG
 *	sub-delims        = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
 *
 */
class non_hier : public error_check {

	public:

		/// A pointer for a non_hier object
		typedef std::shared_ptr<non_hier> ptr;

		non_hier(const non_hier &other) {
			_value = other._value;
		}

		non_hier(const std::string &in) {

			if (in.empty())
				return;

			for (size_t i=1; i < in.size() ; ++i) {
				if (! is_valid_char(in,i)) {
					if (! has_error())
						set_error( "Invalid character in non_hier at position " +
								std::to_string(i) );

					return;
				}
			}

			_value = in;

			/* Removes the hash ('#') character from the value */
			if (! _value.empty() )
				_value.erase(0,1);

		}


		inline bool is_valid_char(const std::string &in, size_t &pos) {

			char c = in[pos];

			/* special isolated allowed chars */
			if (c == '/' || c == '?' || c == ':' || c == '@' ) return true;

			/* unreserved */
			if (details::is_unreserved(c)) return true;

			/* sub-delims */
			if (details::is_sub_delims(c))
				return true;

			// * Percentile encoding i.e. %20
			if (c == '%') {

				// Not enough space for a percentile encdoing. Too close to local part end
				if ( pos  > (in.size() - 2) ) {
					//TODO: to re-check this

					set_error ("Percentile encoding too late in non_hier at position "
							+ std::to_string(pos) );

					return false;
				}

				// Not hex chars right afer percent sign
				if ( ! std::isxdigit(in[pos+1])  || ! std::isxdigit(in[pos+2]) ) {
					set_error("Bad Percentile encoding in non_hier at position "
							+ std::to_string(pos) );
					return false;
				}

				return true;

			}

			return false;

		}


		static ptr clone(const non_hier& in) {
			return ptr(new non_hier(in));
		}

		inline const std::string &value() const { return _value; }

	private:
		std::string _value;

};//class non_hier

class fragment : public non_hier {
	using non_hier::non_hier;

};

class query : public non_hier {
	using non_hier::non_hier;
};



/* Parsing URL syntaxes without the scheme (protocol) portion*/
enum separator {
	PATH = 0,
	QUERY = 1,
	FRAGMENT = 2,
	COLON = 3,
	AT = 4
};

constexpr static const char  separator_chars[] = { '/', '?', '#', ':', '@' };

struct separators {

	struct {
		std::array<size_t, sizeof(separator_chars)> positions;
	} first;

	static_assert( COLON != ( sizeof(separator_chars)   ), "separator / chars  mismatch");

	separators(const std::string &in) {

		size_t npos = std::string::npos;

		first.positions = { npos };

		first.positions[PATH]     =  in.find_first_of(separator_chars[PATH]);
		first.positions[QUERY]    =  in.find_first_of(separator_chars[QUERY]);
		first.positions[FRAGMENT] =  in.find_first_of(separator_chars[FRAGMENT]);
		first.positions[AT]       =  in.find_first_of(separator_chars[AT]);
		first.positions[COLON]    =  in.find_first_of(separator_chars[COLON]);

		if (has(AT) &&  first.positions[COLON] < first.positions[AT]) {
			/* find next colon - must be port */
			first.positions[COLON] = in.find_first_of(separator_chars[COLON], (first.positions[COLON] + 1));
		}

	}

	inline bool has(separator s) const {  return first.positions[s] != std::string::npos; }
	inline size_t position(separator s) const {  return first.positions[s]; }

	inline int diff(separator lhs, separator rhs) const {

		if (! has(lhs) && ! has(rhs))
			return 0;

		if (! has(lhs))
			return ( 0 - first.positions[rhs] );

		if (! has(rhs))
			return 0;

		return ( first.positions[rhs] - first.positions[lhs]) ;
	}



};//struct separators

/**
 * @brief 'cid' URI syntax
 *
 * http://tools.ietf.org/html/rfc2392
 *
 * cid:foo4*foo1@bar.net
 */
class cid : public error_check {

	/* syntax parts */
	struct  {
		cm::smtp::address::ptr  _address     = nullptr;
	} parts;

	public:

	cid(const std::string &in) {

		error_check_assert(in.empty(), "Empty cid syntax.");

		cm::smtp::address e(in);

		/* Invalid email address */
		error_check_assert(e.has_error(), e.error());

		/* Valid email address. Save it. */

		parts._address = cm::smtp::address::clone(in);

	}

};//class cid


/**
 * @brief 'mailto' URI syntax
 *
 * http://tools.ietf.org/html/rfc6068
 *
 * mailto:jsmith@example.com?subject=A%20Test
 */

class mailto : public error_check {

	/* syntax parts */
	struct  {
		cm::smtp::address::ptr  _address     = nullptr;
		query::ptr               _query       = nullptr;

	} parts;

	public:

	mailto(const std::string &in) {

		error_check_assert(in.empty(), "Empty mailto syntax.");

		size_t query_pos = in.find_last_of(separator_chars[QUERY]);

		if (query_pos != std::string::npos) {
			cm::range r (in, query_pos);

			query q(r);

			/* Query with error */
			error_check_assert(q.has_error(), q.error());

			/* Valid query. Save it.*/
			parts._query = query::clone(q);
		}

		std::string addr = in;
		if (query_pos != std::string::npos) {
			addr.erase(query_pos);
		}

		//if (parts._query.get())
		//	std::cerr << "    Query => ["<< parts._query->value() << "]" << std::endl;
		//else
		//	std::cerr << "    Query => []" << std::endl;

		cm::smtp::address e(addr);

		/* Invalid email address */
		error_check_assert(e.has_error(), e.error());

		/* Valid email address. Save it. */

		parts._address = cm::smtp::address::clone(addr);

	}

};//class mailto


/**
 * @brief generic URL syntax

 Generic syntax - used by HTTP
 */
class generic : public error_check {

	private:

		/* syntax parts */
		struct  {

			struct {
				userinfo::ptr      _userinfo    = nullptr;
				std::string         _host       = "";
				unsigned short int  _port       = 0;
			} authority;

			path::ptr              _path       = nullptr;
			query::ptr             _query      = nullptr;
			fragment::ptr          _fragment   = nullptr;

		} parts;

	public:

		/**
		 * @brief
		 *
		 * @param in
		 *
		 * L <- R
		 *
		 */
		generic(const std::string &in) {

			/* Empty after scheme URL syntax */
			error_check_assert(in.empty(), "Empty generic URL.");

			/* Empty authority part */
			error_check_assert( (
						in.size() == 1 &&
						( in.front() == '/' || in.front() == '#' || in.front() =='?')) ,
					"Empty authority part.");

			/* Gets the separators last position */
			separators sep(in);

			/* Checks fragment part*/
			if (sep.has(FRAGMENT)) {

				fragment f(cm::range(in, sep.position(FRAGMENT)));

				/* Fragment with error */
				error_check_assert(f.has_error(), f.error());

				/* Valid fragment. Save it.*/
				parts._fragment = fragment::clone(f);

			}

			/* If the query appears first than the fragment*/
			int diff_f = sep.diff(QUERY, FRAGMENT);

			/* Check the query part */
			if (sep.has(QUERY) && diff_f >= 0) {
				query q(cm::range(in, sep.position(QUERY), diff_f));

				/* Query with error */
				error_check_assert(q.has_error(), q.error());

				/* Valid query. Save it.*/
				parts._query = query::clone(q);
			}


			/* The distance of the path to the fragment */
			diff_f = sep.diff(PATH, FRAGMENT);
			/* The distance of the path to the query */
			int diff_q = sep.diff(PATH, QUERY);
			int min_diff = std::min(diff_q, diff_f);

			/* If the path appears first than the non hierarchical parts*/
			/* Check the path part */
			if (sep.has(QUERY) && min_diff >= 0) {
				path p(cm::range(in, sep.position(PATH), min_diff));

				/* Path with error */
				error_check_assert(p.has_error(), p.error());

				/* Valid path. Save it.*/
				parts._path = path::clone(p);
			}


			// foo://username:password@example.com:8042/over/there/index.dtb?type=animal&name=narwhal#nose
			/* Get the authority part range */
			cm::range auth_range(in,
					0,
					std::min({ sep.position(PATH), sep.position(QUERY), sep.position(FRAGMENT)})
					);

			/* Check how many '@' until we reach the path */
			/* If the authority part has a '@' (at sign) then we must have the userinfo part */
			bool has_userinfo = std::count(auth_range.begin(), auth_range.end(), separator_chars[AT]);

			/* Check the user info part */
			if (has_userinfo) {
				userinfo u(cm::range(in, 0, sep.position(AT)));

				/* Userinfo with error */
				error_check_assert(u.has_error(), u.error());

				/* Valid userinfo. Save it. */
				parts.authority._userinfo = userinfo::clone(u);


				auth_range += sep.position(AT) + 1;
			}

			/* Check the host */

			// https://www.ietf.org/rfc/rfc2732.txt -   Format for Literal IPv6 Addresses in URL's

			/* if literal host  */
			if(auth_range.front() == '[') {

				range::iterator it = std::find(auth_range.begin(), auth_range.end(), ']');
				error_check_assert(it == auth_range.end() , "Invalid literal host.");

				size_t literal_sz = ( std::distance(auth_range.begin(), it) + 1);

				std::string addr = auth_range;
				addr.erase(literal_sz);

				/* Check the IP literal value */
				net::ip_literal_facade f(addr, false);
				error_check_assert(f.has_error() , f.error());

				/* Valid host. Save it. */
				parts.authority._host = addr;

				auth_range += literal_sz;

				/* Check port */
				if (auth_range.front() == ':') {

					error_check_assert(! auth_range.has_next() , "Invalid empty port.");

					++auth_range;

					cm::net::port p(auth_range);

					/* Port with error */
					error_check_assert(p.has_error(), p.error());

					/* Valid port. Save it */
					parts.authority._port = p.value();

				}

			}


			/* If non literal host has port */
			range::iterator it = std::find(auth_range.begin(), auth_range.end(), ':');
			size_t host_sz = auth_range.size();

			bool has_port = false;

			/* Found port separator */
			if (it != auth_range.end()) {
				host_sz = std::distance(auth_range.begin(), it);
				has_port = true;
			}

			std::string addr = auth_range;
			if (has_port)
				addr.erase(host_sz);

			dns::domain h(addr);

			/* Host with error */
			error_check_assert(h.has_error(), h.error());

			/* Valid host. Save it. */
			parts.authority._host = addr;

			if (has_port) {
				auth_range += host_sz;

				/* Check port */
				if (auth_range.front() == ':') {

					error_check_assert(! auth_range.has_next() , "Invalid empty port.");

					++auth_range;

					cm::net::port p(auth_range);

					/* Port with error */
					error_check_assert(p.has_error(), p.error());

					/* Valid port. Save it */
					parts.authority._port = p.value();

				}
			}

		}

		friend std::ostream & operator<<  ( std::ostream &os, const generic & generic);

	private:

		generic() = delete;


};//class generic

std::ostream & operator<<  ( std::ostream &os, const generic & generic) {

	if (generic.parts.authority._userinfo.get())
		os << " Userinfo => ["<< generic.parts.authority._userinfo->value() << "]" << std::endl;
	else
		os << " Userinfo => []" << std::endl;

	os << "     Host => ["<< generic.parts.authority._host << "]" << std::endl;
	os << "     Port => ["<< generic.parts.authority._port << "]" << std::endl;

	if (generic.parts._path.get())
		os << "     Path => ["<< generic.parts._path->value() << "]" << std::endl;
	else
		os << "     Path => []" << std::endl;

	if (generic.parts._query.get())
		os << "    Query => ["<< generic.parts._query->value() << "]" << std::endl;
	else
		os << "    Query => []" << std::endl;

	if (generic.parts._fragment.get())
		os << " Fragment => ["<< generic.parts._fragment->value() << "]" << std::endl;
	else
		os << " Fragment => []" << std::endl;

	return os ;
}

}//namespace syntax

/// The enumerated list of allowed schemes ( "protocols") URL's
enum schemes {
	HTTP = 0,
	HTTPS = 1,
	FTP = 2,
	CAP = 3,
	NFS = 4,
	MAILTO = 5,
	CID =6,
	UNDEF
};

/// The null terminated list of names for URL's schemes ( "protocols").
constexpr static const char *  schemes_names[] = {
	"http",
	"https",
	"ftp",
	"cap",
	"nfs",
	"mailto",
	"cid",
	nullptr
};


/**
 * @brief
 */
class scheme : public error_check {

	public:

		typedef std::shared_ptr<scheme> ptr;

		static ptr create(const std::string & in) {
			return ptr(new scheme(in));
		}

		operator std::string() const { return value(); }
		operator schemes() const { return id(); }
		inline const std::string & value() const { return _value; }
		inline schemes id() const { return _id; }

	private:

		scheme(const std::string &name) : _id(UNDEF) {

			constexpr int equal = 0;

			std::string s(name);
			std::transform(s.begin(), s.end(), s.begin(), ::tolower);


			for(size_t i = 0; i < (UNDEF - 1); ++i) {

				if (::strcmp(s.c_str(),schemes_names[i]) == equal ) {
					_value = s;
					_id = static_cast<schemes>(i);
					return;
				}
			}

			set_error("Scheme type not found");

		}

		friend std::ostream & operator<<  ( std::ostream &os, const scheme & scheme);

		std::string _value;
		schemes _id;

};//class scheme

std::ostream & operator<<  ( std::ostream &os, const scheme & scheme) {
	return os << scheme.value() << ": (" << scheme.id() << ")";
}

template < schemes s =  url::UNDEF, class S = syntax::generic >
class resource : public error_check {

	static_assert( UNDEF != ( sizeof(schemes_names) / sizeof (const char *) ),
			"schemes / schemes names mismatch");

	static_assert(std::is_base_of<error_check, S>::value, "S must be a descendant of error_check");

	public:

	/// The validator type for an address
	///TODO: typedef cm::validator< address, exceptions::invalid_email_address> validator_type;
	///TODO: typedef cm::validator< address, exceptions::invalid_email_address, true> validator_count_type;

	resource(const std::string &in) {

		/* Check if empty */
		error_check_assert(in.empty(), "Empty URL string.");

		size_t pos_colon = in.find_first_of(':');
		error_check_assert(pos_colon == std::string::npos, "Missing ':' (colon) character.");


		std::string name;
		if (s != UNDEF )
			name 	= schemes_names[s];
		else
			name = in.substr(0, pos_colon );

		/* scheme + : */
		size_t idx = name.size();
		error_check_assert( in.size() <= ( idx + 1 ) , "URL too small.");

		/* TODO: What about the maximum length ? */
		// http://stackoverflow.com/questions/417142/what-is-the-maximum-length-of-a-url-in-different-browsers

		/* Check scheme */
		_scheme = scheme::create(name);

		error_check_assert(_scheme.get() == nullptr , "Invalid scheme.");
		error_check_assert(_scheme->has_error() , "Invalid scheme. " + _scheme->error());

		/* Check the scheme separator */
		if (in[idx++] != ':'  ) {
			set_error("Invalid scheme separator.");
			return;
		}

		/* Trim '/' ( Slashes) if needed */

		if (in[idx] == '/' ) ++idx;
		if (in[idx] == '/' ) ++idx;

		/* Checks this resource syntax */

		S syn(std::string(in, idx));
		error_check_assert( syn.has_error() , syn.error());

		/* std::cerr << syn << std::endl; */

		/* Save input as value for later access*/
		_value = in;

	}

	inline const std::string & value() const { return _value; }
	inline const cm::url::scheme & scheme() const { return *_scheme; }

	private:
	std::string _value;
	scheme::ptr _scheme;

};//template class resource

typedef resource<url::HTTP>                     http;
typedef resource<url::HTTPS>                    https;
typedef resource<url::FTP>                      ftp;
typedef resource<url::CAP>                      cap;
typedef resource<url::NFS>                      nfs;

typedef resource<url::MAILTO, syntax::mailto>   mailto;
typedef resource<url::CID, syntax::cid>         cid;

//TODO: to change to functor?
class factory {

	public:
		static std::shared_ptr< resource <  > > create(const std::string &in) {

			auto ret = std::shared_ptr<resource <> >(new resource<>(in));

			return ret;
		}

};



}// namespace url
}// namespace cm

#endif // _CM_URL
