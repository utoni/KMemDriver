#include "pch.h"
#include <iostream>
#include <sstream>
#include <KInterface.h>
#include "www.h"
#include "minitmpl.h"

using httplib::Request;
using httplib::Response;

static const char *host = "127.0.0.1";
static const int port = 8080;
static const std::string template_content = DEFAULT_TEMPLATE;

static void page_root(const Request &req, Response &res)
{
	TemplateString ss;

	ss << template_content;
	res.set_content(ss.doTemplateStr(), "text/html");
}

static std::string& template_test_cb(std::string &out)
{
	out.append("--- TEST ---");
	return out;
}

int main()
{
	KInterface &ki = KInterface::getInstance();
	httplib::Server httpServer;

	TemplateString::registerTemplateCallback("<% CONTENT %>", template_test_cb);

    std::cout << "Starting WebServer on " << host << ":" << port << "\n"; 
	httpServer.Get("/", page_root);

	httpServer.set_error_handler([](const Request & req, Response &res) {
		std::cerr << "ERROR " << res.status << ": " << req.method << " " << req.path << std::endl;
		std::stringstream ss;
		ss << "<p>Error Status: <span style='color:red;'>" << res.status << "</span></p>";
		res.set_content(ss.str(), "text/html");
	});
	httpServer.set_logger([](const Request &req, const Response &res) {
		if (res.status == 200)
			std::cout << req.method << " " << req.path << std::endl;
	});
	httpServer.listen(host, port);
}