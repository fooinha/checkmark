#ifndef _CM_SMTP
#define _CM_SMTP

#include <string>
#include <stdexcept>
#include <memory>
#include <sstream>
#include <algorithm>
#include <vector>

#include <mutex>
#include <thread>

#include <cm/validator.h>
#include <cm/domain.h>

namespace cm {

/**
 * @namespace cm::smtp
 * @brief Classes for SMTP protocol addresses
 */
namespace smtp {

	namespace exceptions {

	/**
	 * @class invalid_email_address
	 * @brief An exception class to indicate that an address could not be construted because
	 *        of an invalid input.
	 */
	class invalid_email_address : public std::invalid_argument {
		// C++11 inheriting constructors
		using invalid_argument::invalid_argument;
	};

	} //namespace exceptions


	/**
	* @class local_part
	* @brief Represents the left hand side of the "@" in an email address.
	*
	* Does not throw any exception in case of invalid arguments.

	* Caller must check the local_part::has_error().

	* Only instances with a valid syntax (defined on SMTP related RFC documents) will be constructed.


	- The local-part of the email address may use any of these ASCII characters.[4] RFC 6531 permits Unicode
	characters beyond the ASCII range:
	- Uppercase and lowercase English letters (a–z, A–Z) (ASCII: 65–90, 97–122)
	- Digits 0 to 9 (ASCII: 48–57)
	- These special characters: - _ ~ ! $ & ' ( ) * + , ; = : and percentile encoding i.e. %20
	- Character . (dot, period, full stop) (ASCII: 46) provided that it is not the first or last character,
 	 and provided also that it does not appear consecutively (e.g. John..Doe\@example.com is not allowed).

	- Special characters are allowed with restrictions. They are:
	- Space and "(),:;<>@[\] (ASCII: 32, 34, 40, 41, 44, 58, 59, 60, 62, 64, 91–93)

	- The restrictions for special characters are that they must only be used when contained between
	quotation marks, and that 2 of them (the backslash \ and quotation mark " (ASCII: 92, 34)) must also
	be preceded by a backslash \ (e.g. "\\\"").

	- Comments are allowed with parentheses at either end of the local part;
 	 e.g. "john.smith(comment)\@example.com" and "(comment)john.smith\@example.com" are both
 	 equivalent to "john.smith\@example.com".

	- International characters above U+007F, encoded as UTF-8, are permitted by RFC 6531, though mail
 	 systems may restrict which characters to use when assigning local parts.

	- A quoted string may exist as a dot separated entity within the local-part, or it may exist when
	the outermost quotes are the outermost characters of the local-part (e.g. abc."defghi".xyz\@example.com or
	"abcdefghixyz"\@example.com are allowed. Conversely, abc"defghi"xyz\@example.com is not;
	neither is abc\"def\"ghi\@example.com). Quoted strings and characters however, are not commonly used.
	RFC 5321 also warns that "a host that expects to receive mail SHOULD avoid defining mailboxes where the
	Local-part requires (or uses) the Quoted-string form".

	- The local-part postmaster is treated specially–it is case-insensitive, and should be forwarded to the
	domain email administrator. Technically all other local-parts are case-sensitive, therefore
	jsmith\@example.com and JSmith\@example.com specify different mailboxes; however, many organizations treat
	uppercase and lowercase letters as equivalent.


	*/
	class local_part : public error_check {

 	 public:


	 	 /// The underlying type for the local part value
	 	 typedef std::string                 value_type;

	 	 /// A pointer for a local part object
	 	 typedef std::shared_ptr<local_part> ptr;

	 	 /// Maximum size allowed for an email's local part
	 	 static constexpr size_t             max_size = 64;

	 	 /**
	  	  * @brief Local part object factory method
	  	  *
	  	  * Allocates and creates a pointer to local_part object
	  	  *
	  	  * @param in The input argument
	  	  *
	  	  * @return The shared pointer
	  	  */
	 	 static ptr create(const std::string & in) {
		 	 return ptr(new local_part(in));
	 	 }

