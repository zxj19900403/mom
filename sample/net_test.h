#pragma once

#include <circular_buf.h>
#include <util.h>
#include <session.h>
#include <monitor.h>
#include <tcp_client.h>
#include <tcp_server.h>

using namespace VK;
using namespace Net;

char reqBytes[] = "Hello mom!";
char respBytes[] = "Hello IO!";

static void run_server_test() {
	auto server = std::make_unique<TcpServer>("127.0.0.1", 5002,
	                                          // session established callback
	                                          [=](bool success, session_ptr_t session) { },

	                                          // session closed callback
	                                          [](session_ptr_t session) {},

	                                          // request handler
	                                          [](session_ptr_t session, cbuf_ptr_t pcb, resp_cb_t cb) {
		                                          if (cb) {
			                                          auto resp = alloc_cbuf(cbuf_len_t(strlen(respBytes) + 1));
			                                          resp->write_binary(respBytes, cbuf_len_t(strlen(respBytes) + 1));

			                                          cb(0, resp);
		                                          }
	                                          },

	                                          // push handler
	                                          [](session_ptr_t session, cbuf_ptr_t pcb) {});

	server->startup();
	RUN_UV_DEFAULT_LOOP();
	server->shutdown();
}

static void _request(session_ptr_t session) {
	auto pcb = alloc_cbuf(cbuf_len_t(strlen(reqBytes) + 1));
	pcb->write_binary(reqBytes, cbuf_len_t(strlen(reqBytes) + 1));

	session->request(pcb, [](session_ptr_t session, error_no_t err, cbuf_ptr_t pcb) {
		                 if (!err)
			                 _request(session);
	                 });
}

static void run_client_test(int cnt) {
	std::vector<std::shared_ptr<TcpClient>> clients;
	for (auto i = 0; i < cnt; ++i) {
		clients.push_back(std::make_shared<TcpClient>("127.0.0.1", 5002,
		                                              // session established callback
		                                              [=](bool success, session_ptr_t session) {
			                                              if (success)
				                                              _request(session);
		                                              },

		                                              // session closed callback
		                                              [](session_ptr_t session) {},

		                                              // request handler
		                                              [](session_ptr_t session, cbuf_ptr_t pcb, resp_cb_t cb) {},

		                                              // push handler
		                                              [](session_ptr_t session, cbuf_ptr_t pcb) {},

		                                              // auto reconnect enabled
		                                              true));
	}

	for (auto client : clients) {
		client->startup();
	}

	RUN_UV_DEFAULT_LOOP();

	for (auto client : clients) {
		client->shutdown();
	}
}
