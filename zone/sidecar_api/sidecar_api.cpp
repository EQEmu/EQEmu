#include "sidecar_api.h"
#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../zonedb.h"
#include "../../common/process.h"
#include "../common.h"
#include "../zone.h"
#include "../client.h"
#include "../../common/json/json.hpp"
#include <csignal>

void CatchSidecarSignal(int sig_num)
{
	LogInfo("[SidecarAPI] Caught signal [{}]", sig_num);
	LogInfo("Forcefully exiting for now");
	std::exit(0);
}

void SidecarApi::RequestLogHandler(const httplib::Request& req, const httplib::Response& res)
{
	if (!req.path.empty()) {
		std::vector<std::string> params;
		for (auto& p : req.params) {
			params.emplace_back(fmt::format("{}={}", p.first, p.second));
		}

		LogInfo(
			"[API] Request [{}] [{}{}] via [{}:{}]",
			res.status,
			req.path,
			(!params.empty() ? "?" + Strings::Join(params, "&") : ""),
			req.remote_addr,
			req.remote_port
		);
	}
}

void SidecarApi::TestController(const httplib::Request& req, httplib::Response& res)
{
	nlohmann::json j;

	j["data"]["test"] = "test";

	res.set_content(j.dump(), "application/json");
}

void SidecarApi::MapBestZController(const httplib::Request& req, httplib::Response& res)
{

}

#include "../../common/file.h"

constexpr static int HTTP_RESPONSE_OK           = 200;
constexpr static int HTTP_RESPONSE_BAD_REQUEST  = 400;
constexpr static int HTTP_RESPONSE_UNAUTHORIZED = 401;

std::string authorization_key;

void SidecarApi::BootWebserver(int port, const std::string &key)
{
	LogInfo("Booting zone sidecar API");

	std::signal(SIGINT, CatchSidecarSignal);
	std::signal(SIGTERM, CatchSidecarSignal);
	std::signal(SIGKILL, CatchSidecarSignal);

	if (!key.empty()) {
		authorization_key = key;
		LogInfo("Booting with authorization key [{}]", authorization_key);
	}

	int         web_api_port = port > 0 ? port : 9099;
	std::string hotfix_name  = "zonesidecar_api_";

	// bake shared memory if it doesn't exist
	// TODO: Windows
	if (!File::Exists("shared/zonesidecar_api_loot_drop")) {
		LogInfo("Creating shared memory for prefix [{}]", hotfix_name);

		std::string output = Process::execute(
			fmt::format(
				"./bin/shared_memory -hotfix={} loot items",
				hotfix_name
			)
		);
		std::cout << output << "\n";
	}

	// bootup a fake zone
	Zone::Bootup(ZoneID("qrg"), 0, false);
	zone->StopShutdownTimer();

	httplib::Server api;

	api.set_logger(SidecarApi::RequestLogHandler);
	api.set_pre_routing_handler(
		[](const auto &req, auto &res) {
			for (const auto &header: req.headers) {
				auto header_key   = header.first;
				auto header_value = header.second;

				LogHTTPDetail("[API] header_key [{}] header_value [{}]", header_key, header_value);

				if (header_key == "Authorization") {
					std::string auth_key = header_value;
					Strings::FindReplace(auth_key, "Bearer", "");
					Strings::Trim(auth_key);

					LogHTTPDetail(
						"Request Authorization key is [{}] set key is [{}] match [{}]",
						auth_key,
						authorization_key,
						auth_key == authorization_key ? "true" : "false"
					);

					// authorization key matches, pass the request on to the route handler
					if (!authorization_key.empty() && auth_key != authorization_key) {
						LogHTTPDetail("[Sidecar] Returning as unhandled, authorization passed");
						return httplib::Server::HandlerResponse::Unhandled;
					}
				}
			}

			if (!authorization_key.empty()) {
				nlohmann::json j;
				j["error"] = "Authorization key not valid!";
				res.set_content(j.dump(), "application/json");
				res.status = HTTP_RESPONSE_UNAUTHORIZED;
				return httplib::Server::HandlerResponse::Handled;
			}

			return httplib::Server::HandlerResponse::Unhandled;
		}
	);
	api.Get("/api/v1/test-controller", SidecarApi::TestController);
	api.Get("/api/v1/loot-simulate", SidecarApi::LootSimulatorController);

	LogInfo("Webserver API now listening on port [{0}]", web_api_port);

	// this is not supposed to bind to the outside world
	api.listen("localhost", web_api_port);
}