	 	 /**
	  	  * @brief Gets the value used to create the local_part object.
	  	  *
	  	  * @return The original underlying string value for the email's local part
	  	  */
	 	 inline const value_type & value() const { return _value; }



 	 private:

	 	 // Parse state for dotted/quoted/commented addresses
	 	 struct parse_state {
		 	 bool lc:1;     // left hand side comment
		 	 bool rc:1;     // right hand side comment
		 	 bool qt:1;     // in quote
		 	 int lqt;       // last quote
		 	 char prev;     // prev
		 	 unsigned err;  // err
		 	 size_t pos;    // pos

		 	 explicit parse_state() {
			 	 lc = false;
			 	 rc = false;
			 	 qt = false;
			 	 prev = '\0';
			 	 err = 0;
			 	 pos = 0;
			 	 lqt = -1;
		 	 }

	 	 };

	 	 /**
	  	  * @brief Checks if a character is a delimiter for dotted/quoted/commented addresses
	  	  *
	  	  *
	  	  *
	  	  * - dotted form  :    john.doe\@example.com
	  	  * - quoted form  :    john."common".doe\@example.com
	  	  * - comment form :    john.doe(Doo)\@example.com
	  	  *
	  	  *
	  	  * @param c The input character to evaluate
	  	  *
	  	  * @return The boolean result.
	  	  */
	 	 inline bool is_delim(char c) const {
		 	 // . <- most common
		 	 return ( c== '.' || c == '"' ||  c == '(' );
	 	 }

	 	 // * Space and "(),:;<>@[\] (ASCII: 32, 34, 40, 41, 44, 58, 59, 60, 62, 64, 91–93)
	 	 /**
	  	  * @brief Checks if a character belongs to the restricted class and must appear only between
	  	  * quotation marks
	  	  *
	  	  * @param c The input character to evaluate
	  	  *
	  	  * @return The boolean result.
	  	  */
	 	 inline bool is_special_restricted(char c) {

		 	 return (
				 	 c == ' ' || c == '"' ||
				 	 c == '(' || c == ')' ||
				 	 c == ',' || c == ':' || c == ';' ||
				 	 c == '<' || c == '>' || c == '@' ||
				 	 c == '[' || c == '\\' || c == ']');
	 	 }



