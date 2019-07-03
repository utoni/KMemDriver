#pragma once

#include <sstream>

class TemplateString : public std::stringstream
{
public:
	std::string& doTemplateStr(const std::string& search, const std::string& replace) {
		size_t pos = 0;
		std::string s = this->str();
		while ((pos = s.find(search, pos)) != std::string::npos) {
			s.replace(pos, search.length(), replace);
			pos += replace.length();
		}
		return s;
	}
};