#ifndef _CM_NET_MEDIA_
#define _CM_NET_MEDIA_

#include <cm/validator.h>

#include <string>
#include <set>

namespace cm {
namespace net {

/*

	  Media type consists of top-level type name and subtype name, which is further structured into
	so-called "trees". Media types can optionally define companion data, known as parameters.

	top-level type name / subtype name [ ; parameters ]
	top-level type name / [ tree. ] subtype name [ +suffix ] [ ; parameters ]

	  The currently registered top-level type names are: application, audio, example, image, message,
	model, multipart, text, video.

	  Subtype name typically consists of a media type name, but it may or must also contain other content,
	such as tree prefix (facet), producer's name, product name or suffix - according the different rules
	in registration trees.

	https://www.iana.org/assignments/media-types/media-types.xhtml

	http://tools.ietf.org/html/rfc4288#section-3.1

	Type and subtype names MUST conform to the following ABNF:

       type-name = reg-name
       subtype-name = reg-name


*/
namespace media {

namespace details {

		/*


		 http://tools.ietf.org/html/rfc4288#section-4.2

       reg-name = 1*127reg-name-chars
       reg-name-chars = ALPHA / DIGIT / "!" /
                       "#" / "$" / "&" / "." /
                       "+" / "-" / "^" / "_"
		*/
	 	inline bool is_reg_name_char(char c) {

		 	// * Uppercase and lowercase English letters (a–z, A–Z) (ASCII: 65–90, 97–122)
		 	if ((c >= 65 && c<= 90) || (c>=97 && c<= 122)) return true;

		 	// * Digits 0 to 9 (ASCII: 48–57)
		 	if (c >= 48 && c<= 57) return true;

			if (c == '!' || c == '#' || c == '$' || c == '&' ||
				 c == '.' || c == '+' || c == '-' || c == '^' || c == '_' )
				return true;

			return false;

		}

		/*
			http://tools.ietf.org/html/rfc2231#section-7

			attribute-char := <any (US-ASCII) CHAR except SPACE, CTLs,
				                  "*", "'", "%", or tspecials>

			tspecials :=  "(" / ")" / "<" / ">" / "@" /
				"," / ";" / ":" / "\" / <">
				"/" / "[" / "]" / "?" / "="
			; Must be in quoted-string,
			; to use within parameter values


		 */

		inline bool is_tspecial(char c) {

			if (c == '(' || c == ')' || c == '<' || c == '>' || c == '@' ||
 			 c == ',' || c == ';' || c == ':' || c == '\\' || c == '"' ||
			 c == '/' || c == '[' || c == ']' || c == '?' || c == '=')
				return true;

			return false;

		}

		inline bool is_attribute_char(char c, bool quoted) {

			/* SPACE and CTLs */
			if (c <= 32 || c == 127)
				return false;

			if ( c == '*' || c == '\'' || c == '%')
				return false;

			if (! quoted && is_tspecial(c))
				return false;

			return true;

		}


}//namespace details


/**
 * @brief
 *
 * http://tools.ietf.org/html/rfc2231#section-7
 */
class attribute : public error_check {

	public:

		/**
		 * @brief
		 *
		 * @param in
		 */
		attribute(const std::string &in) {

			/* Check if empty */
			error_check_assert(in.empty(), "Empty attribute string.");

			_value = in;
			bool quoted = false;

			/* Check if is quoted */
			if (in.front() == '"' && in.back() == '"') {
				error_check_assert(in.size() == 2, "Empty attribute string.");
				_value.erase(_value.begin());
				_value.pop_back();
				quoted = true;
			}

			char previous = '\0';
			size_t pos = 0;

			/* Check if has valid characters including if quoted */
    	  	error_check_assert(

					! std::all_of(

						_value.begin(), _value.end(),

						[&]
						(char &c ){

							++pos;
							if (c == '"' && previous != '\\') return false;
							previous = c;

							return details::is_attribute_char(c, quoted);
						}
					),

					"Invalid characters in attribute string :" + std::to_string(pos+quoted)
					);

		}

		///
		inline const std::string & value() const { return _value; }

	private:
		std::string _value;

};

class value {};


struct parameter {
	const attribute &attr;
	const value     &val;
};



/**
* @brief
*
* //TODO: optional [ ; parameters ]
*/
class type: public error_check {

	private:

		struct level {
			// The list of top level media names
			static const std::set<std::string> top;
			static const std::set<std::string> tree;
			static const std::set<std::string> suffix;
		};

	public:

		type(const std::string &in) : _value(in) {

			/* Check if empty */
			error_check_assert(in.empty(), "Empty media type string.");

			/* Check the top level type */
			size_t type_sep = in.find_first_of('/');
			error_check_assert(type_sep == std::string::npos, "Missing media type '/' (slash) separator.");
			error_check_assert(type_sep == 0, "Missing top level media type.");

			_top = in.substr(0,type_sep);
			error_check_assert(level::top.find(_top) == level::top.end(),
				"Invalid top level type.");


			/* Check the subtype size and format */
			_sub = in.substr(type_sep+1);
			error_check_assert(_sub.empty(), "Empty subtype.");
			error_check_assert(_sub.size() > 127, "Subtype is too big.");

    	  	error_check_assert(
					! std::all_of(_sub.begin(), _sub.end(), details::is_reg_name_char),
					"Invalid characters in subtype."
			);


			size_t tree_sep = in.find_first_of('.');
			size_t suffix_sep = in.find_first_of('+');

			/* Check the suffix if exists */
			if (suffix_sep != std::string::npos) {
				_suffix = std::string(in, suffix_sep+1);
				error_check_assert(level::suffix.find(_suffix) == level::suffix.end(),
					"Invalid suffix.");
			}

			/* Check the subtype tree if exists */
			if (tree_sep != std::string::npos) {

				int min = std::min(tree_sep, suffix_sep);
				_tree = std::string(in, type_sep+1, min-type_sep-1);

				error_check_assert(_tree.empty(),
					"Invalid subtype tree.");

				error_check_assert(level::tree.find(_tree) == level::tree.end(),
					"Invalid subtype tree.");
			}

		}


		inline const std::string & value() const { return _value; }
		inline const std::string & top() const { return _top; }
		inline const std::string & sub() const { return _sub; }
		inline const std::string & tree() const { return _tree; }
		inline const std::string & suffix() const { return _suffix; }

	private:

		std::string _value;
		std::string _top;
		std::string _sub;
		std::string _tree;
		std::string _suffix;

};

const std::set<std::string> type::level::top = {
	"application", "audio", "example", "image", "message", "model", "multipart", "text", "video"
};

// RFC 6838
const std::set<std::string> type::level::tree = {
	"vnd", "prs", "x"
};

// The currently registered suffixes are: (in RFC 6839) +xml, +json, +ber, +der, +fastinfoset, +wbxml, +zip,
// (in RFC 7049) +cbor
const std::set<std::string> type::level::suffix = {
	"xml", "json", "ber", "der", "fastinfoset", "wbxml", "zip", "cbor"
};

}//namespace media
}//namespace net
}//namespace cm

#endif //_CM_NET_MEDIA_