	 	 /**
	  	  * @brief Verifies if a character is valid in the context of a position

			- Uppercase and lowercase English letters (a–z, A–Z) (ASCII: 65–90, 97–122)
			- Digits 0 to 9 (ASCII: 48–57)
			- These special characters: - _ ~ ! $ & ' ( ) * + , ; = : and
			- Percentile encoding i.e. %20
			- International characters above U+007F, encoded as UTF-8

	  	  *
	  	  * @param addr The input address value
	  	  * @param pos  The current position.

			This position value may advance if we are verifying a multibyte character.
	  	  *
	  	  * @return The boolean result.
	  	  */
	 	 inline bool is_valid_char(const std::string &addr, size_t &pos) {

		 	 char c = addr[pos];

		 	 // * Uppercase and lowercase English letters (a–z, A–Z) (ASCII: 65–90, 97–122)
		 	 if ((c >= 65 && c<= 90) || (c>=97 && c<= 122)) return true;

		 	 // * Digits 0 to 9 (ASCII: 48–57)
		 	 if (c >= 48 && c<= 57) return true;

		 	 if (c == '.' || c == '&' || c == '_' || c == '-' ||
		 		 	 c == '=' || c == '/' || c == '+' ||
		 		 	 c == '$' || c == '\''|| c == '*' ||
		 		 	 c == '#' || c == '!' |
		 		 	 c == '?' || c == '`' || c == '{' || c == '}' ||
		 		 	 c == '|' || c == '~' || c == '^' ||
				 	 c == '%' //TODO: to check % encoding
 	  		 	 )
			 	 return true;

		 	 /* TODO: to check rules for this . When quoted and otherwise.
   	 	 // * Percentile encoding i.e. %20

		 	 if (c == '%') {

		 	 // Not enough space for a percentile encdoing. Too close to local part end
		 	 if ( pos  > ( addr.size() - 2) ) {
		 	 //TODO: to re-check this
		 	 throw invalid_address("Percentile encoding at local part too late at position "
		 	 + std::to_string(pos) );
		 	 }

		 	 // Not hex chars right afer percent sign
		 	 if ( ! std::isxdigit(addr[pos+1])  || ! std::isxdigit(addr[pos+2]) ) {
		 	 throw invalid_address("Bad Percentile encoding at local part at position "
		 	 + std::to_string(pos) );
		 	 }

		 	 return true;
		 	 }
		  	  */

		 	 // * International characters above U+007F, encoded as UTF-8
		 	 unsigned u = (unsigned ) (c & 0x000000ff);

		 	 if (u > 0x7f) {

			 	 /* http://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c */
			 	 /* http://www.w3.org/International/questions/qa-forms-utf-8 */

   		 	 const unsigned char * bytes = (const unsigned char *) addr.c_str();

      	 	 if( (// non-overlong 2-byte
               	 	 (0xC2 <= bytes[pos] && bytes[pos] <= 0xDF) &&
               	 	 (0x80 <= bytes[pos+1] && bytes[pos+1] <= 0xBF)
          	  	  )
        			) {
         	 	 pos += 1;
         	 	 return true;
      	 	 }

      	 	 if( (// excluding overlongs
               	 	 bytes[pos] == 0xE0 &&
               	 	 (0xA0 <= bytes[pos+1] && bytes[pos+1] <= 0xBF) &&
               	 	 (0x80 <= bytes[pos+2] && bytes[pos+2] <= 0xBF)
          	  	  ) ||
            	 	 (// straight 3-byte
             	  	  ((0xE1 <= bytes[pos] && bytes[pos] <= 0xEC) ||
              			bytes[pos] == 0xEE ||
              			bytes[pos] == 0xEF) &&
             	  	  (0x80 <= bytes[pos+1] && bytes[pos+1] <= 0xBF) &&
             	  	  (0x80 <= bytes[pos+2] && bytes[pos+2] <= 0xBF)
            	 	 ) ||
            	 	 (// excluding surrogates
             	  	  bytes[pos] == 0xED &&
             	  	  (0x80 <= bytes[pos+1] && bytes[pos+1] <= 0x9F) &&
             	  	  (0x80 <= bytes[pos+2] && bytes[pos+2] <= 0xBF)
            	 	 )
        			) {
         	 	 pos += 2;
         	 	 return true;
      	 	 }

      	 	 if( (// planes 1-3
               	 	 bytes[pos] == 0xF0 &&
               	 	 (0x90 <= bytes[pos+1] && bytes[pos+1] <= 0xBF) &&
               	 	 (0x80 <= bytes[pos+2] && bytes[pos+2] <= 0xBF) &&
               	 	 (0x80 <= bytes[pos+3] && bytes[pos+3] <= 0xBF)
          	  	  ) ||
            	 	 (// planes 4-15
             	  	  (0xF1 <= bytes[pos] && bytes[pos] <= 0xF3) &&
             	  	  (0x80 <= bytes[pos+1] && bytes[pos+1] <= 0xBF) &&
             	  	  (0x80 <= bytes[pos+2] && bytes[pos+2] <= 0xBF) &&
             	  	  (0x80 <= bytes[pos+3] && bytes[pos+3] <= 0xBF)
            	 	 ) ||
            	 	 (// plane 16
             	  	  bytes[pos] == 0xF4 &&
             	  	  (0x80 <= bytes[pos+1] && bytes[pos+1] <= 0x8F) &&
             	  	  (0x80 <= bytes[pos+2] && bytes[pos+2] <= 0xBF) &&
             	  	  (0x80 <= bytes[pos+3] && bytes[pos+3] <= 0xBF)
            	 	 )
        			) {
         	 	 pos += 3;
         	 	 return true;
      	 	 }

			 	 return false;
		 	 }

		 	 return false;
	 	 }

