#pragma once

#include <map>
#include <sstream>

typedef std::function<std::string&(std::string&)> template_cb;

class TemplateString : public std::stringstream
{
public:
	static void registerTemplateCallback(const char *variable, template_cb cb) {
		TemplateString::template_callbacks[std::string(variable)] = cb;
	}
	std::string doTemplateStr();
private:
	std::string in_cache, out_cache;
	static std::map<std::string, template_cb> template_callbacks;
};