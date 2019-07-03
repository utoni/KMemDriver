#include "pch.h"
#include "minitmpl.h"

std::map<std::string, template_cb> TemplateString::template_callbacks;

std::string TemplateString::doTemplateStr() {
	size_t pos = 0;
	in_cache = str();

	out_cache.clear();
	for (auto& key : template_callbacks) {
		while ((pos = in_cache.find(key.first, pos)) != std::string::npos) {
			in_cache.replace(pos, key.first.length(), key.second(out_cache));
			pos += out_cache.length();
		}
	}

	return in_cache;
}