	 	 local_part() = delete; // Disables the empty constructor

		/**
		* @brief Constructs an email's local part from a std::string.
		*
		* @param in The input argument.
		*/
	 	 local_part(const std::string &in) : _value(in) {

		 	 /* Size check for local part */
		 	 if (in.empty()) {
			 	 set_error("Empty local part.");
			 	 return;
		 	 }

		 	 if (in.size() > max_size) {
			 	 set_error("Local part too big.");
			 	 return;
		 	 }

		 	 /* No leading space */
		 	 if (std::isspace(in.front())) {
			 	 set_error("Local part with leading whitespace.");
			 	 return;
		 	 }

		 	 /* No trailing space */
		 	 if (std::isspace(in.back())) {
			 	 set_error("Local part with trailing whitespace.");
			 	 return;
		 	 }

		 	 /* Dot (.) at start of local part */
		 	 if (in.front() == '.') {
			 	 set_error("Local part begins with the '.' (Dot) character.");
			 	 return;
		 	 }

		 	 /* At end of local part */
		 	 if (in.back() == '.') {
			 	 set_error("Local part ends with the '.' (Dot) character.");
			 	 return;
		 	 }

		 	 /* The local-part postmaster is treated specially–it is case-insensitive */
		 	 if (in[0] == 'p' || in[0] == 'P')
			 	 if (in[1] == 'o' || in[1] == 'O')
				 	 if (in[2] == 's' || in[2] == 'S')
					 	 if (in[3] == 't' || in[3] == 'T')
						 	 if (in[4] == 'm' || in[4] == 'M')
							 	 if (in[5] == 'a' || in[5] == 'A')
								 	 if (in[6] == 's' || in[6] == 'S')
									 	 if (in[7] == 't' || in[7] == 'T')
										 	 if (in[8] == 'e' || in[8] == 'E')
											 	 if (in[9] == 'r' || in[9] == 'R') {
												 	 return;
											 	 }

		 	 /* Check if it has any dotted/quoted/comment character */
		 	 bool has_delim = false;
		 	 for (auto c : in)
			 	 if ( ( has_delim = is_delim(c)))
				 	 break;


		 	 /* Check for invalid characters if there is not need to process dotted/quoted/comment addresses */
		 	 if (! has_delim) {
			 	 for (size_t i=0; i < in.size() ; ++i) {
				 	 if (! is_valid_char(in,i)) {
					 	  set_error( "Invalid character at local part at position " + std::to_string(i) );
					 	 return;
				 	 }
			 	 }
			 	 return;
		 	 }

		 	 /* Too small starting quoted local part */
		 	 if (in.front() == '"' && in.size() < 3) {
			 	 set_error("Quoted local part to small");
			 	 return;
		 	 }

		 	 /* Parse dotted/quoted/comments */
		 	 // ()xxx@    ; xxx()@    ; "xxxx"@  ;  x.y.z@

		 	 parse_state s;

		 	 for (size_t i = 0; i < in.size() ; ++i) {

			 	 char c = in[i];
			 	 s.pos = i;

			 	 if (i > 0) {

				 	 /* When not quoted */
				 	 if (! s.qt && ! s.lc && ! s.rc) {

					 	 /* And not a quote char */
					 	 if (c != '"') {

						 	 if (! is_valid_char(in, i) &&
								 	 c != '.' &&
								 	 ( s.lc && c != '('  ) && c != '"') {
							 	 s.err = 7;
							 	 break;
						 	 }

						 	 /* And is a special restricted char */
						 	 if (is_special_restricted(c) ) {

							 	 if (c == '(') {

								 	 s.rc = true ; /* rhs comment */
								 	 s.prev = c;
								 	 continue;

							 	 } else if ( s.rc && c == ')') {
								 	 if ( i == (in.size() - 1))
									 	 s.rc = false;
								 	 else
									 	 s.lc = false;

								 	 s.prev = c;
								 	 continue;
							 	 }

							 	 s.err = 2;
							 	 break;
						 	 }

						 	 /* Consecutive Dots(.) */
						 	 if (c == '.' && s.prev == '.') {
							 	 s.err = 3;
							 	 break;
						 	 }

					 	 } else {

						 	 if (s.prev != '.') {
							 	 s.err = 5;
							 	 break;
						 	 }

						 	 s.qt = true;
						 	 s.lqt = i;
					 	 }

				 	 } else {
					 	 /* Inside quotes*/

					 	 /* Consecutive quotes */
					 	 if (c=='"' && s.prev == '"' &&
							 	 ( s.lqt + 1 )  == i ) {
							 	 s.err = 8;
							 	 break;
					 	 }

					 	 /* When escaping */

					 	 /*
					 	 if (c == '\\'){
						 	 char n = in[i+1];
						 	 if (n != '\\' && n != '"') {
							 	 s.err = 6;
							 	 break;

						 	 } else {

							 	 // If found escaped chars then advance a position
							 	 s.prev = n;
							 	 ++i;
							 	 continue;
						 	 }

					 	 } else */ if (c == '"' && s.prev != '\\')  {

						 	 /* Closing quotes */
						 	 s.lqt = i;
						 	 s.qt = false;

					 	 } else{

						 	 if (! is_valid_char(in, i) &&  !is_special_restricted(c)) {
							 	 s.err = 7;
							 	 break;
						 	 }
					 	 }
				 	 }

				 	 s.prev = c;
				 	 continue;
			 	 }

			 	 /* First byte */

			 	 /* Check for special chars */
			 	 if (c == '(')
				 	 s.lc = true;
			 	 else if (c == '"') {
				 	 s.qt = true;
				 	 s.lqt = 0;
			 	 }
			 	 else if (c == 41 || c == 44 ||
					 	 c == 58 || c == 59 ||
					 	 c == 60 || c == 62 || c == 64 ||
					 	 c == 91 || c == 92 || c == 93
					 	 ) {
				 	 /* special not allowed unquoted and at begin - lhs end */
				 	 /* ),:;<>@[\] */
				 	 //41, 44, 58, 59, 60, 62, 64, 91–93
				 	 s.err = 1;
				 	 break;
			 	 }

		 	 }

		 	 if (s.err) {

			 	 std::stringstream ss;
			 	 char c = in[s.pos];

			 	 if (s.err == 1) {
				 	 ss << "Invalid leading restricted special character (" << c <<
					 	 ") [pos: " << s.pos << "]";

			 	 } else if (s.err == 2) {
				 	 ss << "Unquoted restricted special character ("<< c << ") [pos: "<< s.pos <<"]";

			 	 } else if (s.err == 3) {
				 	 ss << "Consecutive unquoted Dot(.) separator ("<< c << ") [pos: "<< s.pos <<"]";

			 	 } else if (s.err == 5) {
				 	 ss << "Not starting quoted without Dot(.) separator ("<< c << ") [pos: "<< s.pos <<"]";

			 	 } else if (s.err == 6) {
				 	 ss << "Quoted quote or backslash not escaped ("<< c << ") [pos: "<< s.pos <<"]";

			 	 } else if (s.err == 7) {
				 	 ss << "Invalid char ("<< c << ") [pos: "<< s.pos <<"]";

			 	 } else if (s.err == 8) {
				 	 ss << "Consecutive quotes ("<< c << ") [pos: "<< s.pos <<"]";

			 	 } else {
				 	 ss << "Unspecified error ("<< c << ") [pos: "<< s.pos <<"]";
			 	 }

			 	 set_error(ss.str());
			 	 return;

		 	 }

		 	 if (s.qt) {
			 	 std::stringstream ss;
			 	 ss << "Unfinished quote ("<< in[s.lqt] << ") [pos: "<< s.lqt <<"]";
			 	 set_error(ss.str());
			 	 return;
		 	 }

		 	 if (s.lc) {
			 	 std::stringstream ss;
			 	 ss << "Comment not finished at lhs local part begin ("<< in[s.pos] << ") [pos: " << s.pos  <<"]";
			 	 set_error(ss.str());
			 	 return;
		 	 }

		 	 if (s.rc) {
			 	 std::stringstream ss;
			 	 ss << "Comment not finished at rhs local part end ("<< in[s.pos] << ") [pos: "<< s.pos <<"]";
			 	 set_error(ss.str());
			 	 return;
		 	 }
	 	 }

