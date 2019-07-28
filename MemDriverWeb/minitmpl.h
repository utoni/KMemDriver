#pragma once

#include <map>
#include <sstream>

typedef std::function<std::string&(std::string&, void *)> template_cb;

class TemplateString : public std::stringstream
{
public:
	static void registerTemplateCallback(const char *variable, template_cb cb, void *user_ptr = NULL, bool recursive = false) {
		TemplateString::template_callbacks[std::string(variable)] = std::pair<template_cb, std::pair<bool, void *>>(cb, std::pair<bool, void *>(recursive, user_ptr));
	}
	std::string doTemplateStr();
private:
	std::string in_cache, out_cache;
	static std::map<std::string, std::pair<template_cb, std::pair<bool, void *>>> template_callbacks;
};