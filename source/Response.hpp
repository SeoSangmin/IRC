#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <sstream>
#include <string>
#include <vector>

#include "NumericDefine.hpp"
#define CRLF "\r\n"

class Response {
	private:
		std::string	buffer_;

	public:
		Response(void);
		const std::string&	get_str(void);
		const char * 		get_chr(void);
		void				flush(void);
		size_t				size(void);

		Response& operator << (const char * char_ptr);
		Response& operator << (const std::string str);
		Response& operator << (const int i);
};

#endif