	 	 std::string _value;

	}; // class local_part

	/*

	 Valid email addresses
	 --------------------------------------------------------------------------------

		1234567890123456789012345678901234567890123456789012345678901234@example.com
		label.at.limit@123456789012345678901234567890123456789012345678901234567890123.example.com
		niceandsimple@example.com
		very.common@example.com
		very.common(lastcomment)@example.com
		a.little.lengthy.but.fine@dept.example.com
		disposable.style.email.with+symbol@example.com
		other.email-with-dash@example.com
		"much.more unusual"@example.com
		"very.unusual.@.unusual.com"@example.com
		"very.(),:;<>[]\".VERY.\"very@\\ \"very\".unusual"@strange.example.com
		!#$%&'*+-/=?^_`{}|~@example.org
		"()<>[]:,;@\\\"!#$%&'*+-/=?^_`{}| ~.a"@example.org
		" "@example.org
		üñîçøðé@example.com
		üñîçøðé@üñîçøðé.com
		🚀🚁🚂🚃.🚄.🚅.🚆🚨@example.com
		postmaster@example.com
		A%20B@example.com
		"\\\""@example.org
		"\\"."finishedquote"@example.org

		--------------------------------------------------------------------------------

		Invalid email addresses
		--------------------------------------------------------------------------------

    	Abc.example.com (an @ character must separate the local and domain parts)
    	A@b@c@example.com (only one @ is allowed outside quotation marks)
    	a"b(c)d,e:f;g<h>i[j\k]l@example.com (none of the special characters in this local part is allowed
      outside quotation marks)
    	just"not"right@example.com (quoted strings must be dot separated or the only element making
            up the local-part)
    	this is"not\allowed@example.com (spaces, quotes, and backslashes may only exist when within quoted
      strings and preceded by a backslash)
    	this\ still\"not\\allowed@example.com (even if escaped (preceded by a backslash), spaces, quotes,
      and backslashes must still be contained by quotes)
    		john..doe@example.com (double dot before @)
    		john.doe@example..com (double dot after @)
    		a valid address with a leading space
    		a valid address with a trailing space

			--------------------------------------------------------------------------------

			12345678901234567890123456789012345678901234567890123456789012345@example.com
			123456789012345678901234567890123456789012345678901234567890123	@example.com
			.foo.bar@example.com
			Abc.example.com
			A@b@c@example.com
			(unfinishedcomment@example.com
			unfinished(commen@example.com
			a"b(c)d,e:f;g<h>i[j\k]l@example.com
			ab(c)d,e:f;g<h>i[j\k]l@example.com
			abc)d,e:f;g<h>i[j\k]l@example.com
			"unfinishedquote""@example.com
			"unfinishedquote"."@example.com
			backslash."\".notescaped@example.com
			quotes."\ "moar".notescaped@example.com
			space" "notseparated@example.com
			just"not"right@example.com
			this is"not\allowed@example.com
			this\ still\"not\\allowed@example.com
			john.doe@foo&bar.com
			j(ohn.do)e@example.com
			john(bad-middle-comment)e@example..com
			foo@example.com
			foo@example.com   
			0%NO.GOODZ.perc.encoding@example.com
			012%0@example.com
			"\\""@example.org
			"\\"."@example.org
			"\\"."unfinishedquote@example.org
			"\\""not.separatedA.finishedquote"@example.org
			badcharacteratdomain@exampler!.org
			john..doe@example.com
			john.doe@example.-com
			john.doe@example..com
			john.doe@example-.com
			bad.domain.space@ example.com
			label.too.long@1234567890123456789012345678901234567890123456789012345678901234.example.com
			 --------------------------------------------------------------------------------

	 		 Additional stuff: http://en.wikipedia.org/wiki/Talk%3AEmail_address
			 */

