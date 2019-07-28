#include "pch.h"
#include <iostream>
#include <sstream>
#include <KInterface.h>

#include "www.h"
#include "minitmpl.h"

using httplib::Request;
using httplib::Response;

static std::mutex ki_mutex;
static bool ki_running = true;

static const char *host = "127.0.0.1";
static const int port = 8080;
static const std::string template_content = DEFAULT_TEMPLATE;

static void page_root(const Request &req, Response &res)
{
	TemplateString ss;

	ss << template_content;
	res.set_content(ss.doTemplateStr(), "text/html");
}

static std::string& template_test_cb(std::string &out, void *user_ptr)
{
	out.append("--- TEST ---");
	return out;
}

static std::string& template_status_cb(std::string &out, void *user_ptr)
{
	KInterface &ki = KInterface::getInstance();
	try {
		ki.getBuffer();
	}
	catch (std::runtime_error &) {
		out.append(STATUS_OFFLINE);
		return out;
	}
	out.append(STATUS_ONLINE);
	return out;
}

void kernel_communication_thread(void) {
	std::cout << "Kernel Interface Thread Init.." << std::endl;

	while (ki_running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(2222));
		std::cout << "Kernel Interface Thread Heartbeat.." << std::endl;
	}
}

int main()
{
	KInterface &ki = KInterface::getInstance();
	httplib::Server httpServer;
	std::thread ki_thread(kernel_communication_thread);

	TemplateString::registerTemplateCallback("<% CONTENT %>", template_test_cb);
	TemplateString::registerTemplateCallback("<% STATUS %>", template_status_cb);

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

	ki_running = false;
	ki_thread.join();
}