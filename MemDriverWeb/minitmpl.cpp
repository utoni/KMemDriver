#include "pch.h"
#include "minitmpl.h"

std::map<std::string, std::pair<template_cb, std::pair<bool, void *>>> TemplateString::template_callbacks;

std::string TemplateString::doTemplateStr() {
	size_t pos;
	in_cache = str();

	for (auto& key : template_callbacks) {
		pos = 0;
		while ((pos = in_cache.find(key.first, pos)) != std::string::npos) {
			in_cache.replace(pos, key.first.length(), key.second.first(out_cache, key.second.second.second));
			if (!key.second.second.first ) {
				pos += out_cache.length();
			}
			out_cache.clear();
		}
	}

	return in_cache;
}