	/**
	* @class address
	* @brief Represents a email address spec with a valid syntax
	*
	*
	*	@details
	*
	* The format of email addresses is local-part\@domain where the local-part may be up to 64 characters long and
	* the domain name may have a maximum of 253 characters – but the maximum of 256-character length of a forward
	* or reverse path restricts the entire email address to be no more than 254 characters long.[2]
	* The formal definitions are in RFC 5322 (sections 3.2.3 and 3.4.1) and RFC 5321 – with a more readable
	* form given in the informational RFC 3696[3] and the associated errata.
	*
   * Only instances with a valid syntax (defined on SMTP related RFC documents) will be constructed.
	*
	*  \example email.cpp
	*	\sa address::get_local_part()
	*	\sa address::get_domain()
	*/
	class address : public error_check {
		public:

			/// The validator type for an address
			typedef cm::validator< address, exceptions::invalid_email_address> validator_type;
			typedef cm::validator< address, exceptions::invalid_email_address, true> validator_count_type;

			/// A pointer for an address object
			typedef std::shared_ptr<address> ptr;

			/// The minimum size for an email address
			static constexpr size_t min_size = 3;   // l@d

			/// The maximum size for an email address
			static constexpr size_t max_size = 254;

			/**
			 * @brief Constructs an email's address from a std::string.
			 *
			 * @param in The input argument.
			 */
			address(const std::string &in) {

				/* address cannot be empty */
				if (in.empty()) {
					set_error("Address specification cannot be empty.");
					return;
				}

				/* address size must be inside limits */
				if (in.size() < min_size) {
					set_error("Address specification is too small.");
					return;
				}

				if (in.size() > max_size) {
					set_error("Address specification too big.");
					return;
				}

				size_t at_pos = in.find_last_of('@', ( in.size() - 1 ) );

				if (at_pos == 0) {
					set_error("Address cannot begin with the '@' (at-sign) character.");
					return;
				}

				if (at_pos == std::string::npos) {
					set_error("Missing '@' (at-sign) character.");
					return;
				}

				_lp = local_part::create(in.substr(0, at_pos));
				if (get_local_part().has_error()) {
					set_error(get_local_part().error());
					return;
				}

				_dp = dns::domain::create(in.substr(at_pos + 1));
				if (get_domain().has_error()) {
					set_error(get_domain().error());
					return;
				}

			}

