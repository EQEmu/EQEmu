
#pragma once

#include "common/http/httplib.h"

class SidecarApi {
public:
	static void BootWebserver(int req = 0, const std::string& key = "");
	static void AuthMiddleware(const httplib::Request &req, const httplib::Response &res);
	static void RequestLogHandler(const httplib::Request &req, const httplib::Response &res);
	static void TestController(const httplib::Request &req, httplib::Response &res);
	static void LootSimulatorController(const httplib::Request &req, httplib::Response &res);
	static void MapBestZController(const httplib::Request &req, httplib::Response &res);
};
