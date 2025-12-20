#include <server.hpp>
#include <request.hpp>
#include <console.hpp>
#include <status.hpp>
#include <sys/socket.h>

void methodGet(int client, request& req, ctr& currentServer, long long startRequestTime) {
  send(client, "HTTP/1.1 204 No Content\r\n\r\n", 25, 0);
  (void)req;
  (void)currentServer;
  (void)startRequestTime;
  (void)client;
}