			/**
			 * @brief Gets the local part object of this address.
			 *
			 * @return The local part object instance.
			 *
			 * @throw std::runtime_error if the local part instance is nil.
			 */
			inline const local_part &  get_local_part() const throw(std::runtime_error) {
				if (_lp.get() == nullptr)
					throw std::runtime_error("Missing local part for address");
				return *_lp;
			}

			/**
			 * @brief Gets the domain part object of this address
			 *
			 * @return The domain object instance.
			 *
			 * @throw std::runtime_error if the domain part instance is nil
			 */
			inline const dns::domain &  get_domain() const throw(std::runtime_error) {
				if (_dp.get() == nullptr)
					throw std::runtime_error("Missing domain for address");
				return *_dp;
			}

			/**
			 * @brief Gets the domain part object pointer of this address
			 *
			 * @return The domain object pointer.
			 *
			 * @throw std::runtime_error if the domain part pointer is nil
			 */
			inline const dns::domain::ptr &  get_domain_ptr() const throw(std::runtime_error) {
				if (_dp.get() == nullptr)
					throw std::runtime_error("Missing domain for address");
				return _dp;
			}

			/**
			 * @brief 
			 *
			 * @param in
			 *
			 * @return 
			 */
	 	 	static ptr clone(const address& in) {
				return ptr(new address(in));
	 	 	}


		private:
			address() = delete; // Disables the empty constructor

			local_part::ptr _lp;
			dns::domain::ptr     _dp;


	}; // class address




} // namespace stmp
} // namespace cm

#endif // _CM_SMTP
